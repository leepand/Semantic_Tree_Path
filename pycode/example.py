# -*- coding:utf-8 -*-  
#tagger_score.py
load_rules('./rules/rule.json')
b_p=simplebayes.SimpleBayes(cache_path='./my/tree/')
b_p.get_cache_location()
print b_p.get_cache_location()
b_p.cache_train()
print 'predict:'
import jieba
#print ' '.join(jieba.lcut('逛街購物訂購住宿'))             
for k,v in b_p.score(' '.join(jieba.lcut('逛街購物訂購住宿鬧鐘鬧鐘贝叶斯自修室饭堂饭堂')).encode('utf-8')).items():
    print k,v

