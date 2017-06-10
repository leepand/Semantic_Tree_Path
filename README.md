### 基于语义树标签标注算法

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

- 威尔逊区间-标签置信

  - 计算每个标签的投票率（即一个语料归属该标签的标签词的比率）
  - 计算每个投票率低置信区间（95%）
  - 根据置信区间的下限值作为该标签的置信度
  - 原理解释：置信区间的宽窄与样本的数量有关，比如，某条语料中归属A标签的标签词1个，9个为其他标签；另外一条归属B标签的有3个，27个为其他标签，两个标签的投票比率都是10%，但是B的置信度要高于A
  - 公式：$\frac {\hat p+\frac 1{2n}+z^2_{1-\frac{\alpha}{2}}\pm z_{1-\frac\alpha{2}}\sqrt{\frac {\hat p(1-\hat p)}{n}+\frac {z^2_{1-\frac \alpha{2}}}{4n^2}}}{1+\frac 1{n}z^2_{1-\frac \alpha{2}}}$


<img src="http://www.forkosh.com/mathtex.cgi? \Large \frac {\hat p+\frac 1{2n}+z^2_{1-\frac{\alpha}{2}}\pm z_{1-\frac\alpha{2}}\sqrt{\frac {\hat p(1-\hat p)}{n}+\frac {z^2_{1-\frac \alpha{2}}}{4n^2}}}{1+\frac 1{n}z^2_{1-\frac \alpha{2}}}">


- TF*IDF-标签rewight

  - 标签局部频率（TF）= $\frac {该标签在语料中出现的次数}{该语料中次数最多的标签的出现次数}$
  - 标签逆全局频率（IDF）=$log(\frac {当前的用户总量}{包含该标签的用户树+1})$，可以发现，该标签越常见，分母越大，逆全局频率就越小越接近0
  - 标签权重=TF*IDF

  ​

