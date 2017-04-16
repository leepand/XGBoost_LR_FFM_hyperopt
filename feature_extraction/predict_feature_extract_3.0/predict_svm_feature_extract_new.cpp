#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>//sort

using namespace std;

#define MAX_STRING 100

char in_path[MAX_STRING], suffix[MAX_STRING], out_file[MAX_STRING], cluster_file[MAX_STRING], fid_file[MAX_STRING];
int debug_mode = 2;
pthread_mutex_t *pmutex = NULL;
typedef enum _FEATURE_TYPE_E{
    D_E,    //Disperse
    SD_E,   //Self defined Disperse: time->split string and keep the date without time.
    SCD_E,  //self defined Continuous to Disperse
    C_E,    //Continuous: pv->
    MK_E,   //merge key
    MV_E,   //merge value
    CD10_E, //Continuous to Disperse, by /= 10
    CD100_E,//Continuous to Disperse, by /= 100
    G_E,    //Global Disperse:
    I_E,    //ID:
    N_E     //None:
} FEATURE_TYPE_E;
map<string, vector<pair<string, FEATURE_TYPE_E> > > g_mapFeaturesType; //map<file_name, vector<pair<featureName, FEATURE_TYPE_E>>>
map<string, map<int, int> > g_mapResFeatures;//map<uid, map<featureid, value>>
//map<string, int> g_mapUids;//<cookie, uid>
map<string, int> g_mapFids;//<featurename, fid>
map<string, string> g_mapCluster;//<featurename, clustername>
vector<string> g_vfilenames;//<filename>
//static int g_uid = 0; //new user id
//static int g_fid = 0;//new feature id

static void split(std::string& s, std::string& delim,std::vector< std::string >* ret)
{
    size_t last = 0;
    size_t index=s.find_first_of(delim,last);
    while (index!=std::string::npos)
    {
        ret->push_back(s.substr(last,index-last));
        //last=index+1;
        last = index + delim.length();
        index = s.find_first_of(delim,last);
    }
    if (index-last>0)
    {
        ret->push_back(s.substr(last,index-last));
    }
}
bool cmpIds(int &a, int &b){
    return a < b;
}
void InitFids(){
    ifstream ifs(fid_file);
    if(!ifs.is_open()){
        cout << "cant open the feature<->id file:" << fid_file << endl;
        return;
    }
    string line;
    string delim = " ";
    vector<string> vec;
    while(getline(ifs, line)){
        vec.clear();
        split(line, delim, &vec);
        //map<string, int> g_mapFids;//<featurename, fid>
        if(2 <= vec.size()){
            g_mapFids[vec[0]] = atoi(vec[1].c_str());
        }
    }
    if(debug_mode >= 1){
        cout << "g_mapFids.size = " << g_mapFids.size() << endl;
        if(debug_mode >= 2){
            for(auto &it:g_mapFids){
                cout << "g_mapFids[" << it.first << "]=" << it.second << endl;
            }
        }
    }
    ifs.close();
}
void InitClusterData(){
    if(strlen(cluster_file)<= 0) return;
    //map<string, string> g_mapCluster;//<featurename, clustername>
    ifstream ifs(cluster_file);
    if(!ifs.is_open()){
        cout << "open base file failed:" << cluster_file << endl;
        return;
    }
    string line;
    string delim = " ";
    vector<string> vec;
    while(getline(ifs, line)){
        vec.clear();
        split(line, delim, &vec);
        if(vec.size() >= 2){
            g_mapCluster[vec[0]] = "C_"+vec[1];
        }
    }
    if(debug_mode >= 2){
        for(auto &it:g_mapCluster){
            cout << "g_mapCluster[" << it.first << "]=" << it.second << endl;
        }
    }
    ifs.close();
}

