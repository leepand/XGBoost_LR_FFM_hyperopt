# coding: utf-8
from util import GBDT4featureGen
####GBDT gen new features(Libsvm/FFM)
import xgboost as xgb

### load data in do training
dtrain = xgb.DMatrix('./data/train.txt')
dtest = xgb.DMatrix('./data/test.txt')
param = {'max_depth':2, 'eta':1, 'silent':1, 'objective':'binary:logistic' }
watchlist  = [(dtest,'eval'), (dtrain,'train')]
num_round = 30
bst = xgb.train(param, dtrain, num_round, watchlist)

print ('start testing predict the leaf indices')
### predict using first 2 tree
leafindex = bst.predict(dtest, ntree_limit=2, pred_leaf=True)
print(leafindex.shape)
#print(leafindex)
bst.save_model('./xgb_leaf.model') 
# this is prediction  
preds = bst.predict(dtest)  
labels = dtest.get_label()  
print ('Before error=%f' % (  sum(1 for i in range(len(preds)) if int(preds[i]>0.5)!=labels[i]) /float(len(preds))))
### predict all trees
leafindex = bst.predict(dtest, pred_leaf = True)
leafindex_train = bst.predict(dtrain, pred_leaf= True)
GBDT4featureGen.save_ffm_libsvm_features_GBDT_gen('./featGen/hash_libsvm.test','./featGen/hash_ffm.test',leafindex.tolist(),dtest.get_label().tolist(),1000)
GBDT4featureGen.save_ffm_libsvm_features_GBDT_gen('./featGen/hash_libsvm.train','./featGen/hash_ffm.train',leafindex_train.tolist(),dtrain.get_label().tolist(),1000)

param2 = {'max_depth':2, 'eta':1, 'silent':1, 'objective':'binary:logistic' ,'booster':'gblinear'}
leaftrain = xgb.DMatrix('./featGen/hash_libsvm.train')
leaftest = xgb.DMatrix('./featGen/hash_libsvm.test')
watchlist2=[(leaftest,'eval'), (leaftrain,'train')]
bst2 = xgb.train(param2, leaftrain, num_round, watchlist2)
preds2 = bst2.predict(leaftest)
labels2 = leaftest.get_label()
print ('After error=%f' % (  sum(1 for i in range(len(preds2)) if int(preds2[i]>0.5)!=labels2[i]) /float(len(preds2))))
