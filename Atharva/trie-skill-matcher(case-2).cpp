#include <bits/stdc++.h>
using namespace std;

struct ProviderRecord {
    int id;
    string category;
    int availability;
    double score;
};

struct TrieNode {
    static const int ALPH = 128;
    array<int, ALPH> next;
    vector<int> providers;
    bool terminal;
    TrieNode(){ next.fill(-1); terminal=false; }
};

class Trie {
    vector<TrieNode> nodes;
public:
    Trie(){ nodes.emplace_back(); }
    int new_node(){ nodes.emplace_back(); return (int)nodes.size()-1; }
    void clear(){ nodes.clear(); nodes.emplace_back(); }
    void insert_token(const string &token, int providerIdx){
        int cur = 0;
        for(unsigned char ch : token){
            int c = ch % TrieNode::ALPH;
            if(nodes[cur].next[c] == -1) nodes[cur].next[c] = new_node();
            cur = nodes[cur].next[c];
        }
        nodes[cur].terminal = true;
        nodes[cur].providers.push_back(providerIdx);
    }
    bool erase_token(const string &token, int providerIdx){
        vector<int> path;
        int cur = 0;
        for(unsigned char ch : token){
            int c = ch % TrieNode::ALPH;
            if(nodes[cur].next[c] == -1) return false;
            cur = nodes[cur].next[c];
            path.push_back(cur);
        }
        auto &vec = nodes[cur].providers;
        auto it = find(vec.begin(), vec.end(), providerIdx);
        if(it==vec.end()) return false;
        vec.erase(it);
        if(vec.empty()) nodes[cur].terminal = false;
        return true;
    }
    vector<pair<string, vector<int>>> collect_with_prefix(const string &prefix, int limit=50) const {
        vector<pair<string, vector<int>>> out;
        int cur = 0;
        for(unsigned char ch: prefix){
            int c = ch % TrieNode::ALPH;
            if(nodes[cur].next[c] == -1) return out;
            cur = nodes[cur].next[c];
        }
        string accum = prefix;
        function<void(int)> dfs = [&](int node){
            if((int)out.size() >= limit) return;
            if(nodes[node].terminal) out.emplace_back(accum, nodes[node].providers);
            for(int c=0;c<TrieNode::ALPH;++c){
                int nx = nodes[node].next[c];
                if(nx!=-1){
                    accum.push_back((char)c);
                    dfs(nx);
                    accum.pop_back();
                    if((int)out.size() >= limit) return;
                }
            }
        };
        dfs(cur);
        return out;
    }
    vector<string> variants_for_edit_distance(const string &s) const {
        unordered_set<string> setv;
        string tmp;
        for(size_t i=0;i<=s.size();++i){
            for(char ch='a'; ch<='z'; ++ch){
                tmp = s.substr(0,i) + ch + s.substr(i);
                setv.insert(tmp);
            }
            for(char ch='a'; ch<='z'; ++ch){
                if(i < s.size()){
                    tmp = s.substr(0,i) + s.substr(i+1);
                    setv.insert(tmp);
                }
            }
        }
        vector<string> out(setv.begin(), setv.end());
        return out;
    }
    vector<string> brute_list_all(int limit=1000) const {
        vector<string> out;
        string s;
        function<void(int)> dfs = [&](int node){
            if((int)out.size()>=limit) return;
            if(nodes[node].terminal) out.push_back(s);
            for(int c=0;c<TrieNode::ALPH;++c){
                int nx=nodes[node].next[c];
                if(nx!=-1){ s.push_back((char)c); dfs(nx); s.pop_back(); if((int)out.size()>=limit) return; }
            }
        };
        dfs(0);
        return out;
    }
};

class CSVReader {
public:
    static vector<vector<string>> read(const string &path){
        ifstream in(path);
        vector<vector<string>> rows;
        if(!in.is_open()) return rows;
        string header;
        if(!getline(in, header)) return rows;
        string line;
        while(getline(in, line)){
            vector<string> cols;
            string cur;
            bool inq=false;
            for(char ch : line){
                if(ch=='"'){ inq = !inq; continue; }
                if(ch==',' && !inq){ cols.push_back(cur); cur.clear(); } else cur.push_back(ch);
            }
            cols.push_back(cur);
            rows.push_back(cols);
        }
        return rows;
    }
    static void write(const string &path, const vector<vector<string>> &rows){
        ofstream out(path);
        for(auto &r: rows){
            for(size_t i=0;i<r.size();++i){
                if(i) out<<',';
                bool needq = r[i].find(',')!=string::npos || r[i].find('"')!=string::npos;
                if(needq) out<<'"'<<r[i]<<'"'; else out<<r[i];
            }
            out<<"\n";
        }
    }
};