/*
void InitUids(){
    ifstream ifs(uid_file);
    if(!ifs.is_open()){
        cout << "cant open the user<->id file:" << uid_file << endl;
        return;
    }
    string line;
    string delim = " ";
    vector<string> vec;
    while(getline(ifs, line)){
        vec.clear();
        split(line, delim, &vec);
        //map<string, int> g_mapUids;//<cookie, uid>
        if(2 <= vec.size()){
            g_mapUids[vec[0]] = atoi(vec[1].c_str());
        }
    }
    if(debug_mode >= 1){
        cout << "g_mapUids.size = " << g_mapUids.size() << endl;
        if(debug_mode >= 2){
            for(auto &it:g_mapUids){
                cout << "g_mapUids[" << it.first << "]=" << it.second << endl;
            }
        }
    }

    ifs.close();
}*/
void ReadConfFile(){
    string conf_file(in_path);
    conf_file += "/conf/feature.conf";
    ifstream ifs(conf_file.c_str());
    if(!ifs.is_open()){
        cout << "open file_name.conf failed:" << conf_file << endl;
    }
    string line;
    string delim = "\t";
    string sdel = " ";
    string ssdel = ":";
    vector<string> vec;
    vector<string> svec;
    vector<string> ssvec;
    while(getline(ifs,line)){
        vec.clear();
        split(line, delim, &vec);
        if(vec.size() == 2){
            svec.clear();
            split(vec[1], sdel, &svec);
            for(auto & it: svec){
                ssvec.clear();
                split(it, ssdel, &ssvec);
                if(ssvec.size() == 2){

                    //D_E,    //Disperse
                    //SD_E,   //Self defined Disperse: time->split string and keep the date without time.
                    //C_E,    //Continuous: pv->
                    //CD10_E, //Continuous to Disperse, by /= 10
                    //CD100_E,//Continuous to Disperse, by /= 100
                    //SCD_E,  //self defined Continuous to Disperse
                    //G_E,    //Global Disperse:
                    //I_E,    //ID:
                    //N_E     //None:

                    if("D" == ssvec[1]){
                        g_mapFeaturesType[vec[0]].push_back(pair<string, FEATURE_TYPE_E>(ssvec[0], D_E));
                    }else if("SD" == ssvec[1]){
                        g_mapFeaturesType[vec[0]].push_back(pair<string, FEATURE_TYPE_E>(ssvec[0], SD_E));
                    }else if("C" == ssvec[1]){
                        g_mapFeaturesType[vec[0]].push_back(pair<string, FEATURE_TYPE_E>(ssvec[0], C_E));
                    }else if("CD10" == ssvec[1]){
                        g_mapFeaturesType[vec[0]].push_back(pair<string, FEATURE_TYPE_E>(ssvec[0], CD10_E));
                    }else if("CD100" == ssvec[1]){
                        g_mapFeaturesType[vec[0]].push_back(pair<string, FEATURE_TYPE_E>(ssvec[0], CD100_E));
                    }else if("SCD" == ssvec[1]){
                        g_mapFeaturesType[vec[0]].push_back(pair<string, FEATURE_TYPE_E>(ssvec[0], SCD_E));
                    }else if("G" == ssvec[1]){
                        g_mapFeaturesType[vec[0]].push_back(pair<string, FEATURE_TYPE_E>(ssvec[0], G_E));
                    }else if("I" == ssvec[1]){
                        g_mapFeaturesType[vec[0]].push_back(pair<string, FEATURE_TYPE_E>(ssvec[0], I_E));
                    }else if("N" == ssvec[1]){
                        g_mapFeaturesType[vec[0]].push_back(pair<string, FEATURE_TYPE_E>(ssvec[0], N_E));
                    }else if("MK" == ssvec[1]){
                        g_mapFeaturesType[vec[0]].push_back(pair<string, FEATURE_TYPE_E>(ssvec[0], MK_E));
                    }else if("MV" == ssvec[1]){
                        g_mapFeaturesType[vec[0]].push_back(pair<string, FEATURE_TYPE_E>(ssvec[0], MV_E));
                    }else{
                        cout << "unknown FEATURE_TYPE:" << ssvec[1] << " in line :" << line << endl;
                    }
                }else{
                    cout << it << " spit failed, size = " << ssvec.size() << endl;
                }
            }
        }else{
            cout << "format error of file_name.conf:" << line << " size=" << vec.size() << endl;
        }
    }
    ifs.close();
    if(debug_mode >= 2){
        for(auto & it:g_mapFeaturesType){
            for(auto &itt:it.second){
                cout << it.first << ":" << itt.first << ":" << itt.second << endl;
            }
        }
    }
}
/*int GetUid(string cookie){
    //map<string, int> g_mapUids;//<cookie, uid>
    auto it = g_mapUids.find(cookie);
    if(it != g_mapUids.end()){
        return it->second;
    }else{
        g_mapUids[cookie] = g_uid++;
        return g_uid - 1;
    }
}*/
string regex_number(vector<double> &res, string text){
    int len = text.length();
    double v = 0.0;
    bool start = false;
    bool isDot = false;
    if(debug_mode >= 1){
        cout << "regex strvalue:" << text << endl;
    }
    for(int i = 0; i < len; ++i){
        if(debug_mode >= 2){
            cout << "text[" << i << "]=" <<text[i]<< endl;
        }
        if(text[i] >= '0' && text[i] <= '9'){
            if(!start){
                start = true;
                v = text[i]-'0';
            }else{
                if(!isDot){
                    v *= 10;
                    v += text[i] - '0';
                }else{
                    double vv = (double)(text[i] - '0');
                    v += vv/10;
                }
            }

            if(debug_mode >= 2){
                cout << "v=" <<v<< endl;
            }
        }
        else if(text[i] == '.' && start){
            isDot = true;
        }else{
            if(start){
                res.push_back(v);
                if(debug_mode >= 1){
                    cout << "regex v:" << v << endl;
                }
                v = 0;
            }
            start = false;
            isDot = false;
        }
    }
    if(start){
        res.push_back(v);
        if(debug_mode >= 1){
            cout << "regex v:" << v << endl;
        }
        v = 0;
        start = false;
        isDot = false;
    }
    if(res.size() != 1){
        return text;
    }else{
        return "";
    }
}
int GetFid(string filename, string featurename, string strvalue, FEATURE_TYPE_E type){
    //map<string, int> g_mapFids;//<featurename, fid>

    //D_E,    //Disperse: <filename_varname_value:1>
    //SD_E,   //Self defined Disperse: time->split string and keep the date without time. <filename_varname_date:1>
    //C_E,    //Continuous: pv-> plus the count for according to the userID <filename_pv_uid:n>
    //CD10_E, //Continuous to Disperse, by /= 10 <filename_varname_10:1>
    //CD100_E,//Continuous to Disperse, by /= 100 <filename_varname_100:1>
    //SCD_E,  //self defined Continuous to Disperse
    //G_E,    //Global Disperse: <varname_value:1>
    //I_E,    //ID:
    //N_E     //None:

    string fname = "";
    string sub = "NULL";
    switch(type){
        case C_E://pv
            if(featurename == "pv"){
                fname = filename +"_pv";
            }
            break;
        case D_E:
            fname = filename+"_"+featurename+"_"+strvalue;
            break;
        case SD_E://for time data only
            if(strvalue != "NULL"){
                string delim = " ";
                vector<string> vec;
                split(strvalue, delim, &vec);
                if(vec.size() != 2){
                    cout << "GetFid::SD_E split time string error:" << strvalue << endl;
                }
                sub = vec[0];
            }
            fname = filename+"_"+featurename+"_"+sub;
            break;
        case SCD_E://special format <number + string>, select the number and /100
            if(strvalue != "NULL"){
                vector<double> vd;
                string strval = regex_number(vd, strvalue);
                if(vd.size() != 1){
                    sub = strval;
                    if(debug_mode >= 2){
                        cout << "strvalue: " << strvalue << "->regex_number return string:" << sub << endl;
                    }
                }else{
                    int ivalue = (int)vd[0];
                    stringstream ss;
                    ss << ivalue/100;
                    sub = "";
                    ss >> sub;

                    if(debug_mode >= 2){
                        cout << "strvalue: " << strvalue << "->regex_number return number:" << vd[0] << endl;
                    }
                }
            }
            fname = filename+"_"+featurename+"_"+sub;
            break;
        case CD10_E://age
            fname = filename+"_"+featurename+"_NULL";
            if(strvalue != "NULL"){
                int value = (int)atof(strvalue.c_str());
                value /= 10;
                stringstream ss;
                ss << filename << "_" << featurename << "_" << value;
                fname = ss.str();
            }
            break;
        case CD100_E://age
            fname = filename+"_"+featurename+"_NULL";
            if(strvalue != "NULL"){
                int value = (int)atof(strvalue.c_str());
                value /= 100;
                stringstream ss;
                ss << filename << "_" << featurename << "_" << value;
                fname = ss.str();
            }
            break;
        case G_E:
            fname = strvalue;
            break;
        case MK_E:
        case MV_E:
        case I_E:
        case N_E:
        default:
            break;
    }
    if(fname.length() > 0){
        //add cluster feature
        auto itclu = g_mapCluster.find(fname);
        if(itclu != g_mapCluster.end()){
            fname = g_mapCluster[fname];
        }
        //add
        auto it = g_mapFids.find(fname);
        if(it != g_mapFids.end()){
            return it->second;
        }else{
            return -1;
        }
    }else{
        return -1;
    }

}
bool GetMergeFIDs(int &mfeatureid,
                  int &value,
                  string &filename,
                  vector<pair<string, FEATURE_TYPE_E> > &vfeatures,
                  vector<string> &values){
    if(values.size() != vfeatures.size()){
        cout << "features size[" << vfeatures.size() << "] != Values size[" << values.size() << "]" << endl;
        return false;
    }
    int mkindex = -1, mvindex = -1;
    bool hasMF = false;
    for(int i = 0; i < vfeatures.size(); ++i){
        if(vfeatures[i].second == MK_E){
            hasMF = true;
            mkindex = i;
        }else if(vfeatures[i].second == MV_E){
            hasMF = true;
            mvindex = i;
        }
    }
    if(mvindex > -1){
        value = atoi(values[mvindex].c_str());
    }

    if(debug_mode >= 2){
        cout << "hasMF:" << hasMF << " mkindex:" << mkindex << " mvindex:" << mvindex << endl;
    }
    if(hasMF && mkindex > -1 && mvindex > -1){
        string featname = filename + "_" + vfeatures[mkindex].first + "_" + values[mkindex] + "_" + vfeatures[mvindex].first;
        auto itfeat = g_mapFids.find(featname);
        if(itfeat != g_mapFids.end()){
            mfeatureid = itfeat->second;
            return true;
        }else{
            mfeatureid = -1;
            return false;
        }
    }else{
        mfeatureid = -1;
        return false;
    }
}
bool AddFeature(int fid, string uid, int val, bool isCVal = false){
    if(fid < 0 || uid.length() <= 0 || val < 0) return false;
    //map<int, map<int, int> > g_mapResFeatures;//map<uid, map<featureid, value>>
    if(debug_mode >= 2){
        cout << "add feature:uid=" <<uid << " fid=" << fid << " val=" << val << endl;
    }
    auto it = g_mapResFeatures.find(uid);
    if(it != g_mapResFeatures.end()){
        auto itf = it->second.find(fid);
        if(itf != it->second.end()){
            if(isCVal){
                pthread_mutex_lock(pmutex);
                itf->second += val;
                pthread_mutex_unlock(pmutex);
            }
        }else{
            pthread_mutex_lock(pmutex);
            it->second[fid] = val;
            pthread_mutex_unlock(pmutex);
        }
    }else{
        map<int, int>  mf;
        mf[fid] = val;
        pthread_mutex_lock(pmutex);
        g_mapResFeatures[uid] = mf;
        pthread_mutex_unlock(pmutex);
    }
    return true;
}

