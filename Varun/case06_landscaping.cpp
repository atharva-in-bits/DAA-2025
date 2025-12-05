#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using db = double;
const db INF = 1e18;

struct Edge { int u,v; db w; db benefit; Edge(){} Edge(int a,int b,db c,db be=0):u(a),v(b),w(c),benefit(be){} };
struct Graph {
    int n;
    vector<vector<pair<int,db>>> adj;
    vector<Edge> edges;
    Graph(int n_=0){ init(n_); }
    void init(int N){ n=N; adj.assign(n,{}); edges.clear(); }
    void addEdge(int u,int v,db w, db benefit=0){
        if(u<0||v<0||u>=n||v>=n) return;
        adj[u].push_back({v,w});
        adj[v].push_back({u,w});
        edges.emplace_back(u,v,w,benefit);
    }
    int size() const { return n; }
};

struct DSU {
    int n;
    vector<int> p;
    DSU(int n_=0){ init(n_); }
    void init(int n_){ n=n_; p.resize(n); iota(p.begin(), p.end(), 0); }
    int find(int x){ return p[x]==x?x:p[x]=find(p[x]); }
    bool unite(int a,int b){ a=find(a); b=find(b); if(a==b) return false; p[b]=a; return true; }
};

db kruskal_mst(const Graph &g, vector<Edge>* out_edges = nullptr){
    auto es = g.edges;
    sort(es.begin(), es.end(), [](const Edge&a,const Edge&b){ return a.w < b.w; });
    DSU d(g.n);
    db total=0; int used=0;
    for(auto &e: es){
        if(d.unite(e.u, e.v)){
            total += e.w;
            if(out_edges) out_edges->push_back(e);
            if(++used==g.n-1) break;
        }
    }
    if(used==g.n-1) return total;
    return INF;
}

vector<int> bfs_component(const Graph &g, int s){
    vector<char> vis(g.n,0);
    deque<int> dq; dq.push_back(s); vis[s]=1;
    vector<int> comp;
    while(!dq.empty()){
        int u=dq.front(); dq.pop_front();
        comp.push_back(u);
        for(auto &pr: g.adj[u]) if(!vis[pr.first]){ vis[pr.first]=1; dq.push_back(pr.first); }
    }
    return comp;
}

vector<vector<int>> all_components(const Graph &g){
    vector<char> vis(g.n,0);
    vector<vector<int>> comps;
    for(int i=0;i<g.n;i++){
        if(!vis[i]){
            deque<int> dq; dq.push_back(i); vis[i]=1;
            vector<int> comp;
            while(!dq.empty()){
                int u=dq.front(); dq.pop_front();
                comp.push_back(u);
                for(auto &pr: g.adj[u]) if(!vis[pr.first]){ vis[pr.first]=1; dq.push_back(pr.first); }
            }
            comps.push_back(comp);
        }
    }
    return comps;
}

vector<int> dfs_order(const Graph &g, int s){
    vector<char> vis(g.n,0);
    vector<int> order;
    function<void(int)> dfs = [&](int u){
        vis[u]=1;
        order.push_back(u);
        for(auto &pr: g.adj[u]) if(!vis[pr.first]) dfs(pr.first);
    };
    dfs(s);
    return order;
}

vector<vector<int>> floyd_warshall_compact(const Graph &g){
    int n=g.n;
    vector<vector<db>> dist(n, vector<db>(n, INF));
    for(int i=0;i<n;i++) dist[i][i]=0;
    for(auto &e: g.edges){
        if(e.w < dist[e.u][e.v]){ dist[e.u][e.v]=e.w; dist[e.v][e.u]=e.w; }
    }
    for(int k=0;k<n;k++){
        for(int i=0;i<n;i++){
            if(dist[i][k]==INF) continue;
            for(int j=0;j<n;j++){
                if(dist[k][j]==INF) continue;
                db nd = dist[i][k] + dist[k][j];
                if(nd < dist[i][j]) dist[i][j]=nd;
            }
        }
    }
    vector<vector<int>> mat(n, vector<int>(n, -1));
    for(int i=0;i<n;i++) for(int j=0;j<n;j++) if(dist[i][j]<INF) mat[i][j]=1;
    return mat;
}

struct CandidateEdge {
    Edge e;
    db score;
    CandidateEdge(){}
    CandidateEdge(const Edge&ee, db sc):e(ee),score(sc){}
};

vector<CandidateEdge> evaluate_edges(const Graph &g, const vector<int>&priority_nodes){
    vector<CandidateEdge> res;
    int N = g.n;
    vector<db> centrality(N,0.0);
    for(int p: priority_nodes) centrality[p]+=1.0;
    for(const auto &e: g.edges){
        db score = e.benefit / max<db>(1.0, e.w);
        score += (centrality[e.u] + centrality[e.v]) * 0.5;
        res.emplace_back(e, score);
    }
    return res;
}

