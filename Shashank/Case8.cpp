

// Smart Streetlight Monitoring Demo
// Purpose: Detect faults and optimize maintenance crew routes.
//
// Algorithms / DS:
// - BFS/DFS: fault cluster detection on pole network
// - Dijkstra: compute repair crew routes on road graph
// - Union-Find: maintain circuit connectivity components
// - Sparse Table: RMQ on historical brightness logs
// - Queue: fault report buffering
// - Arrays/Structures: pole and circuit metadata

#include <bits/stdc++.h>
using namespace std;

// -------------------- Basic structures --------------------

struct Pole {
    int id;
    double lat, lon;      // location (simple placeholder)
    int circuitId;        // electrical circuit ID
    bool isFaulty;
};

struct FaultReport {
    int poleId;
    int timeIndex;
    string description;
};

// -------------------- Graph representation for poles/roads --------------------
// Adjacency list for pole connectivity (for BFS/DFS clustering).
// Separate adjacency list for road graph (for Dijkstra routes).

// ============================================================================
// BFS / DFS: fault cluster detection on the pole network
// ============================================================================

void bfsFaultCluster(int start, const vector<vector<int>>& adj, const vector<bool>& faulty) {
    vector<bool> vis(adj.size(), false);
    queue<int> q;
    q.push(start);
    vis[start] = true;
    cout << "BFS cluster starting from faulty pole " << start << ":\n  ";
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
    cout << "\n\n";
}

void dfsUtil(int u, const vector<vector<int>>& adj, const vector<bool>& faulty, vector<bool>& vis) {
    vis[u] = true;
    if (faulty[u]) cout << u << " ";
    for (int v : adj[u]) {
        if (!vis[v]) dfsUtil(v, adj, faulty, vis);
    }
}

void dfsFaultCluster(int start, const vector<vector<int>>& adj, const vector<bool>& faulty) {
    vector<bool> vis(adj.size(), false);
    cout << "DFS cluster starting from faulty pole " << start << ":\n  ";
    dfsUtil(start, adj, faulty, vis);
    cout << "\n\n";
}

/*
BFS/DFS comments (streetlight context):
- Treat poles as nodes and cabling or proximity as edges.
- From a faulty pole, BFS/DFS finds all connected faulty neighbors, identifying clusters (e.g., an entire block out).
- Helps distinguish isolated failures from larger area outages.
*/

// ============================================================================
// Dijkstra: shortest route for repair teams on a road graph
// ============================================================================

struct Edge {
    int u, v;
    double w; // travel time or distance
};

