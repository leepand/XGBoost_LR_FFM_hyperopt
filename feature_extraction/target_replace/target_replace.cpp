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

char in_file[MAX_STRING], base_file[MAX_STRING], out_file[MAX_STRING];
int debug_mode = 2;
map<string, string> g_mapFeatures;//<cookie, features>

static void split(std::string& s, std::string& delim,std::vector< std::string >* ret)
{
    size_t last = 0;
    size_t index=s.find_first_of(delim,last);
    while (index!=std::string::npos)
    {
        ret->push_back(s.substr(last,index-last));
        //last=index+1;
        last = index + delim.length();
        index=s.find_first_of(delim,last);
    }
    if (index-last>0)
    {
        ret->push_back(s.substr(last,index-last));
    }
}

void InitUserFeatures(){
    ifstream ifs(base_file);
    if(!ifs.is_open()){
        cout<< "open base file failed: " << base_file << endl;
        return;
    }
    string line;
    string delim = " ";
    vector<string> vec;
    stringstream ss;
    string fs;
    while(getline(ifs,line)){
        vec.clear();
        split(line, delim, &vec);
        if(vec.size() == 0){
            cout << "base file split failed: vec.size = " << vec.size() << endl;
            continue;
        }
        ss.clear();
        ss.str("");
        if(debug_mode >= 2){
            cout << "vec.size = " << vec.size() << endl;
        }
        for(int i = 1; i < vec.size(); ++i){
            if(i != 1){
                ss << " ";
            }
            ss << vec[i];
        }
        fs = "";
        fs = ss.str();
        g_mapFeatures[vec[0]] = fs;
    }
    ifs.close();
}
void CompFeatures(){
    ifstream ifs(in_file);
    if(!ifs.is_open()){
        cout<< "open input file failed: " << in_file << endl;
        return;
    }
    ofstream ofs(out_file);
    if(!ofs.is_open()){
        cout<< "open output file failed: " << out_file << endl;
        return;
    }
    string out_user_file(out_file);
    out_user_file += ".uid";
    ofstream ofs_u(out_user_file.c_str());
    if(!ofs_u.is_open()){
        cout<< "open user output file failed: " << out_user_file << endl;
        return;
    }
    string line;
    string delim = "\t";
    vector<string> vec;
    while(getline(ifs,line)){
        vec.clear();
        split(line, delim, &vec);
        if(vec.size() != 5){
            cout << "input file split failed: vec.size = " << vec.size() << endl;
            continue;
        }
        auto itf = g_mapFeatures.find(vec[0]);
        if(itf != g_mapFeatures.end()){
            ofs << vec[3] << " " << itf->second << endl;
            ofs_u << vec[0] << endl;
        }
    }
    ifs.close();
    ofs.close();
    ofs_u.close();
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
        printf("ffm input data  toolkit v 1.0\n\n");
        printf("Options:\n");
        printf("Parameters:\n");
        printf("\t-base <file>\n");
        printf("\t\tthe <usercookie fieldid:featureid:value or featureid:value...> <file>\n\n");
        printf("\t-in <file>\n");
        printf("\t\tthe <usercookie->province->city->flag1->flag2> <file>\n\n");
        printf("\t-out <file>\n");
        printf("\t\tthe output <file> <flag1 fieldid:featureid:value fieldid:featureid:value...> and <file>.uid for user's cookie\n\n");
        printf("\t-debug <int>\n");
        printf("\t\tSet the debug mode (default = 2 = more info during training)\n\n");
        printf("\nExamples:\n");
        printf("./ffm_input -base data/ffm_features.txt -in data/flag_car_sample.txt -out out/ffm_features.txt -debug 2\n\n");
        return 0;
    }
    in_file[0] = 0;
    out_file[0] = 0;
    base_file[0] = 0;
    char mode_str[MAX_STRING] = {0};
    if ((i = ArgPos((char *)"-in", argc, argv)) > 0) strcpy(in_file, argv[i + 1]);
    if ((i = ArgPos((char *)"-base", argc, argv)) > 0) strcpy(base_file, argv[i + 1]);
    if ((i = ArgPos((char *)"-out", argc, argv)) > 0) strcpy(out_file, argv[i + 1]);
    if ((i = ArgPos((char *)"-debug", argc, argv)) > 0) debug_mode = atoi(argv[i + 1]);
    if(debug_mode >= 1){
        cout << "parameters:"<< endl
            << "-in:" << in_file << endl
            << "-base:" << base_file << endl
            << "-out:" << out_file << endl
            << "-debug:" << debug_mode << endl;
    }
    cout << "Initialize ..." << endl;

    InitUserFeatures();
    CompFeatures();

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