vector<Edge> greedy_select_by_benefit_cost(const Graph &g, db budget){
    auto cand = evaluate_edges(g, vector<int>());
    sort(cand.begin(), cand.end(), [](const CandidateEdge&a,const CandidateEdge&b){ return a.score > b.score; });
    vector<Edge> chosen;
    db spent=0;
    DSU d(g.n);
    for(auto &c: cand){
        if(spent + c.e.w > budget) continue;
        if(d.find(c.e.u) == d.find(c.e.v)) continue;
        d.unite(c.e.u, c.e.v);
        chosen.push_back(c.e);
        spent += c.e.w;
    }
    return chosen;
}

vector<Edge> knapsack_like_select(const vector<Edge>&edges, db budget){
    int m=edges.size();
    vector<pair<db,int>> arr;
    for(int i=0;i<m;i++){
        db val = edges[i].benefit;
        db wt = edges[i].w;
        arr.push_back({val / max<db>(1e-9, wt), i});
    }
    sort(arr.begin(), arr.end(), [](const pair<db,int>&a,const pair<db,int>&b){ return a.first > b.first; });
    vector<Edge> chosen;
    db used=0;
    for(auto &pr: arr){
        int i=pr.second;
        if(used + edges[i].w <= budget){
            chosen.push_back(edges[i]);
            used += edges[i].w;
        }
    }
    return chosen;
}

Graph build_random_habitat(int n,int m,int seed){
    Graph g(n);
    mt19937_64 rng(seed);
    uniform_real_distribution<db> coord(0,100);
    uniform_real_distribution<db> cost(1,20);
    uniform_real_distribution<db> benefit(1,50);
    set<pair<int,int>> used;
    vector<pair<db,db>> pts(n);
    for(int i=0;i<n;i++) pts[i] = {coord(rng), coord(rng)};
    while((int)g.edges.size() < m){
        int a = rng() % n, b = rng() % n;
        if(a==b) continue;
        if(a>b) swap(a,b);
        if(used.insert({a,b}).second){
            db d = hypot(pts[a].first - pts[b].first, pts[a].second - pts[b].second);
            db w = max<db>(1.0, d * (cost(rng)/10.0));
            db be = benefit(rng);
            g.addEdge(a,b,w,be);
        }
    }
    return g;
}

vector<int> highest_degree_nodes(const Graph &g, int k){
    int n=g.n;
    vector<pair<int,int>> deg;
    for(int i=0;i<n;i++) deg.push_back({(int)g.adj[i].size(), i});
    sort(deg.begin(), deg.end(), [](const pair<int,int>&a,const pair<int,int>&b){ return a.first > b.first; });
    vector<int> out;
    for(int i=0;i<k && i<deg.size(); ++i) out.push_back(deg[i].second);
    return out;
}

vector<Edge> budgeted_improvement_plan(const Graph &g, db budget){
    vector<Edge> res;
    auto degs = highest_degree_nodes(g, max(1, g.n/10));
    auto cand = evaluate_edges(g, degs);
    sort(cand.begin(), cand.end(), [](const CandidateEdge&a,const CandidateEdge&b){ return a.score > b.score; });
    db used=0;
    DSU d(g.n);
    for(auto &c: cand){
        if(used + c.e.w > budget) continue;
        if(d.find(c.e.u) == d.find(c.e.v)) continue;
        d.unite(c.e.u, c.e.v);
        res.push_back(c.e);
        used += c.e.w;
    }
    return res;
}

vector<int> connectivity_after_edges(int n, const vector<Edge>&existing, const vector<Edge>&added){
    DSU d(n);
    for(auto &e: existing) d.unite(e.u, e.v);
    for(auto &e: added) d.unite(e.u, e.v);
    vector<int> comp(n);
    for(int i=0;i<n;i++) comp[i] = d.find(i);
    return comp;
}

db ecosystem_score(const Graph &g, const vector<Edge>&chosen){
    db score=0;
    vector<char> touched(g.n, 0);
    for(auto &e: chosen){
        score += e.benefit;
        touched[e.u]=1; touched[e.v]=1;
    }
    int tcount=0;
    for(int i=0;i<g.n;i++) if(touched[i]) tcount++;
    score += tcount * 0.1;
    return score;
}