vector<double> dijkstra(int n, int src, const vector<vector<pair<int,double>>>& adj) {
    const double INF = 1e18;
    vector<double> dist(n, INF);
    dist[src] = 0.0;
    using P = pair<double,int>;
    priority_queue<P, vector<P>, greater<P>> pq;
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

/*
Dijkstra comments (streetlight context):
- Uses a weighted road graph to compute the fastest path from a depot to all poles.
- Maintenance planners or apps can pick the next pole to visit based on minimal travel time.
- Works well when edge weights are distances or estimated travel times on city streets.
*/

// ============================================================================
// Union-Find (Disjoint Set): circuit connectivity
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

void demoCircuitConnectivity() {
    int n = 6; // 6 poles on a circuit
    UnionFind uf(n);

    // Assume connections forming two circuits: {0,1,2} and {3,4,5}.
    uf.unite(0, 1);
    uf.unite(1, 2);
    uf.unite(3, 4);
    uf.unite(4, 5);

    unordered_map<int, vector<int>> groups;
    for (int i = 0; i < n; ++i) {
        groups[uf.find(i)].push_back(i);
    }

    cout << "Circuit connectivity groups (Union-Find):\n";
    for (auto& g : groups) {
        cout << "  Circuit root " << g.first << " -> poles: ";
        for (int id : g.second) cout << id << " ";
        cout << "\n";
    }
    cout << "\n";
}

/*
Union-Find comments (streetlight context):
- Tracks which poles belong to the same electrical circuit or feeder.
- When a fault occurs, operators can see all poles in the affected circuit quickly.
- Helps in planning isolation and understanding cascading failures along a line.
*/

// ============================================================================
// Sparse Table: brightness logs RMQ (min over interval)
// ============================================================================

class SparseTable {
public:
    int n, K;
    vector<vector<double>> st;
    vector<int> lg;

    void build(const vector<double>& a) {
        n = (int)a.size();
        K = 32 - __builtin_clz(n);
        st.assign(K, vector<double>(n));
        lg.assign(n + 1, 0);
        for (int i = 2; i <= n; ++i)
            lg[i] = lg[i / 2] + 1;

        for (int i = 0; i < n; ++i)
            st[0][i] = a[i];

        for (int k = 1; k < K; ++k) {
            for (int i = 0; i + (1 << k) <= n; ++i) {
                st[k][i] = min(st[k - 1][i], st[k - 1][i + (1 << (k - 1))]);
            }
        }
    }

    double rangeMin(int l, int r) const {
        int len = r - l + 1;
        int k = lg[len];
        return min(st[k][l], st[k][r - (1 << k) + 1]);
    }
};

void demoBrightnessLogs() {
    // Example brightness readings (0..100%) for one pole over time.
    vector<double> brightness = {90, 88, 40, 20, 15, 80, 85, 87};

    SparseTable st;
    st.build(brightness);

    cout << "SparseTable: min brightness[2..5] = "
         << st.rangeMin(2, 5) << "%\n\n";
}

/*
Sparse Table comments (streetlight context):
- Built on historical brightness logs for a pole or segment.
- Answers queries like â€œminimum brightness between t1 and t2â€ in O(1) after preprocessing.
- Helps detect long-running dim or off states that suggest lamp or driver degradation.
*/

// ============================================================================
// Queue: fault reports
// ============================================================================

void demoFaultQueue() {
    queue<FaultReport> faults;
    faults.push({1, 100, "Lamp off at night"});
    faults.push({3, 101, "Blinking behavior"});
    faults.push({2, 105, "Low brightness"});

    cout << "Fault report queue (FIFO handling):\n";
    while (!faults.empty()) {
        auto fr = faults.front(); faults.pop();
        cout << "  Pole " << fr.poleId << " at t=" << fr.timeIndex
             << " : " << fr.description << "\n";
    }
    cout << "\n";
}

/*
Queue comments (streetlight context):
- Buffers incoming fault events from sensors, SCADA, or user apps.
- Allows a central system to process and acknowledge reports in arrival order.
- Can later be combined with priority structures if some faults must be escalated.
*/

// ============================================================================
// Arrays/Structures: pole metadata and graph wiring
// ============================================================================

void demoPolesAndRouting() {
    // Example poles
    vector<Pole> poles = {
        {0, 12.90, 77.60, 0, false},
        {1, 12.91, 77.61, 0, true},
        {2, 12.92, 77.62, 0, true},
        {3, 12.93, 77.63, 1, false},
        {4, 12.94, 77.64, 1, true}
    };

    cout << "Pole metadata:\n";
    for (auto& p : poles) {
        cout << "  Pole " << p.id << " circ=" << p.circuitId
             << " faulty=" << (p.isFaulty ? "Y" : "N") << "\n";
    }
    cout << "\n";

    // Build pole adjacency (simple line graph here) for BFS/DFS
    int n = poles.size();
    vector<vector<int>> poleAdj(n);
    poleAdj[0] = {1};
    poleAdj[1] = {0, 2};
    poleAdj[2] = {1, 3};
    poleAdj[3] = {2, 4};
    poleAdj[4] = {3};

    vector<bool> faulty(n, false);
    for (auto& p : poles) faulty[p.id] = p.isFaulty;

    // Run BFS/DFS from a known faulty pole
    bfsFaultCluster(1, poleAdj, faulty);
    dfsFaultCluster(1, poleAdj, faulty);

    // Build road graph (same structure here for simplicity) for Dijkstra
    vector<vector<pair<int,double>>> roadAdj(n);
    auto addEdge = [&](int u, int v, double w) {
        roadAdj[u].push_back({v, w});
        roadAdj[v].push_back({u, w});
    };
    addEdge(0, 1, 1.0);
    addEdge(1, 2, 1.0);
    addEdge(2, 3, 1.5);
    addEdge(3, 4, 1.0);

    int depot = 0;
    auto dist = dijkstra(n, depot, roadAdj);
    cout << "Dijkstra: travel cost from depot " << depot << " to each pole:\n";
    for (int i = 0; i < n; ++i) {
        cout << "  to pole " << i << " : " << dist[i] << "\n";
    }
    cout << "\n";
}

/*
Arrays/Structures comments (streetlight context):
- Pole structs hold ID, location, circuit, and fault status.
- Arrays/vectors of poles and adjacency lists form the base for BFS/DFS and Dijkstra.
- This metadata underpins monitoring dashboards and routing tools for crews.
*/

// ============================================================================
// MAIN
// ============================================================================

int main() {
    cout << "=== Smart Streetlight Monitoring Demo ===\n\n";

    demoPolesAndRouting();
    demoCircuitConnectivity();
    demoBrightnessLogs();
    demoFaultQueue();

    return 0;
}