void *ExtractThread(void *param){
    int per_print = 500;
    long index = (long)param;
    cout << "thread " << index << ":" << endl;
    string file_name=g_vfilenames[index];
    string file_path(in_path);
    file_path += "data/" +file_name + "." + string(suffix);
    ifstream ifs(file_path.c_str());
    if(!ifs.is_open()){
        cout << "cant open thread file:" << file_path << endl;
        return NULL;
    }
    string line;
    string delim = "\t";
    vector<string> vec;
    //map<file_name, vector<pair<featureName, FEATURE_TYPE_E>>>
    auto itfile = g_mapFeaturesType.find(file_name);
    if(itfile == g_mapFeaturesType.end()){
        cout << "conf file and data file didn't match, cant find file_name:" << file_name << endl;
        return NULL;
    }
    long counter = 0;
    int feature_cnt = itfile->second.size();//there is no cookie-feature in g_mapFeaturesType
    while(getline(ifs, line)){
        vec.clear();
        split(line, delim, &vec);
        if(vec.size() != feature_cnt){
            cout << "conf file and data file didn't match, file:" << file_name << endl
                <<"line:" << line << endl
                <<"[feature size error] size:" << feature_cnt
                << " data feature size:" << vec.size() << endl;
            if(debug_mode >= 2){
                cout << "data feature:" << endl;
                for(auto &ii: vec){
                    cout << ii << " ";
                }
                cout << endl;
            }
            continue;
        }
        string uid = vec[0];
        /*int uid = GetUid(vec[0]);
        if(-1 == uid)
            continue;*/
        int  mfeatureid = -1, val = 0;
        bool hasMFid = GetMergeFIDs( mfeatureid, val, file_name, itfile->second, vec);
        if(hasMFid){
            AddFeature( mfeatureid, uid, val, true);
        }

        //map<string, vector<pair<string, FEATURE_TYPE_E>>> => map<file_name, vector<pair<featureName, FEATURE_TYPE_E>>>
        for(int i = 1; i < feature_cnt; ++i){//i = 0 =>cookie => uid

            int fid = GetFid(file_name,itfile->second[i].first, vec[i], itfile->second[i].second);
            if(fid < 0)
                continue;
            switch(itfile->second[i].second){
                case C_E://vp
                    AddFeature(fid, uid, atoi(vec[i].c_str()), true);
                    break;
                case D_E:
                case CD10_E:
                case CD100_E://age
                case SCD_E:
                case SD_E:
                case G_E:
                    AddFeature(fid, uid, 1, false);
                    break;
                case MV_E:
                case MK_E:
                case N_E:
                case I_E:
                default:
                    break;
            }

            //pthread_mutex_unlock(pmutex);
        }
        counter++;
        if(counter % per_print == 0){
            cout << "file[" << file_name << "]:" << counter << endl;
        }
    }

    ifs.close();
}

