#include <bits/stdc++.h>
using namespace std;
using ll = long long;
const double INF_D = 1e300;

struct Edge {
    int u, v;
    double cost;
    int cap;
    Edge(): u(-1), v(-1), cost(0.0), cap(0) {}
    Edge(int a,int b,double c,int p): u(a), v(b), cost(c), cap(p) {}
};

struct Graph {
    int n;
    vector<vector<pair<int,int>>> adj;
    vector<Edge> edges;
    Graph(): n(0) {}
    void init(int N){ n=N; adj.assign(n,{}); edges.clear(); }
    void addEdge(int u,int v,double cost,int cap){
        if(u<0||v<0) return;
        while(max(u,v) >= n){ adj.resize(n+1); n = adj.size(); }
        edges.emplace_back(u,v,cost,cap);
        int id = edges.size()-1;
        if((int)adj.size() <= max(u,v)) adj.resize(max(u,v)+1);
        adj[u].push_back({v,id});
        adj[v].push_back({u,id});
    }
};

struct Feedstock {
    int id;
    double sx, sy;
    double mass;
    double energy_density;
    double pickup_cost;
    int digester_id;
    int timeslot;
    Feedstock(): id(0), sx(0), sy(0), mass(0), energy_density(0), pickup_cost(0), digester_id(0), timeslot(0) {}
    double energy() const { return mass * energy_density; }
    double value_density() const { if(mass <= 0) return 0; return energy() / mass; }
};

struct Digester {
    int id;
    double capacity;
    Digester(): id(0), capacity(0) {}
};

struct SegTree {
    int n;
    vector<double> seg;
    SegTree(): n(0) {}
    void build(int N){
        n = 1;
        while(n < N) n <<= 1;
        seg.assign(2*n, 0.0);
    }
    void setVal(int idx, double val){
        if(idx < 0) return;
        int p = n + idx;
        if(p >= (int)seg.size()) return;
        seg[p] = val;
        p >>= 1;
        while(p >= 1){
            seg[p] = max(seg[2*p], seg[2*p+1]);
            p >>= 1;
        }
    }
    double rangeMax(int l, int r){
        if(l > r) return 0.0;
        double ans = 0.0;
        l += n; r += n;
        while(l <= r){
            if(l & 1) ans = max(ans, seg[l++]);
            if(!(r & 1)) ans = max(ans, seg[r--]);
            l >>= 1; r >>= 1;
        }
        return ans;
    }
};

struct Fenwick {
    int n;
    vector<double> bit;
    Fenwick(): n(0) {}
    void init(int N){ n=N; bit.assign(n+1,0.0); }
    void add(int idx, double val){ idx++; while(idx<=n){ bit[idx]+=val; idx += idx & -idx; } }
    double sumPrefix(int idx){ idx++; double s=0.0; while(idx>0){ s += bit[idx]; idx -= idx & -idx; } return s; }
    double rangeSum(int l,int r){ if(r<l) return 0.0; return sumPrefix(r) - (l?sumPrefix(l-1):0.0); }
};

static inline vector<string> split_csv_line(const string &s){
    vector<string> out;
    string cur;
    bool inq=false;
    for(size_t i=0;i<s.size();++i){
        char c = s[i];
        if(inq){
            if(c=='"'){
                if(i+1 < s.size() && s[i+1] == '"'){ cur.push_back('"'); ++i; }
                else inq = false;
            } else cur.push_back(c);
        } else {
            if(c==','){ out.push_back(cur); cur.clear(); }
            else if(c=='"') inq = true;
            else cur.push_back(c);
        }
    }
    out.push_back(cur);
    for(auto &t: out){
        size_t a=0,b=t.size();
        while(a<b && isspace((unsigned char)t[a])) ++a;
        while(b>a && isspace((unsigned char)t[b-1])) --b;
        t = t.substr(a, b-a);
    }
    return out;
}

static inline int to_int(const string &s){ if(s.empty()) return 0; try{return stoi(s);}catch(...){return 0;} }
static inline double to_double(const string &s){ if(s.empty()) return 0.0; try{return stod(s);}catch(...){return 0.0;} }
static inline long long to_ll(const string &s){ if(s.empty()) return 0; try{return stoll(s);}catch(...){return 0;} }

