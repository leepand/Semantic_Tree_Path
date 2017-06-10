#ifndef _PARENT_TREE_H
#define _PARENT_TREE_H
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <fstream>
#include <iostream>

extern int debug_mode;
namespace parent_tagging_tree{

using namespace std;

class ParentTree{
    private:
        typedef struct _TreeNode_t{
            struct _TreeNode_t *parent;
            string tag;
            map<string, struct _TreeNode_t *> child;//<tag, TreeNode_t*>
        } TreeNode_t;
        map<string, map<string, pair<TreeNode_t*, double> > > m_keyNodes;//<key, map<tag, pair<TreeNode_t *, weight>>>
        map<string, TreeNode_t*> m_tagNode;//<tag, TreeNode* node> only used when compose the forest
        //TreeNode_t* m_tree;//root
        map<string, TreeNode_t*> m_forest; //<tag, TreeNode* subtree>
        void DeleteNode(TreeNode_t* root){
            if(!root){
                for(auto &it:root->child){
                    DeleteNode(it.second);
                }
                delete(root);
            }
        }
        void _addNodeKeys(TreeNode_t *node, vector<string> &keys, double wgt){
            if(debug_mode >= 1){
                cout << "add node keys:" << keys.size() << endl;
            }
            //map<string, map<string, pair<TreeNode_t*, double> > > m_keyNodes;//<key, map<tag, pair<TreeNode_t *, weight>>>
            for(auto &itk:keys){
                auto itkn = m_keyNodes.find(itk);
                if(itkn != m_keyNodes.end()){
                    auto itag = itkn->second.find(node->tag);
                    if(itag != itkn->second.end()){ //refresh
                        itag->second.first = node;
                        itag->second.second = wgt;
                        //cout << "already exit error, node[" << node->tag << "]'s key[" << itk << "] already exit in m_keyNodes" << endl;
                    }else{//add new
                        itkn->second[node->tag] = pair<TreeNode_t*, double>(node, wgt);
                    }
                    if(debug_mode >= 2){
                        cout << "add key[" << itk << "]=";
                        for(auto &it:itkn->second){
                            cout << it.first << ":" << it.second.second <<"," << endl;
                        }
                    }
                }else{
                    map<string, pair<TreeNode_t*, double> > tmap;
                    tmap[node->tag] = pair<TreeNode_t*, double>(node, wgt);
                    m_keyNodes[itk] = tmap;
                    if(debug_mode >= 2){
                        cout << "add key[" << itk << "]=" << node->tag << ":" << wgt << endl;
                    }
                }
            }
        }
        void _addNodeKeys(TreeNode_t *node, string &key, double wgt){
            //map<string, map<string, pair<TreeNode_t*, double> > > m_keyNodes;//<key, map<tag, pair<TreeNode_t *, weight>>>
            auto itkn = m_keyNodes.find(key);
            if(itkn != m_keyNodes.end()){
                auto itag = itkn->second.find(node->tag);
                if(itag != itkn->second.end()){ //refresh
                    itag->second.first = node;
                    itag->second.second = wgt;
                    //cout << "already exit error, node[" << node->tag << "]'s key[" << itk << "] already exit in m_keyNodes" << endl;
                }else{//add new
                    itkn->second[node->tag] = pair<TreeNode_t*, double>(node, wgt);
                }
            }else{
                map<string, pair<TreeNode_t*, double> > tmap;
                tmap[node->tag] = pair<TreeNode_t*, double>(node, wgt);
                m_keyNodes[key] = tmap;
            }
        }
    public:
        ParentTree(){
            m_forest.clear();
        }
        ~ParentTree(){
            for(auto &it:m_forest){
                DeleteNode(it.second);
            }
            m_forest.clear();
        }
        void print(){
            for(auto &it:m_tagNode){
                cout << it.first << ":";
                TreeNode_t* node = it.second;
                while(node->parent){
                    cout << node->parent->tag << "->";
                    node = node->parent;
                }
                cout << endl;
            }
        }
        void printkeys(){
            //map<string, map<string, pair<TreeNode_t*, double> > > m_keyNodes;//<key, map<tag, pair<TreeNode_t *, weight>>>
            for(auto &it:m_keyNodes){
                cout << "key[" << it.first << "]:";
                for(auto &itm:it.second){
                    cout << itm.first << ":" << itm.second.second << " ";
                }
                cout << endl;
            }
        }
        bool AddNode(string tag, vector<string> &children, vector<string> &keys){
            //cout << "add node :" << tag << endl;
            auto itag = m_tagNode.find(tag);
            if(itag != m_tagNode.end()){//forest already has tag
                //children nodes
                for(auto &ichtag : children){
                    auto itf = m_forest.find(ichtag);
                    auto itch = itag->second->child.find(ichtag);
                    if(itf != m_forest.end() && itch == itag->second->child.end()){//in forest but not in childNodes
                        itag->second->child[ichtag] = itf->second;
                        //_addNodeKeys(itag->second,keys,1.0);
                        m_forest.erase(itf);
                    }else if(itf == m_forest.end() && itch == itag->second->child.end()){ //neither in forest nor in childNodes
                        TreeNode_t *node = new TreeNode_t();
                        node->tag = ichtag;
                        node->parent = itag->second;
                        itag->second->child[ichtag] = node;
                        m_tagNode[ichtag] = node;
                        //_addNodeKeys(node,keys,1.0);
                    }else if(itf != m_forest.end() && itch != itag->second->child.end()){// both in the forest and the childNodes
                        cout << "never here error: find tag[" << ichtag << "] both in the forest and childNodes" << endl;
                    }else{// if(itf == m_forest.end() && itch != itag->second->child.end()){
                        cout << "never here error: tag["<< tag << "]'s child tag[" << ichtag << "] not in the forest but in childNodes, repeat added!" << endl;
                    }
                }
                //keys
                _addNodeKeys(itag->second,keys,1.0);
            }else{ // add new node to the forest
                TreeNode_t *node = new TreeNode_t();
                node->parent = NULL;
                node->tag = tag;
                m_tagNode[tag] = node;
                //children nodes
                for(auto &ichtag : children){
                    auto itf = m_forest.find(ichtag);
                    if(itf != m_forest.end()){//in forest
                        node->child[ichtag] = itf->second;
                        m_forest.erase(itf);
                    }else{ //not in forest
                        TreeNode_t *chnode = new TreeNode_t();
                        chnode->tag = ichtag;
                        chnode->parent = node;
                        node->child[ichtag] = chnode;
                        m_tagNode[ichtag] = chnode;
                    }
                }
                //keys
                _addNodeKeys(node, keys, 1.0);
            }
        }
        bool AddSimKey(string key, string simkey, double wgt){
            //map<string, map<string, pair<TreeNode_t*, double> > > m_keyNodes;//<key, map<tag, pair<TreeNode_t *, weight>>>
            auto it = m_keyNodes.find(key);
            if(it != m_keyNodes.end()){
                map<string, pair<TreeNode_t*, double> > tmap;
                for(auto &itag:it->second){
                    tmap[itag.first] = pair<TreeNode_t*, double>(itag.second.first, wgt);
                }
                m_keyNodes[simkey] = tmap;
                return true;
            }/*else{
                cout << "cant find the main key["<< key <<"] in keylist" << endl;
                return false;
            }*/
        }
        int GetKeyPath(vector<pair<string, double> > &paths, string key){
            //map<string, map<string, pair<TreeNode_t*, double> > > m_keyNodes;//<key, map<tag, pair<TreeNode_t *, weight>>>
            auto it = m_keyNodes.find(key);
            if(it != m_keyNodes.end()){
                stack<string> stk;
                stk.push(key);
                for(auto &itnode:it->second){
                    TreeNode_t *node = itnode.second.first;
                    double wgt = itnode.second.second;
                    stk.push(node->tag);
                    while(node->parent){
                        stk.push(node->parent->tag);
                        node = node->parent;
                    }
                    string path;
                    while(!stk.empty()){
                        path += stk.top();
                        if(stk.size() != 1){
                            path += ":";
                        }
                        stk.pop();
                    }
                    paths.push_back(pair<string, double>(path, wgt));
                    if(!stk.empty()){
                        cout << "errro stack is not empty..." << endl;
                        while(!stk.empty()){
                            stk.pop();
                        }
                    }
                }
            }
            return paths.size();
        }
};
}
#endif//_PARENT_TREE_H
