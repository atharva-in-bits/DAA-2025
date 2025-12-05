#include <bits/stdc++.h>
using namespace std;

struct NodeDist { int v; double d; NodeDist(int _v=0,double _d=0):v(_v),d(_d){} bool operator<(const NodeDist& o) const { return d>o.d; } };
struct Edge { int u,v; double w; Edge(int a=0,int b=0,double c=0):u(a),v(b),w(c){} };

class Graph {
public:
    int n;
    vector<vector<pair<int,double>>> adj;
    Graph(int nn=0){ init(nn); }
    void init(int nn){ n=nn; adj.assign(n,{}); }
    void addEdge(int u,int v,double w){ adj[u].push_back({v,w}); adj[v].push_back({u,w}); }
    vector<double> dijkstra(int src) const {
        vector<double> dist(n,1e18);
        priority_queue<NodeDist> pq;
        dist[src]=0; pq.push(NodeDist(src,0));
        while(!pq.empty()){
            auto cur=pq.top(); pq.pop();
            if(cur.d!=dist[cur.v]) continue;
            for(auto &pr: adj[cur.v]){
                int to=pr.first; double w=pr.second;
                if(dist[cur.v]+w < dist[to]){ dist[to]=dist[cur.v]+w; pq.push(NodeDist(to,dist[to])); }
            }
        }
        return dist;
    }
};

double tourCost(const vector<int>& tour, const vector<vector<double>>& dist){
    double s=0;
    for(size_t i=1;i<tour.size();++i) s+=dist[tour[i-1]][tour[i]];
    if(tour.size()>1) s+=dist[tour.back()][tour.front()];
    return s;
}

vector<int> nearestTour(const vector<int>& nodes, const vector<vector<double>>& dist){
    if(nodes.empty()) return {};
    unordered_set<int> left(nodes.begin(), nodes.end());
    vector<int> tour; tour.reserve(nodes.size());
    int cur = nodes[0]; tour.push_back(cur); left.erase(cur);
    while(!left.empty()){
        int best=-1; double bd=1e18;
        for(int v: left) if(dist[cur][v]<bd){ bd=dist[cur][v]; best=v; }
        tour.push_back(best); left.erase(best); cur=best;
    }
    return tour;
}

void twoOpt(vector<int>& tour, const vector<vector<double>>& dist){
    int n=tour.size();
    if(n<4) return;
    bool improved=true;
    while(improved){
        improved=false;
        for(int i=0;i<n-2 && !improved;i++){
            for(int j=i+2;j<n && !improved;j++){
                if(i==0 && j==n-1) continue;
                double a=dist[tour[i]][tour[i+1]];
                double b=dist[tour[j]][tour[(j+1)%n]];
                double c=dist[tour[i]][tour[j]];
                double d=dist[tour[i+1]][tour[(j+1)%n]];
                if(c + d + 1e-9 < a + b){
                    reverse(tour.begin()+i+1, tour.begin()+j+1);
                    improved=true;
                }
            }
        }
    }
}

vector<vector<int>> segmentToursByBattery(const vector<int>& tour, const vector<vector<double>>& dist, double batteryCapacity){
    vector<vector<int>> out;
    vector<int> cur; double used=0;
    for(size_t i=0;i<tour.size();++i){
        int node=tour[i];
        double cost = (cur.empty()? 0 : dist[cur.back()][node]);
        if(used + cost > batteryCapacity){
            if(!cur.empty()) out.push_back(cur);
            cur.clear(); used=0;
        }
        cur.push_back(node);
        if(i+1<tour.size()) used += dist[node][tour[i+1]];
    }
    if(!cur.empty()) out.push_back(cur);
    return out;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n,m;
    if(!(cin>>n>>m)) return 0;
    Graph g(n);
    for(int i=0;i<m;i++){ int u,v; double w; cin>>u>>v>>w; g.addEdge(u,v,w); }
    vector<vector<double>> distmat(n, vector<double>(n,1e18));
    for(int i=0;i<n;i++){ auto d=g.dijkstra(i); for(int j=0;j<n;j++) distmat[i][j]=d[j]; }
    int k; cin>>k;
    vector<int> targets(k);
    for(int i=0;i<k;i++) cin>>targets[i];
    auto tour = nearestTour(targets, distmat);
    twoOpt(tour, distmat);
    double battery; cin>>battery;
    auto segments = segmentToursByBattery(tour, distmat, battery);
    cout.setf(std::ios::fixed); cout<<setprecision(6);
    cout<<tourCost(tour, distmat)<<"\n";
    cout<<segments.size()<<"\n";
    for(auto &s: segments){ cout<<s.size(); for(int x:s) cout<<" "<<x; cout<<"\n"; }
    return 0;
}
