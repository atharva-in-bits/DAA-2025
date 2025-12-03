#include <bits/stdc++.h>
using namespace std;

using D = double;
const D INF = 1e15;

struct Edge {
    int u;
    int v;
    D w;
    Edge():u(0),v(0),w(0.0){}
    Edge(int a,int b,D c):u(a),v(b),w(c){}
};

static vector<vector<string>> read_csv_raw(const string &path){
    ifstream in(path);
    vector<vector<string>> out;
    if(!in.is_open()) return out;
    string line;
    if(!getline(in,line)) return out;
    while(getline(in,line)){
        vector<string> cols; string cur; bool inq=false;
        for(char c: line){
            if(c=='"'){ inq = !inq; continue; }
            if(c==',' && !inq){ cols.push_back(cur); cur.clear(); } else cur.push_back(c);
        }
        cols.push_back(cur);
        out.push_back(cols);
    }
    return out;
}

struct Loader {
    string path;
    Loader(const string &p=""):path(p){}
    vector<Edge> load(int &n){
        vector<Edge> out; n = 0;
        auto rows = read_csv_raw(path);
        int maxn = -1;
        for(auto &r: rows){
            if(r.size() < 3) continue;
            int u = stoi(r[0]);
            int v = stoi(r[1]);
            D w = stod(r[2]);
            out.emplace_back(u,v,w);
            maxn = max(maxn, max(u,v));
        }
        n = maxn + 1;
        return out;
    }
};

struct Floyd {
    int n;
    vector<vector<D>> dist;
    vector<vector<int>> next;
    Floyd():n(0){}
    void init(int n_){
        n = n_;
        dist.assign(n, vector<D>(n, INF));
        next.assign(n, vector<int>(n, -1));
        for(int i=0;i<n;++i) dist[i][i]=0, next[i][i]=i;
    }
    void load_edges(const vector<Edge> &edges){
        for(auto &e: edges){
            if(e.u>=0 && e.u<n && e.v>=0 && e.v<n){
                if(e.w < dist[e.u][e.v]){
                    dist[e.u][e.v] = e.w;
                    next[e.u][e.v] = e.v;
                }
            }
        }
    }
    void compute(){
        for(int k=0;k<n;++k){
            for(int i=0;i<n;++i){
                if(dist[i][k] >= INF) continue;
                for(int j=0;j<n;++j){
                    D nd = dist[i][k] + dist[k][j];
                    if(nd < dist[i][j]){
                        dist[i][j] = nd;
                        next[i][j] = next[i][k];
                    }
                }
            }
        }
    }
    vector<int> path(int u,int v){
        vector<int> out;
        if(u<0||v<0||u>=n||v>=n) return out;
        if(next[u][v] == -1) return out;
        int cur = u;
        out.push_back(cur);
        while(cur != v){
            cur = next[cur][v];
            if(cur == -1) return {};
            out.push_back(cur);
        }
        return out;
    }
    D allpairs_sum(){
        D s = 0;
        for(int i=0;i<n;++i) for(int j=0;j<n;++j) if(dist[i][j] < INF) s += dist[i][j];
        return s;
    }
    void export_matrix_csv(const string &path){
        ofstream out(path);
        out<<fixed<<setprecision(6);
        for(int i=0;i<n;++i){
            for(int j=0;j<n;++j){
                if(j) out<<",";
                if(dist[i][j] >= INF/2) out<<"INF"; else out<<dist[i][j];
            }
            out<<"\n";
        }
        out.close();
    }
    void export_next_csv(const string &path){
        ofstream out(path);
        out<<"u,v,next\n";
        for(int i=0;i<n;++i) for(int j=0;j<n;++j) if(dist[i][j] < INF){
            out<<i<<","<<j<<","<<next[i][j]<<"\n";
        }
        out.close();
    }
    vector<int> reachable_from(int u){
        vector<int> out;
        if(u<0||u>=n) return out;
        for(int v=0;v<n;++v) if(dist[u][v] < INF && u!=v) out.push_back(v);
        return out;
    }
};

struct BlockManager {
    int n;
    int blockSize;
    int blocks;
    vector<vector<vector<D>>> blockDist;
    BlockManager():n(0),blockSize(0),blocks(0){}
    void init(int n_, int bSize){
        n = n_;
        blockSize = max(1, bSize);
        blocks = (n + blockSize - 1) / blockSize;
        blockDist.assign(blocks, vector<vector<D>>(blocks, vector<D>(blockSize*blockSize, INF)));
    }
    pair<int,int> idx2blk(int idx){ return { idx / blockSize, idx % blockSize }; }
};

