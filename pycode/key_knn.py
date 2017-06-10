# coding: utf-8
"""
The MIT License (MIT)

Copyright (c) 2017 Leepand
"""
#keyord_knn.py
import gzip
import struct
import cPickle
import collections
class BaseANN(object):
    def use_threads(self):
        return True
class Annoy(BaseANN):
    def __init__(self, metric, n_trees, search_k):
        self._n_trees = n_trees
        self._search_k = search_k
        self._metric = metric
        self.name = 'Annoy(n_trees=%d, search_k=%d)' % (n_trees, search_k)
        self.id2word={}
        self.word2id={}
    @classmethod
    def get_vectors(cls,fn, n=float('inf')):
        def _get_vectors(fn):
            if fn.endswith('.gz'):
                f = gzip.open(fn)
                fn = fn[:-3]
            else:
                f = open(fn)
            if fn.endswith('.bin'): # word2vec format
                words, size = (int(x) for x in f.readline().strip().split())
                t = 'f' * size
                while True:
                    pos = f.tell()
                    buf = f.read(1024)
                    if buf == '' or buf == '\n': return
                    i = buf.index(' ')
                    word = buf[:i]
                    f.seek(pos + i + 1)
                    vec = struct.unpack(t, f.read(4 * size))
                    yield word, vec

            elif fn.endswith('.txt'): # Assume simple text format
                for line in f:
                    items = line.strip().split()
                    yield items[0], [float(x) for x in items[1:]]

            elif fn.endswith('.pkl'): # Assume pickle (MNIST)
                mi = 0
                for pics, labels in cPickle.load(f):
                    for pic in pics:
                        yield mi, pic
                        mi += 1
        i = 0
        for line in _get_vectors(fn):
            yield line
            i += 1
            if i >= n:
                break
    def fit(self, vecpath):
        import annoy
        self._annoy = annoy.AnnoyIndex(f=200, metric=self._metric)
        getvec=self.get_vectors(vecpath)
        
        j=0
        for i, x in getvec:
            self.id2word[int(j)]=str(i.strip())
            self.word2id[str(i.strip())]=int(j)
            self._annoy.add_item(j, x)
            j=j+1
        print 'building annoy tree...'
        self._annoy.build(self._n_trees)
        self._annoy.save('wiki2.ann')
        with open('id2word.pickle', 'wb') as f:
            cPickle.dump(self.id2word, f)     
        with open('word2id.pickle', 'wb') as f:
            cPickle.dump(self.word2id, f)
        print 'building annoy tree DONE!'
    def predict(self,annoytreepath):
        import annoy
        self._annoy = annoy.AnnoyIndex(f=200, metric=self._metric)
        self._annoy.load(annoytreepath)
        with open('word2id.pickle', 'rb') as f:
            self.word2id = cPickle.load(f) 
        with open('id2word.pickle', 'rb') as f:
            self.id2word = cPickle.load(f) 
    def query(self, v, n):
        return self._annoy.get_nns_by_vector(v.tolist(), n, self._search_k)
    def search(self,word):
        index=self.word2id[str(word)]
        knn=dict(zip((self._annoy.get_nns_by_item(index, 5,include_distances=True))[0] ,\
                     (self._annoy.get_nns_by_item(index, 5,include_distances=True))[1] ))
        knn_word_dist=collections.OrderedDict({self.id2word[int(k)]:(1-(v**2)/2) for k,v in knn.items()})       
        return knn_word_dist
#build annoy tree
an=Annoy(metric = 'angular',n_trees=10,search_k=10)
an.fit(vecpath='./data/wiki_vec.bin')
