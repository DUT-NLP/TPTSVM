# TPTSVM
半监督学习和迁移学习相结合的跨领域及其学习方法
## 数据集
The mushroom data from the UCI machine learnning repository <http://archive.ics.uci.edu/ml/datasets/Mushroom><br>
We also use the 20 newsgroups data <http://qwone.com/~jason/20Newsgroups/><br>
## 算法描述和结果
算法的详细步骤请参考文献（Semi-supervised Learning with Transfer Learning）<br>在mushroom数据集上的实验结果和在20 newsgroups 上的实验结果如下图所示，图1为mushroom数据集的结果对比，图2为 20 newsgroups 上三个任务的对比结果。<br>


![ex1](https://github.com/DUT-NLP/TPTSVM/blob/master/Sample/ex1.png)<br>
Fig.1. Accuracy on the mushroom dataset <br>

![ex2](https://github.com/DUT-NLP/TPTSVM/blob/master/Sample/ex2.png)<br>

Fig.2. Comparison results on the newsgroups data 





## 程序运行
Windows下直接编译main.cpp，运行<br>
程序会在D盘下新建文件夹PTSVMtemp<br>

### 参数说明
1. argv1：源领域
2. argv2：目标领域标记数据
3. argv3：目标领域未标记数据
4. argv4: 中间结果输出路径
5. 101: 算法的迭代次数
6. 20: 每次迭代从目标领域未标注数据选取的可信样例个数
> 控制台输出文件<br>
详细的程序说明请查阅code文件夹下使用说明PDF文件。
## 参考文献
如果你使用了我们的代码，请引用下面这篇参考文献：<br>
H W Zhou, Y Zhang, D G Huang and L S Li. Semi-supervised Learning with Transfer Learning[M]// Chinese Computational Linguistics and Natural Language Processing Based on Naturally Annotated Big Data. 2013:109-119.