struct GraphOps {
    int n;
    vector<Edge> edges;
    vector<vector<D>> baseDist;
    GraphOps():n(0){}
    void load(const vector<Edge> &e, int n_){
        edges = e; n = n_;
        baseDist.assign(n, vector<D>(n, INF));
        for(int i=0;i<n;++i) baseDist[i][i] = 0;
        for(auto &ed: edges) if(ed.u>=0 && ed.v>=0 && ed.u<n && ed.v<n) baseDist[ed.u][ed.v] = min(baseDist[ed.u][ed.v], ed.w);
    }
    vector<vector<D>> incremental_apsp(){
        vector<vector<D>> dist = baseDist;
        for(int k=0;k<n;++k){
            for(int i=0;i<n;++i){
                if(dist[i][k] >= INF) continue;
                for(int j=0;j<n;++j){
                    D nd = dist[i][k] + dist[k][j];
                    if(nd < dist[i][j]) dist[i][j] = nd;
                }
            }
        }
        return dist;
    }
    vector<vector<D>> blockwise_apsp(int blockSize){
        int B = (n + blockSize - 1)/blockSize;
        vector<vector<D>> dist = baseDist;
        for(int b=0;b<B;++b){
            int kb = b*blockSize;
            int kend = min(n, kb + blockSize);
            for(int i=kb;i<kend;++i) for(int j=kb;j<kend;++j) if(dist[i][j] > dist[i][j]){}
            for(int k=kb;k<kend;++k){
                for(int i=0;i<n;++i){
                    if(dist[i][k] >= INF) continue;
                    for(int j=0;j<n;++j){
                        D nd = dist[i][k] + dist[k][j];
                        if(nd < dist[i][j]) dist[i][j] = nd;
                    }
                }
            }
        }
        return dist;
    }
    vector<int> nodes_sorted_by_centrality(){
        vector<D> sumd(n, 0.0);
        auto Dmat = incremental_apsp();
        for(int i=0;i<n;++i) for(int j=0;j<n;++j) if(Dmat[i][j] < INF) sumd[i] += Dmat[i][j];
        vector<int> idx(n); iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int a,int b){ return sumd[a] < sumd[b]; });
        return idx;
    }
};

struct IO {
    static vector<pair<int,int>> read_pairs_csv(const string &path){
        vector<pair<int,int>> out;
        auto rows = read_csv_raw(path);
        for(auto &r: rows) if(r.size() >= 2) out.emplace_back(stoi(r[0]), stoi(r[1]));
        return out;
    }
    static void write_paths_csv(const string &path, const vector<tuple<int,int,D,vector<int>>> &rows){
        ofstream out(path);
        out<<"s,t,cost,path\n";
        for(auto &t: rows){
            int s,tv; D cost; vector<int> p;
            tie(s,tv,cost,p) = t;
            out<<s<<","<<tv<<","<<cost<<",\"";
            for(size_t i=0;i<p.size();++i){ if(i) out<<"-"; out<<p[i]; }
            out<<"\"\n";
        }
        out.close();
    }
};

struct Bench {
    static long long now(){ return chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count(); }
    static void timeit(function<void()> f, const string &label){
        long long s = now(); f(); long long e = now(); cerr<<label<<" "<<(e-s)<<" ms\n";
    }
};

struct Controller {
    Floyd fw;
    GraphOps ops;
    int n;
    Controller():n(0){}
    void load_graph(const string &csv){
        Loader L(csv);
        int nn; auto E = L.load(nn);
        ops.load(E, nn);
        n = nn;
        fw.init(n);
        fw.load_edges(E);
    }
    void compute_all(){
        fw.compute();
    }
    vector<tuple<int,int,D,vector<int>>> batch_paths(const vector<pair<int,int>> &pairs){
        vector<tuple<int,int,D,vector<int>>> out;
        for(auto &pr: pairs){
            int s = pr.first, t = pr.second;
            if(s<0||s>=n||t<0||t>=n){ out.emplace_back(s,t,INF,vector<int>()); continue; }
            D cost = fw.dist[s][t];
            auto p = fw.path(s,t);
            out.emplace_back(s,t,cost,p);
        }
        return out;
    }
    void export_all(const string &matrixOut, const string &nextOut){
        fw.export_matrix_csv(matrixOut);
        fw.export_next_csv(nextOut);
    }
    vector<int> central_nodes(int k){
        auto v = ops.nodes_sorted_by_centrality();
        if(k < 0 || k > (int)v.size()) return v;
        v.resize(k);
        return v;
    }
    vector<int> reachable_from(int u){ return fw.reachable_from(u); }
    void update_edge_and_recompute(int u,int v,D w){
        bool found=false;
        for(auto &e: ops.edges) if(e.u==u && e.v==v){ e.w = w; found=true; break; }
        if(!found) ops.edges.emplace_back(u,v,w);
        ops.load(ops.edges, ops.n);
        fw.init(n);
        fw.load_edges(ops.edges);
        fw.compute();
    }
    void batch_update_edges(const vector<Edge> &additions){
        for(auto &e: additions) ops.edges.push_back(e);
        ops.load(ops.edges, ops.n);
        fw.init(n);
        fw.load_edges(ops.edges);
        fw.compute();
    }
};

