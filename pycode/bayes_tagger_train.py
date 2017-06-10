#bayes_tagger.py
import json
import simplebayes
def my_tokenizer(sample):
    return sample
def load_rules(path, reload=False, is_root=False):
    with open('word2id.pickle', 'rb') as f:
        word2id_dict = cPickle.load(f)
    """
    Build the rulebase by loading the rules terms from the given file.
    Args: the path of file.
    """
    bn=Annoy(metric = 'angular',n_trees=10,search_k=10)
    bn.predict(annoytreepath='wiki2.ann')
    bayes = simplebayes.SimpleBayes(cache_path='./my/tree/')
    bayes.cache_train()
    with open(path, 'r') as input:
        json_data = json.load(input)
        # load rule and build an instance
        for data in json_data:
            simlist=[]
            domain = data["domain"]
            concepts_list = data["concepts"]
            children_list = data["children"]
            response = data["response"]
            simstr=' '.join(concepts_list).encode('utf-8')
            for word in concepts_list:
                if str(word.encode('utf-8')) in word2id_dict:
                    print 'word:',str(word.encode('utf-8'))
                    simstr+=' '.join([k for k,v in bn.search(str(word.encode('utf-8')) ).items() if v>0.8])
                else:
                    continue
            #' '.join(concepts_list).encode('utf-8')
            #print 'sim',simstr
            bayes.train(domain,simstr)
            
    bayes.cache_persist()
