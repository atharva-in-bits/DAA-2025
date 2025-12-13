#include <bits/stdc++.h>
using namespace std;

// -------------------- Structures --------------------

struct Intersection {
    int id;
    string name;
    double lat, lon;
};

struct CongestionInfo {
    int intersectionId;
    double delayScore;
};

struct VehicleArrival {
    string plate;
    int timeIndex;
};

// =====================================================================
// Segment Tree (Max vehicle count)
// =====================================================================

class SegmentTree {
public:
    int n;
    vector<int> tree;

    SegmentTree(int n = 0) { init(n); }

    void init(int n) {
        this->n = n;
        tree.assign(4 * n, 0);
    }

    void build(const vector<int>& a, int v, int l, int r) {
        if (l == r) tree[v] = a[l];
        else {
            int m = (l + r) / 2;
            build(a, v * 2, l, m);
            build(a, v * 2 + 1, m + 1, r);
            tree[v] = max(tree[v * 2], tree[v * 2 + 1]);
        }
    }

    void build(const vector<int>& a) {
        if (n > 0) build(a, 1, 0, n - 1);
    }

    int query(int v, int l, int r, int ql, int qr) {
        if (ql > qr) return 0;
        if (l == ql && r == qr) return tree[v];
        int m = (l + r) / 2;
        return max(
            query(v * 2, l, m, ql, min(qr, m)),
            query(v * 2 + 1, m + 1, r, max(ql, m + 1), qr)
        );
    }

    int query(int l, int r) {
        return query(1, 0, n - 1, l, r);
    }
};

// =====================================================================
// Fenwick Tree
// =====================================================================

class FenwickTree {
public:
    int n;
    vector<int> bit;

    FenwickTree(int n = 0) { init(n); }

    void init(int n) {
        this->n = n;
        bit.assign(n + 1, 0);
    }

    void update(int idx, int val) {
        for (int i = idx + 1; i <= n; i += i & -i)
            bit[i] += val;
    }

    int prefixSum(int idx) {
        int res = 0;
        for (int i = idx + 1; i > 0; i -= i & -i)
            res += bit[i];
        return res;
    }

    int rangeSum(int l, int r) {
        if (l > r) return 0;
        return prefixSum(r) - (l ? prefixSum(l - 1) : 0);
    }
};

// =====================================================================
// Sparse Table (Max RMQ)
// =====================================================================

class SparseTable {
public:
    int n, K;
    vector<vector<int>> st;
    vector<int> lg;

    void build(const vector<int>& a) {
        n = a.size();
        K = 32 - __builtin_clz(n);
        st.assign(K, vector<int>(n));
        lg.assign(n + 1, 0);

        for (int i = 2; i <= n; i++)
            lg[i] = lg[i / 2] + 1;

        for (int i = 0; i < n; i++)
            st[0][i] = a[i];

        for (int k = 1; k < K; k++)
            for (int i = 0; i + (1 << k) <= n; i++)
                st[k][i] = max(st[k - 1][i],
                               st[k - 1][i + (1 << (k - 1))]);
    }

    int rangeMax(int l, int r) const {
        int len = r - l + 1;
        int k = lg[len];
        return max(st[k][l], st[k][r - (1 << k) + 1]);
    }
};

// =====================================================================
// BFS / DFS Corridor Grouping
// =====================================================================

void bfsCorridor(int start, const vector<vector<int>>& adj) {
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

void dfsCorridor(int start, const vector<vector<int>>& adj) {
    vector<bool> vis(adj.size(), false);
    dfsUtil(start, adj, vis);
    cout << "\n";
}

// =====================================================================
// MAIN
// =====================================================================

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // ---------- Intersections ----------
    int I;
    cin >> I;
    vector<Intersection> intersections(I);
    unordered_map<int, Intersection> intMap;
    for (int i = 0; i < I; i++) {
        cin >> intersections[i].id
            >> intersections[i].name
            >> intersections[i].lat
            >> intersections[i].lon;
        intMap[intersections[i].id] = intersections[i];
    }

    // ---------- Traffic counts ----------
    int T;
    cin >> T;
    vector<int> counts(T);
    for (int i = 0; i < T; i++) cin >> counts[i];

    SegmentTree seg(T);
    seg.build(counts);

    FenwickTree fw(T);
    for (int i = 0; i < T; i++) fw.update(i, counts[i]);

    SparseTable st;
    st.build(counts);

    int l, r;
    cin >> l >> r;
    cout << seg.query(l, r) << "\n";
    cout << fw.rangeSum(l, r) << "\n";
    cout << st.rangeMax(l, r) << "\n";

    // ---------- Congestion Heap ----------
    int C;
    cin >> C;
    priority_queue<pair<double,int>> pq;
    for (int i = 0; i < C; i++) {
        int id;
        double score;
        cin >> id >> score;
        pq.push({score, id});
    }

    while (!pq.empty()) {
        auto x = pq.top(); pq.pop();
        cout << x.second << " " << x.first << "\n";
    }

    // ---------- Vehicle Queue ----------
    int V;
    cin >> V;
    queue<VehicleArrival> vq;
    for (int i = 0; i < V; i++) {
        VehicleArrival va;
        cin >> va.plate >> va.timeIndex;
        vq.push(va);
    }

    while (!vq.empty()) {
        auto v = vq.front(); vq.pop();
        cout << v.plate << " " << v.timeIndex << "\n";
    }

    // ---------- Corridor Graph ----------
    int E;
    cin >> E;
    vector<vector<int>> adj(I);
    for (int i = 0; i < E; i++) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    int start;
    cin >> start;
    bfsCorridor(start, adj);
    dfsCorridor(start, adj);

    // ---------- Intersection Lookup ----------
    int qid;
    cin >> qid;
    if (intMap.count(qid))
        cout << intMap[qid].name << "\n";
    else
        cout << "NOT FOUND\n";

    return 0;
}

