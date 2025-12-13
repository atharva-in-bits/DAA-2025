#include <bits/stdc++.h>
using namespace std;

// -------------------- Basic structures --------------------

struct Machine {
    int id;
    string name;
    string location;
};

struct Alert {
    int machineId;
    string message;
    int severity;
    int timeIndex;
};

struct Task {
    int id;
    int machineId;
    string description;
};

// -------------------- Segment Tree (Max) --------------------

class SegmentTree {
public:
    int n;
    vector<double> tree;

    SegmentTree(int size = 0) { init(size); }

    void init(int size) {
        n = size;
        tree.assign(4 * n, 0.0);
    }

    void build(const vector<double>& a, int v, int tl, int tr) {
        if (tl == tr) tree[v] = a[tl];
        else {
            int tm = (tl + tr) / 2;
            build(a, v * 2, tl, tm);
            build(a, v * 2 + 1, tm + 1, tr);
            tree[v] = max(tree[v * 2], tree[v * 2 + 1]);
        }
    }

    void build(const vector<double>& a) {
        if (n > 0) build(a, 1, 0, n - 1);
    }

    void update(int v, int tl, int tr, int idx, double val) {
        if (tl == tr) tree[v] = val;
        else {
            int tm = (tl + tr) / 2;
            if (idx <= tm) update(v * 2, tl, tm, idx, val);
            else update(v * 2 + 1, tm + 1, tr, idx, val);
            tree[v] = max(tree[v * 2], tree[v * 2 + 1]);
        }
    }

    void update(int idx, double val) {
        update(1, 0, n - 1, idx, val);
    }

    double query(int v, int tl, int tr, int l, int r) {
        if (l > r) return -1e18;
        if (l == tl && r == tr) return tree[v];
        int tm = (tl + tr) / 2;
        return max(query(v * 2, tl, tm, l, min(r, tm)),
                   query(v * 2 + 1, tm + 1, tr, max(l, tm + 1), r));
    }

    double query(int l, int r) {
        return query(1, 0, n - 1, l, r);
    }
};

// -------------------- Fenwick Tree --------------------

class FenwickTree {
public:
    int n;
    vector<int> bit;

    FenwickTree(int size = 0) { init(size); }

    void init(int size) {
        n = size;
        bit.assign(n + 1, 0);
    }

    void update(int idx, int delta) {
        for (int i = idx + 1; i <= n; i += i & -i)
            bit[i] += delta;
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

// -------------------- Sparse Table (Min RMQ) --------------------

class SparseTable {
public:
    int n, K;
    vector<vector<double>> st;
    vector<int> logv;

    void build(const vector<double>& a) {
        n = a.size();
        K = 32 - __builtin_clz(n);
        st.assign(K, vector<double>(n));
        logv.assign(n + 1, 0);

        for (int i = 2; i <= n; i++)
            logv[i] = logv[i / 2] + 1;

        for (int i = 0; i < n; i++)
            st[0][i] = a[i];

        for (int k = 1; k < K; k++)
            for (int i = 0; i + (1 << k) <= n; i++)
                st[k][i] = min(st[k - 1][i],
                               st[k - 1][i + (1 << (k - 1))]);
    }

    double rangeMin(int l, int r) const {
        int len = r - l + 1;
        int k = logv[len];
        return min(st[k][l], st[k][r - (1 << k) + 1]);
    }
};

// -------------------- Alert Heap --------------------

struct AlertHeapItem {
    Alert alert;
    bool operator<(const AlertHeapItem& other) const {
        if (alert.severity != other.alert.severity)
            return alert.severity < other.alert.severity;
        return alert.timeIndex > other.alert.timeIndex;
    }
};

// -------------------- DFS --------------------

void dfsUtil(int u, vector<vector<int>>& g, vector<bool>& visited) {
    visited[u] = true;
    cout << u << " ";
    for (int v : g[u])
        if (!visited[v])
            dfsUtil(v, g, visited);
}

// -------------------- MAIN --------------------

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // ---------- Machines ----------
    int machineCount;
    cin >> machineCount;
    vector<Machine> machines(machineCount);
    for (int i = 0; i < machineCount; i++)
        cin >> machines[i].id >> machines[i].name >> machines[i].location;

    // ---------- Sensor Readings ----------
    int readingCount;
    cin >> readingCount;
    vector<double> readings(readingCount);
    for (int i = 0; i < readingCount; i++)
        cin >> readings[i];

    SegmentTree seg(readingCount);
    seg.build(readings);

    int ql, qr;
    cin >> ql >> qr;
    cout << seg.query(ql, qr) << "\n";

    // ---------- Fenwick ----------
    FenwickTree fw(readingCount);
    int fwUpdates;
    cin >> fwUpdates;
    for (int i = 0; i < fwUpdates; i++) {
        int idx, delta;
        cin >> idx >> delta;
        fw.update(idx, delta);
    }

    int fl, fr;
    cin >> fl >> fr;
    cout << fw.rangeSum(fl, fr) << "\n";

    // ---------- Sparse Table ----------
    SparseTable st;
    st.build(readings);
    int sl, sr;
    cin >> sl >> sr;
    cout << st.rangeMin(sl, sr) << "\n";

    // ---------- Alerts ----------
    int alertCount;
    cin >> alertCount;
    priority_queue<AlertHeapItem> alertPQ;
    for (int i = 0; i < alertCount; i++) {
        Alert a;
        cin >> a.machineId >> a.message >> a.severity >> a.timeIndex;
        alertPQ.push({a});
    }

    while (!alertPQ.empty()) {
        auto x = alertPQ.top(); alertPQ.pop();
        cout << x.alert.machineId << " "
             << x.alert.severity << " "
             << x.alert.message << "\n";
    }

    // ---------- Task Queue ----------
    int taskCount;
    cin >> taskCount;
    queue<Task> tasks;
    for (int i = 0; i < taskCount; i++) {
        Task t;
        cin >> t.id >> t.machineId >> t.description;
        tasks.push(t);
    }

    while (!tasks.empty()) {
        auto t = tasks.front(); tasks.pop();
        cout << t.id << " " << t.machineId << " "
             << t.description << "\n";
    }

    // ---------- Asset → Sensor Mapping ----------
    int mapCount;
    cin >> mapCount;
    unordered_map<int, vector<int>> assetSensors;
    for (int i = 0; i < mapCount; i++) {
        int assetId, sc;
        cin >> assetId >> sc;
        assetSensors[assetId].resize(sc);
        for (int j = 0; j < sc; j++)
            cin >> assetSensors[assetId][j];
    }

    int queryAsset;
    cin >> queryAsset;
    if (assetSensors.count(queryAsset)) {
        for (int s : assetSensors[queryAsset])
            cout << s << " ";
        cout << "\n";
    } else {
        cout << "NOT FOUND\n";
    }

    // ---------- DFS Process Graph ----------
    int nodes, edges;
    cin >> nodes >> edges;
    vector<vector<int>> graph(nodes);
    for (int i = 0; i < edges; i++) {
        int u, v;
        cin >> u >> v;
        graph[u].push_back(v);
    }

    int startNode;
    cin >> startNode;
    vector<bool> visited(nodes, false);
    dfsUtil(startNode, graph, visited);
    cout << "\n";

    return 0;
}

