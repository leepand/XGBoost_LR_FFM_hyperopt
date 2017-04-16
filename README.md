# Features

- 基础特征抽取(Libsvm/FFM)，w2v特征组合，简单转换
- GBDT新的特征生成
- hash trick one-hot编码
- CV+hyperopt+bagging超参数选择



# Usage

- 基础特征抽取

  ```C++
  #抽取特征-onehot
  ./svm_feature_extract -in ./ -suffix txt -feid 1000 -clus conf/cluster.conf -out out/svm_features.txt -debug 1
  #userid与target替换
  ./target_replace -base data/ffm_features.txt -in data/flag_car_sample.txt -out out/ffm_features.txt 
  ```

  - 参数说明：
    - -in：待抽取的数据集
    - -suffix：输入数据集的扩展名
    - -Field：编码开始编号
    - -clus：特征组合配置文件
    - -out：抽取后特征文件

- GBDT特征生成

```python
GBDT4featureGen.save_ffm_libsvm_features_GBDT_gen('./featGen/hash_libsvm.train','./featGen/hash_ffm.train',leafindex_train.tolist(),dtrain.get_label().tolist(),1000)
```

- 生成CV样本

```python
from sklearn.datasets import load_svmlight_file, dump_svmlight_file
import sys
import os
from sklearn.cross_validation import train_test_split  
sys.path.append("../")
from util.param_config import config
#config={}
#n_runs=3
#n_folds=3
#feat_folder='feat_folder'
print("For cross-validation...")
    ## for each run and fold
dfTrain, Y_train = load_svmlight_file('%s/train.txt'% (config.data_folder))
for run in range(1,config.n_runs+1):
    for fold in range(1,config.n_folds+1):
        random_seed = 2017 + 1000 * (run+1)+100*(fold+1)
        print("Run: %d, Fold: %d" % (run, fold))
        path = "%s/Run%d/Fold%d" % (config.feat_folder, run, fold)
        save_path = "%s/Run%d/Fold%d" % (config.feat_folder, run, fold)
        if not os.path.exists(save_path):
            os.makedirs(save_path)
        X_train, X_valid, y_train, Y_valid = train_test_split(dfTrain, Y_train, test_size=0.4, random_state=random_seed)
        dump_svmlight_file(X_train, y_train, "%s/train.feat" % (save_path))
        dump_svmlight_file(X_valid, Y_valid, "%s/valid.feat" % (save_path))
```