void ExtractModel(FILE * fp){
    ofstream ofs(out_file);
    if(!ofs.is_open()){
        fprintf(fp, "open out file[%s] failed!\n",out_file);
        return;
    }
    string file_name_path(in_path);
    file_name_path += "/conf/file_name.conf";
    vector<pthread_t *> vthreads;
    ifstream ifs(file_name_path.c_str());
    if(!ifs.is_open()){
        cout << "file_name.conf file open failed:" << file_name_path << endl;
        return;
    }
    string line;
    while(getline(ifs, line)){
        if(line.length() > 0){
            g_vfilenames.push_back(line);
            pthread_t *pt = (pthread_t *)malloc(sizeof(pthread_t));
            pthread_create(pt, NULL, ExtractThread, (void *)(g_vfilenames.size()-1));
            vthreads.push_back(pt);
        }
    }
    for(auto &it: vthreads){
        pthread_join(*it, NULL);
    }

    /*vector<int> vUids;
    for(auto &it:g_mapResFeatures){
        vUids.push_back(it.first);
    }
    sort(vUids.begin(), vUids.end(), cmpIds);*/
    vector<int> vFs;
    for(auto &it:g_mapResFeatures){
        vFs.clear();
        ofs << it.first;
        for(auto &itt:it.second){
            vFs.push_back(itt.first);
        }
        sort(vFs.begin(), vFs.end(), cmpIds);
        for(auto &itf:vFs){
            auto itfp = it.second.find(itf);
            if(itfp != it.second.end()){
                ofs << " " << itf << ":" << itfp->second;//field:feature:value
            }else{
                cout << "never come here error! cant find " << itf << "(fid) in g_mapResFeatures" << endl;
            }
        }
        ofs << endl;
    }
    ifs.close();
    ofs.close();

    string fid_path(out_file);
    fid_path += ".fid";
    ofstream ofs_fid(fid_path.c_str());
    if(!ofs_fid.is_open()){
        cout << "open output fid file failed:" << fid_path << endl;
    }
    //map<string, int> g_mapFids;//<featurename, fid>
    for(auto & it: g_mapFids){
        ofs_fid << it.first << " " << it.second << endl;
    }
    ofs_fid.close();

    /*string uid_path(out_file);
    uid_path += ".uid";
    ofstream ofs_uid(uid_path.c_str());
    if(!ofs_uid.is_open()){
        cout << "open output uid file failed:" << uid_path << endl;
    }
    //    map<string, int> g_mapUids;//<cookie, uid>
    for(auto & it: g_mapUids){
        ofs_uid << it.first << " " << it.second << endl;
    }
    ofs_uid.close();*/

    if(debug_mode >= 2){
        ////map<uid, map<featureid, value>>
        cout << "g_mapResFeatures:" << endl;
        for(auto &it: g_mapResFeatures){
            for(auto &itt: it.second){
                cout << it.first << ":" << itt.first << ":" << itt.second << endl;
            }
        }
    }
    for(auto & it : vthreads){
        free(it);
    }
}
int ArgPos(char *str, int argc, char **argv) {
  int a;
  for (a = 1; a < argc; a++) if (!strcmp(str, argv[a])) {
    if (a == argc - 1) {
      printf("Argument missing for %s\n", str);
      exit(1);
    }
    return a;
  }
  return -1;
}
int main(int argc, char **argv) {
    struct timeval t_start;
    gettimeofday(&t_start, NULL);
    printf("t_start.tv_sec:%d\n", t_start.tv_sec);
    printf("t_start.tv_usec:%d\n", t_start.tv_usec);

    int i;
    if (argc == 1) {
        printf("libsvm(xgboost) predict feature extract toolkit v 1.0\n\n");
        printf("Options:\n");
        printf("Parameters:\n");
        printf("\t-in <path>\n");
        printf("\t\t the input <path> of confgure and data files, make sure there are 'conf' and 'data' folders in this <path>");
        printf("\t-suffix <string>\n");
        printf("\t\t the suffix of the input data files, eg:css/txt");
        printf("\t-fid <file>\n");
        printf("\t\t the <featurename, fid> file");
        printf("\t-clus <file>(optional)\n");
        printf("\t\t The cluster file, formate <filename_columename_value:clusterid>\n");
        printf("\t-out <file>\n");
        printf("\t\t Will output two files: featureid:value file and featureid list file\n");
        printf("\t-debug <int>\n");
        printf("\t\t Set the debug mode (default = 2 = more info during training)\n");
        printf("\nExamples:\n");
        printf("./predict_svm_f -in ./ -suffix txt -clus conf/cluster.conf -fid out/svm_features.txt.fid -out out/svm_features_pre.txt -debug 2\n\n");
        return 0;
    }
    in_path[0] = 0;
    out_file[0] = 0;
    suffix[0] = 0;
    fid_file[0] = 0;
    cluster_file[0] = 0;
    char mode_str[MAX_STRING] = {0};
    if ((i = ArgPos((char *)"-in", argc, argv)) > 0) strcpy(in_path, argv[i + 1]);
    if ((i = ArgPos((char *)"-suffix", argc, argv)) > 0) strcpy(suffix, argv[i + 1]);
    if ((i = ArgPos((char *)"-fid", argc, argv)) > 0) strcpy(fid_file, argv[i + 1]);
    if ((i = ArgPos((char *)"-clus", argc, argv)) > 0) strcpy(cluster_file, argv[i + 1]);
    if ((i = ArgPos((char *)"-out", argc, argv)) > 0) strcpy(out_file, argv[i + 1]);
    if ((i = ArgPos((char *)"-debug", argc, argv)) > 0) debug_mode = atoi(argv[i + 1]);
    if(debug_mode >= 1){
        cout << "parameters:"<< endl
            << "-in:" << in_path << endl
            << "-suffix:" << suffix << endl
            << "-clus:" << cluster_file << endl
            << "-fid:" << fid_file << endl
            << "-out:" << out_file << endl
            << "-debug:" << debug_mode << endl;
    }
    cout << "Initialize ..." << endl;

    InitClusterData();
    InitFids();
    ReadConfFile();

    pmutex = new pthread_mutex_t();
    pthread_mutex_init(pmutex, NULL);

	FILE *flog;
    if((flog=fopen("train.log", "w")) == NULL) {
		fprintf(stderr, "open train.log file failed.\n");
		return EXIT_FAILURE;
	}
    cout << "extracting features..." << endl;
    ExtractModel(flog);

    pthread_mutex_destroy(pmutex);
    delete pmutex;
    pmutex = NULL;

    struct timeval t_end;
    gettimeofday(&t_end, NULL);
    printf("t_start.tv_sec:%d\n", t_start.tv_sec);
    printf("t_start.tv_usec:%d\n", t_start.tv_usec);
    printf("t_end.tv_sec:%d\n", t_end.tv_sec);
    printf("t_end.tv_usec:%d\n", t_end.tv_usec);
    cout << "start time :" << t_start.tv_sec << "." << t_start.tv_usec << endl
        << "end time :" << t_end.tv_sec << "." << t_end.tv_usec << endl;
    if((t_end.tv_usec - t_start.tv_usec) > 0){
        cout << "using time : " << t_end.tv_sec - t_start.tv_sec << "."<< t_end.tv_usec - t_start.tv_usec << " s" << endl;
    }else{
        cout << "using time : " << t_end.tv_sec - t_start.tv_sec - 1 << "."<< 1000000 + t_end.tv_usec - t_start.tv_usec << " s" << endl;
    }
    return 0;
}