struct Dijkstra {
    Graph *g;
    Dijkstra(Graph *gr=nullptr): g(gr) {}
    vector<double> run(int src){
        int n = g->n;
        vector<double> dist(n, INF_D);
        if(src < 0 || src >= n) return dist;
        using P = pair<double,int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        dist[src] = 0.0;
        pq.push({0.0, src});
        while(!pq.empty()){
            auto p = pq.top(); pq.pop();
            double d = p.first; int u = p.second;
            if(d > dist[u] + 1e-12) continue;
            for(auto &pr : g->adj[u]){
                int v = pr.first; int eid = pr.second;
                Edge &e = g->edges[eid];
                double w = e.cost; 
                if(dist[u] + w < dist[v]){
                    dist[v] = dist[u] + w;
                    pq.push({dist[v], v});
                }
            }
        }
        return dist;
    }
};

struct BellmanFord {
    int n;
    vector<double> dist;
    BellmanFord(): n(0) {}
    void init(int N){ n=N; dist.assign(n, 0.0); }
    bool detect_negative_cycle(const vector<tuple<int,int,double>> &edges){
        if(n==0) return false;
        dist.assign(n, 0.0);
        for(int it=0; it<n-1; ++it){
            bool any=false;
            for(auto &t: edges){
                int u,v; double w; tie(u,v,w)=t;
                if(u<0||v<0||u>=n||v>=n) continue;
                if(dist[u] + w < dist[v]){ dist[v] = dist[u] + w; any=true; }
            }
            if(!any) break;
        }
        for(auto &t: edges){
            int u,v; double w; tie(u,v,w)=t;
            if(u<0||v<0||u>=n||v>=n) continue;
            if(dist[u] + w < dist[v]) return true;
        }
        return false;
    }
};

