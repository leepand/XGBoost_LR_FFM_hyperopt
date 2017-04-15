#### train CV and final model with a specified parameter setting
#from util import 
from sklearn.datasets import load_svmlight_file, dump_svmlight_file
import sys
import os
import pandas as pd
from sklearn.cross_validation import train_test_split 
import numpy as np
from sklearn.metrics import accuracy_score
import xgboost as xgb 
sys.path.append("../")
from util.param_config import config
bootstrap_ratio = 1
bootstrap_replacement = False
bagging_size= 1
from util import GBDT4featureGen
ebc_hard_threshold = False
verbose_level = 1
output_path='./model_batch_out'
#    if not os.path.exists(output_path):
#        os.makedirs(output_path)
#param = {'bst:max_depth':2, 'bst:eta':1, 'silent':1, 'objective':'binary:logistic',"booster":'gbtree',"task":'regression' ,'num_round':10}
def acc(y_test, y_pred):
    return accuracy_score(y_test, y_pred)
def hyperopt_obj(param, feat_folder, feat_name, trial_counter):

    acc_cv = np.zeros((config.n_runs, config.n_folds), dtype=float)
    #print kappa_cv
    for run in range(1,config.n_runs+1):
        for fold in range(1,config.n_folds+1):
            rng = np.random.RandomState(2015 + 1000 * run + 10 * fold)

            #### all the path
            path = "%s/Run%d/Fold%d" % (feat_folder, run, fold)
            save_path = "%s/Run%d/Fold%d" % (output_path, run, fold)
            if not os.path.exists(save_path):
                os.makedirs(save_path)
            # feat
            feat_train_path = "%s/train.feat" % path
            feat_valid_path = "%s/valid.feat" % path
            raw_pred_valid_path = "%s/valid.raw.pred.%s_[Id@%d].csv" % (save_path, feat_name, trial_counter)
            #if not os.path.exists(raw_pred_valid_path):
            #    os.makedirs(raw_pred_valid_path)
            ## load feat
            X_train, labels_train = load_svmlight_file(feat_train_path)
            X_valid, labels_valid = load_svmlight_file(feat_valid_path)
            numTrain=labels_train.shape[0]
            numValid=labels_valid.shape[0]
            print numTrain
            
            ##############
            ## Training ##
            ##############
            ## you can use bagging to stabilize the predictions
            preds_bagging = np.zeros((numValid, bagging_size), dtype=float)
            for n in range(bagging_size):
                if bootstrap_replacement:
                    sampleSize = int(numTrain*bootstrap_ratio)
                    index_base = rng.randint(numTrain, size=sampleSize)
                    index_meta = [i for i in range(numTrain) if i not in index_base]
                else:
                    randnum = rng.uniform(size=numTrain)
                    index_base = [i for i in range(numTrain) if randnum[i] < bootstrap_ratio]
                    index_meta = [i for i in range(numTrain) if randnum[i] >= bootstrap_ratio]
                
                if param.has_key("booster"):
                    dvalid_base = xgb.DMatrix(X_valid, label=labels_valid)
                    dtrain_base = xgb.DMatrix(X_train[index_base], label=labels_train[index_base])
                        
                    watchlist = []
                    if verbose_level >= 2:
                        watchlist  = [(dtrain_base, 'train'), (dvalid_base, 'valid')]
                    
                ## various models
                if param["task"] in ["regression", "ranking"]:
                    ## regression & pairwise ranking with xgboost
                    bst = xgb.train(param, dtrain_base, param['num_round'], watchlist, feval='auc')
                    pred = bst.predict(dvalid_base)
                    #print pred
		    bst2 = xgb.Booster(model_file='./xgb_leaf.model')
                    preds2 = bst2.predict(dvalid_base,pred_leaf=True)
                    #print 'pred:',preds2
                    GBDT4featureGen.save_ffm_libsvm_features_GBDT_gen('./featGen/hash_libsvm.train','./featGen/hash_ffm.train',preds2.tolist(),dvalid_base.get_label().tolist(),1000)
                    
                ## weighted averageing over different models
                pred_valid = pred
                ## this bagging iteration
                preds_bagging[:,n] = pred_valid
                pred_raw = np.mean(preds_bagging[:,:(n+1)], axis=1)
                pred_raw=[1.0 if i>=0.5 else 0.0 for i in pred_raw]
                acc_valid=acc(labels_valid,pred_raw)
                print acc_valid
            acc_cv[run-1,fold-1] = acc_valid 
            ## save this prediction
            dfPred = pd.DataFrame({"target": labels_valid, "prediction": pred_raw})
            dfPred.to_csv(raw_pred_valid_path, index=False, header=True,
                         columns=["target", "prediction"])
    acc_cv_mean = np.mean(acc_cv)
    if verbose_level >= 1:
        print("Mean: %.6f" % acc_cv_mean)
    return acc_cv_mean            
#output_path='./model_out'
#hyperopt_obj(param, config.feat_folder, 'feat_name', 10)
