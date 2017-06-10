### 基于语义树文本结构化算法

文本数据结构化在知识图谱、标签标注、chatbot等应用中占据重要地位，本项目基于语义映射树、word2vec、贝叶斯、威尔逊置信区间等知识对文本语料进行结构化抽取。抽取逻辑：先计算语料归属语义树中相关节点的概率和置信度，然后根据树结构抽取合并当前标注的路径。假设语料：小米科技生产的手机很实惠。抽取后的可能路径为：科技—>制造—>电子产品—>正面

#### 语义树规则模版

- 语义图（树）-标签体系

  - ```
    {
    	"domain": "代表该规则的抽象概念",
    	"concepts": [
    		"该规则的表达（关键词集）"
    	],
    	"children": ["该规则的子规则List"]
    }
    ```

  - 示例：

  - ```
    {
    	"domain": "体育",
    	"concepts": [
    		"健身","打篮球","瑜伽","跑步"
    	],
    	"children": [
    		"健身",
    		"篮球爱好",
    		"瑜伽爱好者",
    	]
    }

    {
    	"domain": "健身",
    	"concepts": [
    		"健身房","哑铃","跑步机"
    	],
    	"children": [
    	]
    }
    ```


#### 威尔逊区间-标签置信

- 计算每个标签的投票率（即一个语料归属该标签的标签词的比率）
  - 计算每个投票率低置信区间（95%）
  - 根据置信区间的下限值作为该标签的置信度
  - 原理解释：置信区间的宽窄与样本的数量有关，比如，某条语料中归属A标签的标签词1个，9个为其他标签；另外一条归属B标签的有3个，27个为其他标签，两个标签的投票比率都是10%，但是B的置信度要高于A
  - 公式：${Wilson_score_interval}=\frac {\hat p+\frac 1{2n}+z^2_{1-\frac{\alpha}{2}}\pm z_{1-\frac\alpha{2}}\sqrt{\frac {\hat p(1-\hat p)}{n}+\frac {z^2_{1-\frac \alpha{2}}}{4n^2}}}{1+\frac 1{n}z^2_{1-\frac \alpha{2}}}$


#### 语料归属标签的bayes概率计算

![\Pr(S|W)={\frac  {\Pr(W|S)\cdot \Pr(S)}{\Pr(W|S)\cdot \Pr(S)+\Pr(W|H)\cdot \Pr(H)}}](https://wikimedia.org/api/rest_v1/media/math/render/svg/dc8c39ec48e65c0ab10dabe343d4da9a9585a77b)

- ![\Pr(S|W)](https://wikimedia.org/api/rest_v1/media/math/render/svg/43b2b14c009a5866c86fc11e9e71e77e43da431a)为语料中token/similarity(token)为某tag S的概率, 比如篮球属于体育的概率；
- ![\Pr(S)](https://wikimedia.org/api/rest_v1/media/math/render/svg/92d3cac5f21efd8c3b89149a016c38f1a612867a) 为对于给定任何语料或token/similarity(token)情况下tag S的先验概率；
- ![\Pr(W|S)](https://wikimedia.org/api/rest_v1/media/math/render/svg/92092547e6ff8eee5358f6d7944a6dbf57097835)为token/similarity(token) W出现在标签S的概率；
- ![\Pr(H)](https://wikimedia.org/api/rest_v1/media/math/render/svg/d321d1c90b5cf98d0619e41a541797dfcfc90bd8) 为对于给定任何语料或token/similarity(token)情况下不在tag S中的先验概率；
- ![\Pr(W|H)](https://wikimedia.org/api/rest_v1/media/math/render/svg/9b8cc835316df92744c04d8a55c47c8cf4b4c8da) 为token/similarity(token) W没有出现在标签S的概率；
- 语料中token归属任一节点的概率：Wilson_score_interval*p(s|w)

#### example

```python
C++ compile:
make
python:

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
```