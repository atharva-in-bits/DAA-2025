#include <bits/stdc++.h>
using namespace std;

// -------------------- Basic structures --------------------

struct Pole {
    int id;
    double lat, lon;
    int circuitId;
    bool isFaulty;
};

struct FaultReport {
    int poleId;
    int timeIndex;
    string description;
};

// ============================================================================
// BFS / DFS: fault cluster detection
// ============================================================================

void bfsFaultCluster(int start, const vector<vector<int>>& adj,
                     const vector<bool>& faulty) {
    vector<bool> vis(adj.size(), false);
    queue<int> q;
    q.push(start);
    vis[start] = true;

    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (faulty[u]) cout << u << " ";
        for (int v : adj[u]) {
            if (!vis[v]) {
                vis[v] = true;
                q.push(v);
            }
        }
    }
    cout << "\n";
}

void dfsUtil(int u, const vector<vector<int>>& adj,
             const vector<bool>& faulty, vector<bool>& vis) {
    vis[u] = true;
    if (faulty[u]) cout << u << " ";
    for (int v : adj[u])
        if (!vis[v])
            dfsUtil(v, adj, faulty, vis);
}

void dfsFaultCluster(int start, const vector<vector<int>>& adj,
                     const vector<bool>& faulty) {
    vector<bool> vis(adj.size(), false);
    dfsUtil(start, adj, faulty, vis);
    cout << "\n";
}

// ============================================================================
// Dijkstra: crew routing
// ============================================================================

vector<double> dijkstra(int n, int src,
                        const vector<vector<pair<int,double>>>& adj) {
    const double INF = 1e18;
    vector<double> dist(n, INF);
    dist[src] = 0.0;

    priority_queue<pair<double,int>,
        vector<pair<double,int>>, greater<pair<double,int>>> pq;
    pq.push({0.0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (auto [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

// ============================================================================
// Union-Find: circuit connectivity
// ============================================================================

class UnionFind {
public:
    vector<int> parent, rnk;
    UnionFind(int n = 0) { init(n); }

    void init(int n) {
        parent.resize(n);
        rnk.assign(n, 0);
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]);
    }

    void unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return;
        if (rnk[a] < rnk[b]) swap(a, b);
        parent[b] = a;
        if (rnk[a] == rnk[b]) rnk[a]++;
    }
};

// ============================================================================
// Sparse Table: brightness RMQ (min)
// ============================================================================

class SparseTable {
public:
    int n, K;
    vector<vector<double>> st;
    vector<int> lg;

    void build(const vector<double>& a) {
        n = a.size();
        K = 32 - __builtin_clz(n);
        st.assign(K, vector<double>(n));
        lg.assign(n + 1, 0);

        for (int i = 2; i <= n; i++)
            lg[i] = lg[i / 2] + 1;

        for (int i = 0; i < n; i++)
            st[0][i] = a[i];

        for (int k = 1; k < K; k++)
            for (int i = 0; i + (1 << k) <= n; i++)
                st[k][i] = min(st[k - 1][i],
                               st[k - 1][i + (1 << (k - 1))]);
    }

    double rangeMin(int l, int r) const {
        int len = r - l + 1;
        int k = lg[len];
        return min(st[k][l], st[k][r - (1 << k) + 1]);
    }
};

// ============================================================================
// MAIN
// ============================================================================

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // ---------- Poles ----------
    int poleCount;
    cin >> poleCount;
    vector<Pole> poles(poleCount);
    for (int i = 0; i < poleCount; i++) {
        cin >> poles[i].id
            >> poles[i].lat
            >> poles[i].lon
            >> poles[i].circuitId
            >> poles[i].isFaulty;
    }

    // ---------- Pole adjacency ----------
    int poleEdges;
    cin >> poleEdges;
    vector<vector<int>> poleAdj(poleCount);
    for (int i = 0; i < poleEdges; i++) {
        int u, v;
        cin >> u >> v;
        poleAdj[u].push_back(v);
        poleAdj[v].push_back(u);
    }

    vector<bool> faulty(poleCount, false);
    for (auto& p : poles)
        faulty[p.id] = p.isFaulty;

    int startFault;
    cin >> startFault;
    bfsFaultCluster(startFault, poleAdj, faulty);
    dfsFaultCluster(startFault, poleAdj, faulty);

    // ---------- Road graph ----------
    int roadEdges;
    cin >> roadEdges;
    vector<vector<pair<int,double>>> roadAdj(poleCount);
    for (int i = 0; i < roadEdges; i++) {
        int u, v;
        double w;
        cin >> u >> v >> w;
        roadAdj[u].push_back({v, w});
        roadAdj[v].push_back({u, w});
    }

    int depot;
    cin >> depot;
    auto dist = dijkstra(poleCount, depot, roadAdj);
    for (double d : dist) cout << d << " ";
    cout << "\n";

    // ---------- Union-Find circuits ----------
    UnionFind uf(poleCount);
    int ufOps;
    cin >> ufOps;
    for (int i = 0; i < ufOps; i++) {
        int a, b;
        cin >> a >> b;
        uf.unite(a, b);
    }

    unordered_map<int, vector<int>> circuits;
    for (int i = 0; i < poleCount; i++)
        circuits[uf.find(i)].push_back(i);

    for (auto& g : circuits) {
        for (int id : g.second) cout << id << " ";
        cout << "\n";
    }

    // ---------- Brightness logs ----------
    int brightnessCount;
    cin >> brightnessCount;
    vector<double> brightness(brightnessCount);
    for (int i = 0; i < brightnessCount; i++)
        cin >> brightness[i];

    SparseTable st;
    st.build(brightness);
    int bl, br;
    cin >> bl >> br;
    cout << st.rangeMin(bl, br) << "\n";

    // ---------- Fault queue ----------
    int faultCount;
    cin >> faultCount;
    queue<FaultReport> fq;
    for (int i = 0; i < faultCount; i++) {
        FaultReport fr;
        cin >> fr.poleId >> fr.timeIndex >> fr.description;
        fq.push(fr);
    }

    while (!fq.empty()) {
        auto f = fq.front(); fq.pop();
        cout << f.poleId << " "
             << f.timeIndex << " "
             << f.description << "\n";
    }

    return 0;
}