struct WTEManager {
    Graph g;
    vector<Feedstock> items;
    vector<Digester> digesters;
    vector<pair<int,double>> timeslot_caps;
    SegTree timeslot_tree;
    Fenwick fen_mass;
    WTEManager(){}
    void load_csv(const vector<string> &lines){
        if(lines.empty()) return;
        int first = 0;
        while(first < (int)lines.size() && lines[first].find(',')==string::npos) ++first;
        if(first >= (int)lines.size()) return;
        vector<string> header = split_csv_line(lines[first]);
        unordered_map<string,int> idx;
        for(int i=0;i<(int)header.size(); ++i){ string k = header[i]; for(auto &c:k) c=tolower((unsigned char)c); idx[k]=i; }
        for(int i=first+1;i<(int)lines.size(); ++i){
            string row = lines[i];
            if(row.find_first_not_of(" \t\r\n") == string::npos) continue;
            auto f = split_csv_line(row);
            string cmd = "";
            if(idx.count("command")) cmd = f[idx["command"]];
            if(cmd.empty()){
                if(idx.count("u") && idx.count("v") && idx.count("transport_cost")) cmd = "EDGE";
            }
            string up = cmd;
            for(auto &c: up) c = toupper((unsigned char)c);
            if(up == "INIT"){
                int n = to_int(f[idx.count("u")?idx["u"]:0]);
                int m = to_int(f[idx.count("v")?idx["v"]:1]);
                if(n <= 0) n = max(0, n);
                g.init(n);
            } else if(up == "EDGE"){
                int u = to_int(f[idx.count("u")?idx["u"]:1]);
                int v = to_int(f[idx.count("v")?idx["v"]:2]);
                double cost = to_double(f[idx.count("transport_cost")?idx["transport_cost"]:3]);
                int cap = to_int(f[idx.count("line_capacity")?idx["line_capacity"]:4]);
                if(g.n == 0){
                    int maxnode = max(u,v);
                    g.init(maxnode+1);
                }
                g.addEdge(u,v,cost,cap);
            } else if(up == "DIGESTER"){
                int did = to_int(f[idx.count("digester_id")?idx["digester_id"]:11]);
                double cap = to_double(f[idx.count("timeslot_capacity")?idx["timeslot_capacity"]:13]);
                Digester d; d.id = did; d.capacity = cap;
                if(did >= (int)digesters.size()) digesters.resize(did+1);
                digesters[did] = d;
            } else if(up == "TIMESLOT"){
                int slot = to_int(f[idx.count("time_slot")?idx["time_slot"]:12]);
                double cap = to_double(f[idx.count("timeslot_capacity")?idx["timeslot_capacity"]:13]);
                timeslot_caps.emplace_back(slot, cap);
            } else if(up == "FEED"){
                Feedstock it;
                it.id = to_int(f[idx.count("item_id")?idx["item_id"]:5]);
                it.sx = to_double(f[idx.count("source_x")?idx["source_x"]:6]);
                it.sy = to_double(f[idx.count("source_y")?idx["source_y"]:7]);
                it.mass = to_double(f[idx.count("mass")?idx["mass"]:8]);
                it.energy_density = to_double(f[idx.count("energy_density")?idx["energy_density"]:9]);
                it.pickup_cost = to_double(f[idx.count("pickup_cost")?idx["pickup_cost"]:10]);
                it.digester_id = to_int(f[idx.count("digester_id")?idx["digester_id"]:11]);
                it.timeslot = to_int(f[idx.count("time_slot")?idx["time_slot"]:12]);
                items.push_back(it);
            } else {
                // ignore unknown
            }
        }
        int slots = 0;
        for(auto &p: timeslot_caps) slots = max(slots, p.first+1);
        if(slots <= 0) slots = max(1, (int)timeslot_caps.size());
        timeslot_tree.build(max(1, slots));
        fen_mass.init(max(1, (int)items.size()+5));
        for(int i=0;i<(int)items.size(); ++i) fen_mass.add(i, items[i].mass);
    }
    vector<int> greedy_knapsack_batch(int digester_id, double capacity_limit){
        vector<int> result;
        vector<pair<double,int>> order;
        for(int i=0;i<(int)items.size(); ++i){
            if(items[i].digester_id != digester_id) continue;
            double kd = items[i].value_density();
            order.emplace_back(-kd, i);
        }
        sort(order.begin(), order.end());
        double cur = 0.0;
        for(auto &p: order){
            int idx = p.second;
            if(cur + items[idx].mass <= capacity_limit){
                result.push_back(idx);
                cur += items[idx].mass;
            }
        }
        return result;
    }
    vector<vector<int>> batch_all_digesters(){
        vector<vector<int>> all;
        for(int d=0; d<(int)digesters.size(); ++d){
            double cap = digesters[d].capacity;
            if(cap <= 0) { all.emplace_back(); continue; }
            auto sel = greedy_knapsack_batch(d, cap);
            all.push_back(sel);
        }
        return all;
    }
    vector<int> dijkstra_route(int src, int dst){
        Dijkstra dj(&g);
        auto dist = dj.run(src);
        int n = g.n;
        if(dst < 0 || dst >= n) return {};
        vector<int> prev(n, -1);
        vector<double> d = dist;
        // For path reconstruction we run Dijkstra with prev tracking
        using P = pair<double,int>;
        vector<double> dd(n, INF_D);
        priority_queue<P, vector<P>, greater<P>> pq;
        dd[src] = 0; pq.push({0, src});
        while(!pq.empty()){
            auto p = pq.top(); pq.pop();
            double cd = p.first; int u = p.second;
            if(cd > dd[u] + 1e-12) continue;
            for(auto &pr: g.adj[u]){
                int v = pr.first; int eid = pr.second;
                double w = g.edges[eid].cost;
                if(dd[u] + w < dd[v]){
                    dd[v] = dd[u] + w;
                    prev[v] = u;
                    pq.push({dd[v], v});
                }
            }
        }
        if(!isfinite(dd[dst])) return {};
        vector<int> path;
        int cur = dst;
        while(cur != -1){ path.push_back(cur); if(cur == src) break; cur = prev[cur]; }
        reverse(path.begin(), path.end());
        return path;
    }
    bool check_timeslot_capacity(const vector<int> &batch){
        unordered_map<int,double> agg;
        for(int idx: batch){
            int slot = items[idx].timeslot;
            agg[slot] += items[idx].mass;
        }
        for(auto &p: agg){
            int slot = p.first;
            double used = p.second;
            double limit = 0.0;
            for(auto &q: timeslot_caps) if(q.first == slot) limit = q.second;
            double curmax = timeslot_tree.rangeMax(slot, slot);
            if(used + curmax > limit + 1e-9) return false;
        }
        return true;
    }
    void commit_batch(const vector<int> &batch){
        unordered_map<int,double> agg;
        for(int idx: batch){
            int slot = items[idx].timeslot;
            agg[slot] += items[idx].mass;
        }
        for(auto &p: agg){
            int slot = p.first;
            double used = p.second;
            double prev = timeslot_tree.rangeMax(slot, slot);
            timeslot_tree.setVal(slot, prev + used);
        }
    }
    bool detect_negative_cycle_pricing(){
        vector<tuple<int,int,double>> ed;
        int N = g.n;
        for(auto &e: g.edges){
            double w = e.cost;
            ed.emplace_back(e.u, e.v, w);
            ed.emplace_back(e.v, e.u, w);
        }
        BellmanFord bf; bf.init(N);
        return bf.detect_negative_cycle(ed);
    }
};

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    vector<string> lines;
    string row;
    while(getline(cin, row)) lines.push_back(row);
    WTEManager mgr;
    mgr.load_csv(lines);
    cout << "Loaded graph nodes=" << mgr.g.n << " edges=" << mgr.g.edges.size() << "\n";
    cout << "Loaded items=" << mgr.items.size() << " digesters=" << mgr.digesters.size() << " timeslots=" << mgr.timeslot_caps.size() << "\n";
    auto batches = mgr.batch_all_digesters();
    int count_nonempty = 0;
    for(auto &b: batches) if(!b.empty()) ++count_nonempty;
    cout << "Prepared batches for " << count_nonempty << " digesters\n";
    bool neg = mgr.detect_negative_cycle_pricing();
    cout << "Negative cycle in pricing graph: " << (neg ? "YES" : "NO") << "\n";
    cout << "Interactive commands:\n";
    cout << "  RUN_BATCH digester_id  -- run greedy knapsack and commit if timeslots OK\n";
    cout << "  ROUTE src dst  -- run Dijkstra and print path\n";
    cout << "  CHECK_SLOT slot  -- print timeslot current usage and limit\n";
    cout << "  FENWICK_SUM l r  -- sum masses in items index range\n    ";
    cout << "  QUIT\n";
    string cmdline;
    while(true){
        if(!getline(cin, cmdline)) break;
        if(cmdline.find_first_not_of(" \t\r\n") == string::npos) continue;
        stringstream ss(cmdline);
        string cmd; ss >> cmd;
        for(auto &c:cmd) c = toupper((unsigned char)c);
        if(cmd == "QUIT") break;
        else if(cmd == "RUN_BATCH"){
            int did; if(!(ss >> did)){ cout << "Usage: RUN_BATCH digester_id\n"; continue; }
            if(did < 0 || did >= (int)mgr.digesters.size()){ cout << "Invalid digester\n"; continue; }
            double cap = mgr.digesters[did].capacity;
            auto batch = mgr.greedy_knapsack_batch(did, cap);
            cout << "Greedy batch size " << batch.size() << " total mass ";
            double totmass = 0;
            for(int idx: batch) totmass += mgr.items[idx].mass;
            cout << totmass << " / cap " << cap << "\n";
            if(mgr.check_timeslot_capacity(batch)){
                mgr.commit_batch(batch);
                cout << "Batch committed to timeslots\n";
            } else cout << "Batch violates timeslot capacity, not committed\n";
        } else if(cmd == "ROUTE"){
            int a,b; if(!(ss >> a >> b)){ cout << "Usage: ROUTE src dst\n"; continue; }
            auto path = mgr.dijkstra_route(a,b);
            if(path.empty()) cout << "No path\n"; else { cout << "Path: "; for(size_t i=0;i<path.size(); ++i){ if(i) cout<<"->"; cout<<path[i]; } cout<<"\n"; }
        } else if(cmd == "CHECK_SLOT"){
            int s; if(!(ss >> s)){ cout<<"Usage: CHECK_SLOT slot\n"; continue; }
            double cur = mgr.timeslot_tree.rangeMax(s,s);
            double limit = 0;
            for(auto &p: mgr.timeslot_caps) if(p.first == s) limit = p.second;
            cout << "Slot " << s << " usage " << cur << " limit " << limit << "\n";
        } else if(cmd == "FENWICK_SUM"){
            int l,r; if(!(ss>>l>>r)){ cout<<"Usage: FENWICK_SUM l r\n"; continue; }
            cout << "Sum mass ["<<l<<","<<r<<"] = " << mgr.fen_mass.rangeSum(l,r) << "\n";
        } else cout << "Unknown command\n";
    }
    cout << "Exiting\n";
    return 0;
}
