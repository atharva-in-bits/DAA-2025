#include <bits/stdc++.h>
using namespace std;

// -------------------- Structures --------------------

struct Node {
    int id;
    string name;
    bool isTreatmentStage;
};

struct BFEdge {
    int u, v;
    double w;
};

struct PumpOp {
    int id;
    int pumpId;
    int timeIndex;
    string action;
};

// =====================================================================
// BFS / DFS
// =====================================================================

void bfsFlow(int start, const vector<vector<int>>& adj) {
    vector<bool> vis(adj.size(), false);
    queue<int> q;
    q.push(start);
    vis[start] = true;

    while (!q.empty()) {
        int u = q.front(); q.pop();
        cout << u << " ";
        for (int v : adj[u]) {
            if (!vis[v]) {
                vis[v] = true;
                q.push(v);
            }
        }
    }
    cout << "\n";
}

void dfsUtil(int u, const vector<vector<int>>& adj, vector<bool>& vis) {
    vis[u] = true;
    cout << u << " ";
    for (int v : adj[u])
        if (!vis[v])
            dfsUtil(v, adj, vis);
}

void dfsFlow(int start, const vector<vector<int>>& adj) {
    vector<bool> vis(adj.size(), false);
    dfsUtil(start, adj, vis);
    cout << "\n";
}

// =====================================================================
// Dijkstra
// =====================================================================

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

// =====================================================================
// Bellman-Ford
// =====================================================================

vector<double> bellmanFord(int n, int src, const vector<BFEdge>& edges) {
    const double INF = 1e18;
    vector<double> dist(n, INF);
    dist[src] = 0.0;

    for (int i = 1; i <= n - 1; i++) {
        for (auto& e : edges) {
            if (dist[e.u] < INF && dist[e.u] + e.w < dist[e.v]) {
                dist[e.v] = dist[e.u] + e.w;
            }
        }
    }
    return dist;
}

// =====================================================================
// Union-Find
// =====================================================================

class UnionFind {
public:
    vector<int> parent, rk;

    UnionFind(int n = 0) { init(n); }

    void init(int n) {
        parent.resize(n);
        rk.assign(n, 0);
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]);
    }

    void unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return;
        if (rk[a] < rk[b]) swap(a, b);
        parent[b] = a;
        if (rk[a] == rk[b]) rk[a]++;
    }
};

// =====================================================================
// Segment Tree (Max)
// =====================================================================

class SegmentTree {
public:
    int n;
    vector<double> tree;

    SegmentTree(int n = 0) { init(n); }

    void init(int n) {
        this->n = n;
        tree.assign(4 * n, 0.0);
    }

    void build(const vector<double>& a, int v, int l, int r) {
        if (l == r) tree[v] = a[l];
        else {
            int m = (l + r) / 2;
            build(a, v*2, l, m);
            build(a, v*2+1, m+1, r);
            tree[v] = max(tree[v*2], tree[v*2+1]);
        }
    }

    void build(const vector<double>& a) {
        build(a, 1, 0, n-1);
    }

    double query(int v, int l, int r, int ql, int qr) {
        if (ql > qr) return -1e18;
        if (l == ql && r == qr) return tree[v];
        int m = (l + r) / 2;
        return max(
            query(v*2, l, m, ql, min(qr, m)),
            query(v*2+1, m+1, r, max(ql, m+1), qr)
        );
    }

    double query(int l, int r) {
        return query(1, 0, n-1, l, r);
    }
};

// =====================================================================
// Sparse Table (Min)
// =====================================================================

class SparseTable {
public:
    int n, K;
    vector<vector<double>> st;
    vector<int> lg;

    void build(const vector<double>& a) {
        n = a.size();
        K = 32 - __builtin_clz(n);
        st.assign(K, vector<double>(n));
        lg.assign(n+1, 0);

        for (int i = 2; i <= n; i++)
            lg[i] = lg[i/2] + 1;

        for (int i = 0; i < n; i++)
            st[0][i] = a[i];

        for (int k = 1; k < K; k++)
            for (int i = 0; i + (1<<k) <= n; i++)
                st[k][i] = min(st[k-1][i],
                               st[k-1][i + (1<<(k-1))]);
    }

    double rangeMin(int l, int r) const {
        int len = r - l + 1;
        int k = lg[len];
        return min(st[k][l], st[k][r - (1<<k) + 1]);
    }
};

// =====================================================================
// MAIN
// =====================================================================

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // ---------- Nodes ----------
    int N;
    cin >> N;
    vector<Node> nodes(N);
    for (int i = 0; i < N; i++)
        cin >> nodes[i].id >> nodes[i].name >> nodes[i].isTreatmentStage;

    // ---------- Graph ----------
    int E;
    cin >> E;
    vector<vector<int>> adj(N);
    for (int i = 0; i < E; i++) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    int start;
    cin >> start;
    bfsFlow(start, adj);
    dfsFlow(start, adj);

    // ---------- Weighted Graph ----------
    int WE;
    cin >> WE;
    vector<vector<pair<int,double>>> adjW(N);
    vector<BFEdge> bfEdges;
    for (int i = 0; i < WE; i++) {
        int u, v;
        double w;
        cin >> u >> v >> w;
        adjW[u].push_back({v, w});
        adjW[v].push_back({u, w});
        bfEdges.push_back({u, v, w});
        bfEdges.push_back({v, u, w});
    }

    int src;
    cin >> src;
    auto dDist = dijkstra(N, src, adjW);
    auto bDist = bellmanFord(N, src, bfEdges);

    for (double d : dDist) cout << d << " ";
    cout << "\n";
    for (double d : bDist) cout << d << " ";
    cout << "\n";

    // ---------- Union-Find ----------
    UnionFind uf(N);
    int ufOps;
    cin >> ufOps;
    for (int i = 0; i < ufOps; i++) {
        int a, b;
        cin >> a >> b;
        uf.unite(a, b);
    }

    unordered_map<int, vector<int>> zones;
    for (int i = 0; i < N; i++)
        zones[uf.find(i)].push_back(i);

    for (auto& z : zones) {
        for (int v : z.second) cout << v << " ";
        cout << "\n";
    }

    // ---------- Flow / Pressure ----------
    int T;
    cin >> T;
    vector<double> flow(T);
    for (int i = 0; i < T; i++) cin >> flow[i];

    SegmentTree seg(T);
    seg.build(flow);

    int l, r;
    cin >> l >> r;
    cout << seg.query(l, r) << "\n";

    SparseTable st;
    st.build(flow);
    cout << st.rangeMin(l, r) << "\n";

    // ---------- Pump Queue ----------
    int P;
    cin >> P;
    queue<PumpOp> q;
    for (int i = 0; i < P; i++) {
        PumpOp op;
        cin >> op.id >> op.pumpId >> op.timeIndex >> op.action;
        q.push(op);
    }

    while (!q.empty()) {
        auto op = q.front(); q.pop();
        cout << op.id << " "
             << op.pumpId << " "
             << op.timeIndex << " "
             << op.action << "\n";
    }

    return 0;
}