class ProviderIndex {
public:
    vector<ProviderRecord> providers;
    unordered_map<string, vector<int>> tokenToProviders;
    unordered_map<int,int> idToIndex;
    Trie trie;
    void load_from_csv(const string &path){
        auto rows = CSVReader::read(path);
        providers.clear(); tokenToProviders.clear(); idToIndex.clear(); trie.clear();
        for(size_t i=0;i<rows.size();++i){
            auto &r = rows[i];
            if(r.size() < 4) continue;
            string token = r[0];
            string cat = r[1];
            int pid = stoi(r[2]);
            int avail = stoi(r[3]);
            ProviderRecord rec; rec.id = pid; rec.category = cat; rec.availability = avail; rec.score = avail;
            idToIndex[pid] = (int)providers.size();
            providers.push_back(rec);
            tokenToProviders[token].push_back(rec.id);
            trie.insert_token(token, idToIndex[pid]);
        }
        compute_scores();
    }
    void compute_scores(){
        for(auto &p : providers) p.score = max(1.0, (double)p.availability);
    }
    vector<pair<int,double>> lookup_providers(const string &token){
        vector<pair<int,double>> out;
        auto it = tokenToProviders.find(token);
        if(it!=tokenToProviders.end()){
            for(int pid: it->second){
                int idx = idToIndex[pid];
                out.emplace_back(pid, providers[idx].score);
            }
        }
        return out;
    }
    vector<pair<int,double>> lookup_providers_by_index(const vector<int> &idxs){
        vector<pair<int,double>> out;
        for(int idx: idxs) if(idx>=0 && idx < (int)providers.size()) out.emplace_back(providers[idx].id, providers[idx].score);
        return out;
    }
    vector<pair<int,double>> ranked_aggregate(const vector<pair<string, vector<int>>> &tokenResults, int limit=20){
        unordered_map<int,double> scoreMap;
        for(auto &pr : tokenResults){
            for(int idx : pr.second) {
                if(idx >=0 && idx < (int)providers.size()) scoreMap[providers[idx].id] += providers[idx].score;
            }
        }
        vector<pair<int,double>> out;
        out.reserve(scoreMap.size());
        for(auto &kv : scoreMap) out.emplace_back(kv.first, kv.second);
        sort(out.begin(), out.end(), [](auto &a, auto &b){ if(a.second==b.second) return a.first < b.first; return a.second > b.second; });
        if((int)out.size() > limit) out.resize(limit);
        return out;
    }
    vector<pair<int,double>> search_prefix_ranked(const string &prefix, int limit=20){
        auto res = trie.collect_with_prefix(prefix, limit*5);
        return ranked_aggregate(res, limit);
    }
    vector<pair<int,double>> fuzzy_search(const string &input, int limit=20){
        auto res = trie.collect_with_prefix(input, limit*2);
        if(res.size() < (size_t)limit){
            auto vars = trie.variants_for_edit_distance(input);
            for(auto &v : vars){
                auto more = trie.collect_with_prefix(v, 5);
                for(auto &m : more) res.push_back(m);
                if(res.size() >= (size_t)limit*2) break;
            }
        }
        return ranked_aggregate(res, limit);
    }
    void export_index(const string &outPath){
        vector<vector<string>> rows;
        rows.push_back({"provider_id","category","availability","score"});
        for(auto &p : providers) rows.push_back({to_string(p.id), p.category, to_string(p.availability), to_string(p.score)});
        CSVReader::write(outPath, rows);
    }
    void export_trie_csv(const string &outPath, int limit=1000){
        auto tokens = trie.brute_list_all(limit);
        vector<vector<string>> rows;
        rows.push_back({"token","provider_ids"});
        for(auto &t: tokens){
            string s;
            if(tokenToProviders.find(t)!=tokenToProviders.end()){
                auto &vec = tokenToProviders[t];
                for(size_t i=0;i<vec.size();++i){ if(i) s.push_back(';'); s += to_string(vec[i]); }
            }
            rows.push_back({t, s});
        }
        CSVReader::write(outPath, rows);
    }
};