vector<Edge> augment_mst_with_budget(const Graph &g, db budget){
    vector<Edge> mst_edges;
    db base = kruskal_mst(g, &mst_edges);
    vector<Edge> extras;
    db used=0;
    set<pair<int,int>> mstset;
    for(auto &e: mst_edges){ int a=min(e.u,e.v), b=max(e.u,e.v); mstset.insert({a,b}); }
    auto sorted_edges = g.edges;
    sort(sorted_edges.begin(), sorted_edges.end(), [](const Edge&a,const Edge&b){ return a.benefit > b.benefit; });
    for(auto &e: sorted_edges){
        int a=min(e.u,e.v), b=max(e.u,e.v);
        if(mstset.count({a,b})) continue;
        if(used + e.w <= budget){ extras.push_back(e); used += e.w; mstset.insert({a,b}); }
    }
    for(auto &e: extras) mst_edges.push_back(e);
    return mst_edges;
}

pair<vector<Edge>, db> plan_restoration(const Graph &g, db budget){
    auto plan1 = greedy_select_by_benefit_cost(g, budget);
    auto plan2 = knapsack_like_select(g.edges, budget);
    db s1 = ecosystem_score(g, plan1);
    db s2 = ecosystem_score(g, plan2);
    if(s1 >= s2) return {plan1, s1};
    return {plan2, s2};
}

vector<int> highest_betweenness_approx(const Graph &g, int samples){
    int n=g.n;
    vector<double> score(n,0.0);
    mt19937 rng(123);
    uniform_int_distribution<int> ui(0,n-1);
    for(int s=0;s<samples;s++){
        int a = ui(rng), b = ui(rng);
        vector<db> dist(n, INF);
        vector<int> prev(n, -1);
        using P=pair<db,int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        dist[a]=0; pq.push({0,a});
        while(!pq.empty()){
            auto cur=pq.top(); pq.pop();
            db d=cur.first; int u=cur.second;
            if(d!=dist[u]) continue;
            if(u==b) break;
            for(auto &pr: g.adj[u]){
                int v=pr.first; db w=pr.second;
                if(d + w < dist[v]){ dist[v]=d+w; prev[v]=u; pq.push({dist[v], v}); }
            }
        }
        int cur=b;
        while(prev[cur]!=-1){
            score[cur]+=1.0;
            cur = prev[cur];
        }
    }
    vector<pair<double,int>> arr;
    for(int i=0;i<n;i++) arr.push_back({score[i], i});
    sort(arr.begin(), arr.end(), [](const pair<double,int>&a,const pair<double,int>&b){ return a.first > b.first; });
    vector<int> out;
    for(int i=0;i<min((int)arr.size(), max(1, n/10)); ++i) out.push_back(arr[i].second);
    return out;
}

pair<vector<Edge>, db> iterative_budgeted_growth(const Graph &g, db total_budget, int steps){
    vector<Edge> final;
    db spent=0;
    Graph cur = g;
    for(int t=0;t<steps && spent < total_budget; ++t){
        db step_budget = min<db>(total_budget - spent, total_budget / steps);
        auto add = budgeted_improvement_plan(cur, step_budget);
        for(auto &e: add){ final.push_back(e); spent += e.w; }
    }
    db sc = ecosystem_score(g, final);
    return {final, sc};
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int mode;
    if(!(cin>>mode)) return 0;
    if(mode==0){
        Graph g = build_random_habitat(80, 240, 2025);
        db budget = 200.0;
        auto best = plan_restoration(g, budget);
        cout.setf(std::ios::fixed);
        cout<<setprecision(6);
        cout<<"CHOSEN "<<best.first.size()<<" SCORE "<<best.second<<"\n";
        for(auto &e: best.first) cout<<e.u<<" "<<e.v<<" "<<e.w<<" "<<e.benefit<<"\n";
        return 0;
    }
    if(mode==1){
        int n,m; cin>>n>>m;
        Graph g(n);
        for(int i=0;i<m;i++){
            int u,v; db w,be;
            cin>>u>>v>>w>>be;
            g.addEdge(u,v,w,be);
        }
        db budget; cin>>budget;
        auto res = augment_mst_with_budget(g, budget);
        cout<<res.size()<<"\n";
        for(auto &e: res) cout<<e.u<<" "<<e.v<<" "<<e.w<<" "<<e.benefit<<"\n";
        return 0;
    }
    if(mode==2){
        int n,m; cin>>n>>m;
        Graph g(n);
        for(int i=0;i<m;i++){
            int u,v; db w,be;
            cin>>u>>v>>w>>be;
            g.addEdge(u,v,w,be);
        }
        db budget; cin>>budget;
        auto it = iterative_budgeted_growth(g, budget, 5);
        cout<<it.first.size()<<" "<<it.second<<"\n";
        for(auto &e: it.first) cout<<e.u<<" "<<e.v<<" "<<e.w<<" "<<e.benefit<<"\n";
        return 0;
    }
    return 0;
}