static vector<string> split_ws(const string &s){
    vector<string> out; string t; stringstream ss(s);
    while(ss>>t) out.push_back(t);
    return out;
}

int main(int argc,char**argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(argc < 2){ cerr<<"Usage: "<<argv[0]<<" <edges.csv>\n"; return 1; }
    string csv = argv[1];
    Controller ctl;
    Bench::timeit([&](){ ctl.load_graph(csv); }, "load_graph");
    Bench::timeit([&](){ ctl.compute_all(); }, "floyd_compute");
    cout<<"nodes="<<ctl.n<<" edges="<<ctl.ops.edges.size()<<"\n";
    cout<<"commands:\npath s t\nbatchpairs file out.csv\nexport matrix.csv next.csv\ncentral k\nreachable u\nupdate u v w\nbatchupdate file\nnearest s k\nsum\nexit\n";
    string line;
    while(true){
        cout<<"> ";
        if(!getline(cin,line)) break;
        if(line.empty()) continue;
        auto parts = split_ws(line);
        if(parts.empty()) continue;
        string cmd = parts[0];
        if(cmd=="exit" || cmd=="quit") break;
        if(cmd=="path"){
            if(parts.size()<3){ cout<<"path s t\n"; continue; }
            int s = stoi(parts[1]), t = stoi(parts[2]);
            auto p = ctl.fw.path(s,t);
            if(p.empty()) cout<<"no path\n"; else { cout<<"cost="<<ctl.fw.dist[s][t]<<" path:"; for(size_t i=0;i<p.size();++i){ if(i) cout<<"-"; cout<<p[i]; } cout<<"\n"; }
            continue;
        }
        if(cmd=="batchpairs"){
            if(parts.size()<3){ cout<<"batchpairs file out.csv\n"; continue; }
            string inFile = parts[1], outFile = parts[2];
            auto pairs = IO::read_pairs_csv(inFile);
            auto res = ctl.batch_paths(pairs);
            IO::write_paths_csv(outFile, res);
            cout<<"wrote "<<outFile<<"\n";
            continue;
        }
        if(cmd=="export"){
            if(parts.size()<3){ cout<<"export matrix.csv next.csv\n"; continue; }
            ctl.export_all(parts[1], parts[2]);
            cout<<"exported\n";
            continue;
        }
        if(cmd=="central"){
            int k=10; if(parts.size()>1) k = stoi(parts[1]);
            auto v = ctl.central_nodes(k);
            for(auto x: v) cout<<x<<"\n";
            continue;
        }
        if(cmd=="reachable"){
            if(parts.size()<2){ cout<<"reachable u\n"; continue; }
            int u = stoi(parts[1]);
            auto v = ctl.reachable_from(u);
            for(auto x: v) cout<<x<<"\n";
            continue;
        }
        if(cmd=="update"){
            if(parts.size()<4){ cout<<"update u v w\n"; continue; }
            int u = stoi(parts[1]), v = stoi(parts[2]); D w = stod(parts[3]);
            Bench::timeit([&](){ ctl.update_edge_and_recompute(u,v,w); }, "update_recompute");
            cout<<"updated\n";
            continue;
        }
        if(cmd=="batchupdate"){
            if(parts.size()<2){ cout<<"batchupdate file\n"; continue; }
            string f = parts[1];
            auto rows = read_csv_raw(f);
            vector<Edge> add;
            for(auto &r: rows) if(r.size()>=3) add.emplace_back(stoi(r[0]), stoi(r[1]), stod(r[2]));
            Bench::timeit([&](){ ctl.batch_update_edges(add); }, "batchupdate");
            cout<<"batch updated\n";
            continue;
        }
        if(cmd=="nearest"){
            if(parts.size()<3){ cout<<"nearest s k\n"; continue; }
            int s = stoi(parts[1]), k = stoi(parts[2]);
            vector<pair<D,int>> nodes;
            for(int i=0;i<ctl.n;++i) if(i!=s && ctl.fw.dist[s][i] < INF) nodes.push_back({ctl.fw.dist[s][i], i});
            sort(nodes.begin(), nodes.end());
            for(int i=0;i< (int)nodes.size() && i<k; ++i) cout<<nodes[i].second<<","<<nodes[i].first<<"\n";
            continue;
        }
        if(cmd=="sum"){
            cout<<"allpairs_sum="<<ctl.fw.allpairs_sum()<<"\n";
            continue;
        }
        cout<<"unknown\n";
    }
    return 0;
}
