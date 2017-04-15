# coding: utf-8
"""
__file__
    param_config.py
__description__
    This file provides global parameter configurations for the project.
"""

import os
import numpy as np

############
## Config ##
############
class ParamConfig:
    def __init__(self,
                 feat_folder):
    
        self.n_classes = 4

        ## CV params
        self.n_runs = 3
        self.n_folds = 3

        ## path
        self.data_folder = "../"
        self.feat_folder = feat_folder
        ## create feat folder
        if not os.path.exists(self.feat_folder):
            os.makedirs(self.feat_folder)
        ## creat folder for the training and testing feat
        if not os.path.exists("%s/All" % self.feat_folder):
            os.makedirs("%s/All" % self.feat_folder)
        ## creat folder for each run and fold
        for run in range(1,self.n_runs+1):
            for fold in range(1,self.n_folds+1):
                path = "%s/Run%d/Fold%d" % (self.feat_folder, run, fold)
                if not os.path.exists(path):
                    os.makedirs(path)


## initialize a param config					
config = ParamConfig(feat_folder='./feat_folder')

