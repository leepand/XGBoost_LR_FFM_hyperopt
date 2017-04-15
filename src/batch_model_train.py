#### warpper for hyperopt for logging the training reslut
# adopted from
#
import os
import csv
import numpy as np
import sys
sys.path.append("../")
#hypheropt
from hyperopt import fmin, tpe, hp, STATUS_OK, Trials
from util.model_library_config  import feat_folders, feat_names, param_spaces, int_feat
from util.auto_cv import hyperopt_obj
output_path='./model_batch_out'
if not os.path.exists(output_path):
    os.makedirs(output_path)
def hyperopt_wrapper(param, feat_folder, feat_name):
    global trial_counter
    global log_handler
    trial_counter += 1

    # convert integer feat
    for f in int_feat:
        if param.has_key(f):
            param[f] = int(param[f])

    print("--------------------------------")
    print "Trial %d" % trial_counter

    print("Model")
    print("%s" % feat_name)
    print("Param")
    for k,v in sorted(param.items()):
        print("%s: %s" % (k,v))
    print("Result")
    print("Run  Fold Bag Acc Shape")

    ## evaluate performance
    acc_cv_mean = hyperopt_obj(param, feat_folder, feat_name, trial_counter)

    ## log
    var_to_log = [
        "%d" % trial_counter,
        "%.6f" % acc_cv_mean
    ]
    for k,v in sorted(param.items()):
        var_to_log.append("%s" % v)
    writer.writerow(var_to_log)
    log_handler.flush()

    return {'loss': -acc_cv_mean, 'status': STATUS_OK}
####################
## Model Buliding ##
####################

def check_model(models, feat_name):
    if models == "all":
        return True
    for model in models:
        if model in feat_name:
            return True
    return False

if __name__ == "__main__":
    specified_models = sys.argv[1:]
    if len(specified_models) == 0:
        print("You have to specify which model to train.\n")
        print("Usage: python ./batch_model_train.py model1 model2 model3 ...\n")
        sys.exit()
    #output_path='./model_batch_out'
    #if not os.path.exists(output_path):
    #    os.makedirs(output_path)
    log_path = "%s/Log" % output_path
    if not os.path.exists(log_path):
        os.makedirs(log_path)
    for feat_name, feat_folder in zip(feat_names, feat_folders):
        if not check_model(specified_models, feat_name):
            continue
        param_space = param_spaces[feat_name]
        #"""

        log_file = "%s/%s_hyperopt.log" % (log_path, feat_name)
        log_handler = open( log_file, 'wb' )
        writer = csv.writer( log_handler )
        headers = [ 'trial_counter', 'cv_mean' ]
        for k,v in sorted(param_space.items()):
            headers.append(k)
        writer.writerow( headers )
        log_handler.flush()
        
        print("************************************************************")
        print("Search for the best params")
        #global trial_counter
        trial_counter = 0
        trials = Trials()
        objective = lambda p: hyperopt_wrapper(p, feat_folder, feat_name)
        best_params = fmin(objective, param_space, algo=tpe.suggest,
                           trials=trials, max_evals=param_space["max_evals"])
        for f in int_feat:
            if best_params.has_key(f):
                best_params[f] = int(best_params[f])
        print("************************************************************")
        print("Best params")
        for k,v in best_params.items():
            print "        %s: %s" % (k,v)
        trial_acc = -np.asarray(trials.losses(), dtype=float)
        best_acc_mean = max(trial_acc)
        ind = np.where(trial_acc == best_acc_mean)[0][0]
        print("ACC stats")
        print("Mean: %.6f\n" % (best_acc_mean))
