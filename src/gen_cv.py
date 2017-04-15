"""
__file__
    gen_cv.py
__description__
    This file generates the CV files  which will be kept fixed in
    ALL the following model building parts.
"""
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
