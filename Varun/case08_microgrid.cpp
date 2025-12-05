#include <bits/stdc++.h>
using namespace std;

struct Edge { int u,v; double w; Edge(int a=0,int b=0,double c=0):u(a),v(b),w(c){} };

struct NodeDist{ int v; double d; NodeDist(int _v=0,double _d=0):v(_v),d(_d){} bool operator<(const NodeDist& o) const { return d>o.d; } };

class Graph {
public:
    int n;
    vector<vector<pair<int,double>>> adj;
    Graph(int nn=0){ init(nn); }
    void init(int nn){ n=nn; adj.assign(n,{}); }
    void addEdge(int u,int v,double w){ adj[u].push_back({v,w}); adj[v].push_back({u,w}); }
    vector<double> dijkstra(int src) const {
        vector<double> dist(n,1e18); priority_queue<NodeDist> pq; dist[src]=0; pq.push(NodeDist(src,0));
        while(!pq.empty()){
            auto cur=pq.top(); pq.pop();
            if(cur.d!=dist[cur.v]) continue;
            for(auto &pr: adj[cur.v]){ int to=pr.first; double w=pr.second; if(dist[cur.v]+w<dist[to]){ dist[to]=dist[cur.v]+w; pq.push(NodeDist(to,dist[to])); } }
        }
        return dist;
    }
};

struct Fenwick {
    int n; vector<long long> bit; Fenwick(int nn=0){ init(nn); } void init(int nn){ n=nn; bit.assign(n+1,0); }
    void add(int i,long long v){ for(++i;i<=n;i+=i&-i) bit[i]+=v; }
    long long sum(int i){ long long s=0; for(++i;i>0;i-=i&-i) s+=bit[i]; return s; }
    long long rangeSum(int l,int r){ if(r<l) return 0; return sum(r)-(l?sum(l-1):0); }
};

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int nodes,edges; if(!(cin>>nodes>>edges)) return 0;
    Graph g(nodes);
    for(int i=0;i<edges;i++){ int u,v; double w; cin>>u>>v>>w; g.addEdge(u,v,w); }
    int T; cin>>T;
    vector<long long> demand(T), renewable(T);
    for(int t=0;t<T;t++) cin>>demand[t];
    for(int t=0;t<T;t++) cin>>renewable[t];
    int B; cin>>B;
    vector<long long> cap(B), soc(B);
    for(int i=0;i<B;i++) cin>>cap[i]>>soc[i];
    Fenwick fen(T);
    for(int t=0;t<T;t++) fen.add(t, demand[t]);
    vector<int> chargePlan(T, -1);
    for(int t=0;t<T;t++){
        long long net = demand[t] - renewable[t];
        if(net<=0){ chargePlan[t]=0; continue; }
        for(int b=0;b<B;b++){
            if(soc[b]>=net){ soc[b]-=net; chargePlan[t]=b; net=0; break; }
        }
        if(net>0) chargePlan[t]=-1;
    }
    int src; cin>>src;
    auto dist=g.dijkstra(src);
    cout.setf(std::ios::fixed); cout<<setprecision(6);
    long long totalUnserved=0;
    for(int t=0;t<T;t++) if(chargePlan[t]==-1) totalUnserved += demand[t]-renewable[t];
    cout<<totalUnserved<<"\n";
    for(int t=0;t<T;t++) cout<<chargePlan[t]<<" ";
    cout<<"\n";
    double avgDist=0; int cnt=0;
    for(int i=0;i<nodes;i++) if(dist[i]<1e17){ avgDist+=dist[i]; cnt++; }
    if(cnt) avgDist/=cnt; cout<<avgDist<<"\n";
    return 0;
}

