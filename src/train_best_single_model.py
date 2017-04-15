"""
__file__
	train_best_single_model.py
__description__
	This file generates the best single model.
"""

import os

feat_names = [
	"GBDT_pred_leaf_[Model@reg_xgb_tree]",
        "[GBDT_pred_leaf_[Model@reg_xgb_linear]",
]

for feat_name in feat_names:
	cmd = "python ./batch_model_train.py %s" % feat_name
	os.system( cmd )