static string now_iso(){
    auto t = chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(t);
    char buf[64]; strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&tt));
    return string(buf);
}

static void timed_action(function<void()> fn, const string &label){
    auto t1 = chrono::high_resolution_clock::now();
    fn();
    auto t2 = chrono::high_resolution_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(t2-t1).count();
    cerr<<label<<" took "<<ms<<" ms\n";
}

static vector<string> split_ws(const string &s){
    vector<string> out; string cur; stringstream ss(s);
    while(ss>>cur) out.push_back(cur);
    return out;
}

static string to_lower(const string &s){
    string out=s; for(auto &c:out) c=tolower((unsigned char)c); return out;
}

int main(int argc,char**argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(argc < 2){
        cerr<<"Usage: "<<argv[0]<<" <skills-csv>\n";
        return 1;
    }
    string csv = argv[1];
    ProviderIndex idx;
    timed_action([&](){ idx.load_from_csv(csv); }, "Load CSV");
    cout<<"Loaded providers="<<idx.providers.size()<<"\n";
    cout<<"Commands:\nsearch <prefix>\nfuzzy <term>\ninspect <provider_id>\nexport idx <out.csv>\nexport trie <out.csv>\nbench <prefix> <iters>\nstress <n>\nquit\n";
    string line;
    while(true){
        cout<<"> ";
        if(!getline(cin, line)) break;
        if(line.empty()) continue;
        auto parts = split_ws(line);
        if(parts.empty()) continue;
        string cmd = parts[0];
        if(cmd=="quit" || cmd=="exit") break;
        if(cmd=="search"){
            if(parts.size()<2){ cout<<"need prefix\n"; continue; }
            string prefix = to_lower(parts[1]);
            vector<pair<int,double>> out = idx.search_prefix_ranked(prefix, 20);
            for(auto &p : out) cout<<p.first<<","<<p.second<<"\n";
            continue;
        }
        if(cmd=="fuzzy"){
            if(parts.size()<2){ cout<<"need term\n"; continue; }
            string term = to_lower(parts[1]);
            auto out = idx.fuzzy_search(term, 20);
            for(auto &p: out) cout<<p.first<<","<<p.second<<"\n";
            continue;
        }
        if(cmd=="inspect"){
            if(parts.size()<2){ cout<<"need id\n"; continue; }
            int id = stoi(parts[1]);
            if(idx.idToIndex.find(id)==idx.idToIndex.end()){ cout<<"not found\n"; continue; }
            int i = idx.idToIndex[id];
            auto &pr = idx.providers[i];
            cout<<pr.id<<","<<pr.category<<","<<pr.availability<<","<<pr.score<<"\n";
            continue;
        }
        if(cmd=="export"){
            if(parts.size()<3){ cout<<"export idx|trie <out.csv>\n"; continue; }
            string what = parts[1];
            string outp = parts[2];
            if(what=="idx"){ idx.export_index(outp); cout<<"exported idx\n"; }
            else if(what=="trie"){ idx.export_trie_csv(outp, 2000); cout<<"exported trie\n"; }
            else cout<<"unknown export\n";
            continue;
        }
        if(cmd=="bench"){
            if(parts.size()<3){ cout<<"bench <prefix> <iters>\n"; continue; }
            string prefix = to_lower(parts[1]);
            int iters = stoi(parts[2]);
            auto run = [&](){
                for(int i=0;i<iters;++i) idx.search_prefix_ranked(prefix, 50);
            };
            timed_action(run, "bench");
            continue;
        }
        if(cmd=="stress"){
            if(parts.size()<2){ cout<<"stress <n>\n"; continue; }
            int n = stoi(parts[1]);
            mt19937_64 rng((uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count());
            uniform_int_distribution<int> d(1,8);
            auto run = [&](){
                for(int i=0;i<n;++i){
                    int len = d(rng);
                    string s;
                    for(int j=0;j<len;++j) s.push_back('a' + (rng()%26));
                    idx.search_prefix_ranked(s, 10);
                }
            };
            timed_action(run, "stress");
            continue;
        }
        cout<<"unknown\n";
    }
    return 0;
}
