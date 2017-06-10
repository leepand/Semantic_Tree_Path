#include "ParentTree.h"
#include <json/json.h>
#include <sstream>
#include <iostream>
#include <sys/time.h>
#include <string.h>

using namespace std;
using namespace parent_tagging_tree;

#define MAX_STRING 100

char in_file[MAX_STRING], out_file[MAX_STRING],json_conf_file[MAX_STRING],sim_file[MAX_STRING];
int debug_mode = 2, num_threads = 12;
pthread_mutex_t *pmutex = NULL, *pfmutex = NULL;

const long PER_PRINT = 100000l;
long per_print = 100000l;
map<string, string> g_mapUText;//<userid, text>
ParentTree g_tree;


typedef struct _threadParam{
    long index;
    long start_pos;
    long end_pos;
}ThreadParam;

static void split_2(std::string &s, std::vector< std::string >* ret){
    size_t last = 0;
    string space = " ";
    string tab = "\t";
    size_t ispace = s.find_first_of(space,last);
    size_t itab = s.find_first_of(tab,last);
    if(ispace != std::string::npos && itab != std::string::npos  && ispace < itab){
        ret->push_back(s.substr(last, ispace-last));
        last = ispace + space.length();
        if (ispace-last>0)
        {
            ret->push_back(s.substr(last,ispace-last));
        }
    }else if(ispace != std::string::npos && itab != std::string::npos){
        ret->push_back(s.substr(last, itab-last));
        last = itab + tab.length();
        if (itab-last>0)
        {
            ret->push_back(s.substr(last,itab-last));
        }
    }else if(ispace == std::string::npos && itab != std::string::npos){
        ret->push_back(s.substr(last, itab-last));
        last = itab + tab.length();
        if (itab-last>0)
        {
            ret->push_back(s.substr(last,itab-last));
        }
    }else if(ispace != std::string::npos && itab == std::string::npos){
        ret->push_back(s.substr(last, ispace-last));
        last = ispace + space.length();
        if (ispace-last>0)
        {
            ret->push_back(s.substr(last,ispace-last));
        }
    }
    if(ret->size() != 2){
        cout << "split_2::" << ret->size() << endl;
    }
}
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
typedef struct TreeInfo_t{
    string tag;
    vector<string> keys;
    vector<string> child;

} TreeInfo_t;
void ParseTreeNode(vector<TreeInfo_t> &info, string &json){
    info.clear();
    Json::Reader    reader;
    Json::Value     root;
    if(debug_mode >= 2){
        cout << "parsejson::json_in:" << json << endl;
    }
    if (!reader.parse(json, root)){
        cerr << "Exception caught by ParseJson::parse " << endl;
        return;
    }
    try {
        for(int idx = 0; idx < root.size(); ++idx){
            TreeInfo_t ti;
            if(!root[idx]["domain"].isNull()){
                ti.tag = root[idx]["domain"].asString();
                if(debug_mode >= 2){
                    cout << "tag:" << ti.tag << endl;
                }
            }
            if(!root[idx]["concepts"].isNull()){
                for(int i = 0; i < root[idx]["concepts"].size(); ++i){
                    ti.keys.push_back(root[idx]["concepts"][i].asString());
                }
                if(debug_mode >= 2){
                    for(auto &it:ti.keys){
                        cout << "concepts:" << it << endl;
                    }
                }
            }
            if(!root[idx]["children"].isNull()){
                for(int i = 0; i < root[idx]["children"].size(); ++i){
                    ti.child.push_back(root[idx]["children"][i].asString());
                }
                if(debug_mode >= 2){
                    for(auto &it:ti.child){
                        cout << "children:" << it << endl;
                    }
                }
            }
            if(!ti.tag.empty()){
                info.push_back(ti);
            }
        }

    }catch (const std::exception &ex) {
        cerr << "Exception caught by ParseJson: " << ex.what() << endl;
    }
}
void InitializeTree(){
    ifstream ifs(json_conf_file);
    if(!ifs.is_open()){
        cout << "cant open the json config file:" << json_conf_file << endl;
        return;
    }
    string line;
    vector<TreeInfo_t> TreeNodes;
    while(getline(ifs, line)){
        ifstream iifs(line.c_str());
        if(!iifs.is_open()){
            cout << "cant open the json file:" << line << endl;
        }
        std::stringstream buffer;
        buffer << iifs.rdbuf();
        std::string json(buffer.str());
        ParseTreeNode(TreeNodes, json);
        for(auto &it:TreeNodes){
            g_tree.AddNode(it.tag, it.child, it.keys);
        }
        iifs.close();
    }
    ifs.close();

    if(debug_mode >= 2){
        g_tree.print();
    }
}
void InitializeSimKeys(){
    ifstream ifs(sim_file);
    if(!ifs.is_open()){
        cout << "cant open the sim keys file:" << sim_file << endl;
        return;
    }
    string line;
    string delim = "\t";
    vector<string> vec;
    string delim2 = " ";
    vector<string> vec2;
    string delim3 = ":";
    vector<string> vec3;
    while(getline(ifs, line)){
        vec.clear();
        split(line, delim, &vec);
        if(vec.size() >= 2){
            string tag = vec[0];
            vec2.clear();
            split(vec[1], delim2, &vec2);
            for(int i = 0; i < vec2.size(); ++i){
                vec3.clear();
                split(vec2[i], delim3, &vec3);
                if(vec3.size() == 2){
                    g_tree.AddSimKey(tag, vec3[0], atof(vec3[1].c_str()));
                }/*else{
                    cout << "split failed:" << vec2[i] << "[" << vec3.size() << "]" << endl;
                }*/
            }
        }
    }
    ifs.close();
    if(debug_mode >= 2){
        g_tree.printkeys();
    }
}
void Tagging(){
    ifstream ifs(in_file);
    if(!ifs.is_open()){
        cout << "cant open the input file:" << in_file << endl;
        return;
    }
    ofstream ofs(out_file);
    if(!ofs.is_open()){
        cout << "cant open the output file:" << out_file << endl;
        return;
    }
    string line;
    string delim = " ";
    vector<string> vec;
    vector < pair < string, double > > paths;
    map<string, double> mapTag;//<path, wgt>
    while(getline(ifs, line)){
        vec.clear();
        split(line, delim, &vec);
        for(int i = 0; i < vec.size(); ++i){
            paths.clear();
            g_tree.GetKeyPath(paths, vec[i]);
            for(auto &it:paths){
                auto itpath = mapTag.find(it.first);
                if(itpath != mapTag.end()){
                    itpath->second += it.second;
                }else{
                    mapTag[it.first] = it.second;
                }
            }
        }
        bool isfirst = true;
        for(auto &it:mapTag){
            if(isfirst){
                isfirst = false;
            }else{
                ofs << " ";
            }
            ofs << it.first << ":" << it.second;
        }
        ofs << endl;
        mapTag.clear();
    }
    ifs.close();
    ofs.close();
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
        printf("tagging tree toolkit v 1.0\n\n");
        printf("Options:\n");
        printf("Parameters:\n");
        printf("\t-json <file>\n");
        printf("\t\t The input configure <file>, format: <filename with path>\n");
        printf("\t-sim <file>\n");
        printf("\t\t The similar keys <file>, format: <key->key:value key:value...>\n");
        printf("\t-in <file>\n");
        printf("\t\t The input <file>, format: <key key key ...>\n");
        printf("\t-out <file>\n");
        printf("\t\t Will output file, format: <uid tag1:tag2:value:[keys...] ...>\n");
        printf("\t-threads <int>\n");
        printf("\t\tUse <int> threads (default 12) to run the train and predict process\n");
        printf("\t-debug <int>\n");
        printf("\t\t Set the debug mode (default = 2 = more info during training)\n");
        printf("\nExamples:\n");
        printf("./tagging_tree -in in.txt -json data/json.conf -sim data/vec.sim.20 -out out.txt -debug 2\n\n");
        return 0;
    }
    in_file[0] = 0;
    out_file[0] = 0;
    json_conf_file[0] = 0;
    sim_file[0] = 0;
    char mode_str[MAX_STRING] = {0};
    if ((i = ArgPos((char *)"-in", argc, argv)) > 0) strcpy(in_file, argv[i + 1]);
    if ((i = ArgPos((char *)"-json", argc, argv)) > 0) strcpy(json_conf_file, argv[i + 1]);
    if ((i = ArgPos((char *)"-sim", argc, argv)) > 0) strcpy(sim_file, argv[i + 1]);
    if ((i = ArgPos((char *)"-out", argc, argv)) > 0) strcpy(out_file, argv[i + 1]);
    if ((i = ArgPos((char *)"-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);
    if ((i = ArgPos((char *)"-debug", argc, argv)) > 0) debug_mode = atoi(argv[i + 1]);
    if(debug_mode >= 1){
        cout << "parameters:"<< endl
            << "-in:" << in_file << endl
            << "-json:" << json_conf_file << endl
            << "-sim:" << sim_file << endl
             << "-out:" << out_file << endl
            << "-threads:" << num_threads<< endl
             << "-debug:" << debug_mode << endl;
    }

    /*pmutex = new pthread_mutex_t();
    pthread_mutex_init(pmutex, NULL);
    pfmutex = new pthread_mutex_t();
    pthread_mutex_init(pfmutex, NULL);*/
    cout << "parse json files and make the tagging tree" << endl;
    InitializeTree();
    cout << "Initialize similar keys..." << endl;
    InitializeSimKeys();
    cout << "Tagging..." << endl;
    Tagging();
	/*FILE *flog;
    if((flog=fopen("train.log", "w")) == NULL) {
		fprintf(stderr, "open train.log file failed.\n");
		return EXIT_FAILURE;
	}
    cout << "extracting features..." << endl;
    UniqueModel(flog);*/

    /*pthread_mutex_destroy(pmutex);
    delete pmutex;
    pmutex = NULL;
    pthread_mutex_destroy(pfmutex);
    delete pfmutex;
    pfmutex = NULL;*/

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

