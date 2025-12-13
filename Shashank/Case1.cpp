// Ride-Sharing Dispatch Demo
// Algorithms: BFS, DFS, Dijkstra, Bellman-Ford
// Data structures: queue (request buffer), priority_queue (min-heap for closest driver),
// unordered_map (hashing for fast driver lookup)

#include <bits/stdc++.h>
using namespace std;

// -------------------- Shared Graph Structure --------------------
// Nodes can represent map intersections or zones.
// Edges represent roads between them.
struct Edge {
    int u, v;
    int w; // travel time or distance
};

class Graph {
public:
    int V;
    vector<vector<int>> adjList;                // for BFS/DFS
    vector<vector<pair<int,int>>> adjWeighted;  // for Dijkstra
    vector<Edge> edges;                         // for Bellman-Ford

    Graph(int n = 0) {
        V = n;
        adjList.assign(V, {});
        adjWeighted.assign(V, {});
    }

    void resize(int n) {
        V = n;
        adjList.assign(V, {});
        adjWeighted.assign(V, {});
        edges.clear();
    }

    void addEdge(int u, int v) {
        adjList[u].push_back(v);
        adjList[v].push_back(u);  // undirected for area reachability
    }

    void addWeightedEdge(int u, int v, int w) {
        adjWeighted[u].push_back({v, w});
        edges.push_back({u, v, w}); // directed edge for BF
    }
};

// =================================================================
// BFS: Find nearby drivers/zones level by level from a rider’s zone
// =================================================================
void BFS(const Graph& g, int start) {
    vector<bool> visited(g.V, false);
    queue<int> q; // queue used for level-order exploration

    visited[start] = true;
    q.push(start);

    cout << "BFS (zones reachable from rider zone " << start << "): ";
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        cout << u << " ";

        // explore all neighboring zones/roads
        for (int v : g.adjList[u]) {
            if (!visited[v]) {
                visited[v] = true;
                q.push(v);
            }
        }
    }
    cout << "\n";
}

// =================================================================
// DFS: Explore connectivity (e.g., road network coverage) deeply
// =================================================================
void DFSUtil(const Graph& g, int u, vector<bool>& visited) {
    visited[u] = true;
    cout << u << " ";
    for (int v : g.adjList[u]) {
        if (!visited[v]) {
            DFSUtil(g, v, visited);
        }
    }
}

void DFS(const Graph& g, int start) {
    vector<bool> visited(g.V, false);
    cout << "DFS (deep exploration from zone " << start << "): ";
    DFSUtil(g, start, visited);
    cout << "\n";
}

// =================================================================
// Dijkstra: Fastest route (non-negative weights) from rider to all zones
// =================================================================
void Dijkstra(const Graph& g, int src) {
    const int INF = 1e9;
    vector<int> dist(g.V, INF);

    // Min-heap based on current best known distance (ETA-like).
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;

    dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue; // outdated entry

        for (auto [v, w] : g.adjWeighted[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }

    cout << "Dijkstra shortest times from rider zone " << src << ":\n";
    for (int i = 0; i < g.V; ++i) {
        cout << "  to zone " << i << " : " << dist[i] << "\n";
    }
}

// =================================================================
// Bellman-Ford: Shortest paths even with negative edges (e.g., penalties)
// =================================================================
void BellmanFord(const Graph& g, int src) {
    const int INF = 1e9;
    vector<int> dist(g.V, INF);
    dist[src] = 0;

    // Relax all edges V-1 times.
    for (int i = 1; i <= g.V - 1; ++i) {
        for (const auto& e : g.edges) {
            if (dist[e.u] < INF && dist[e.u] + e.w < dist[e.v]) {
                dist[e.v] = dist[e.u] + e.w;
            }
        }
    }

    // Detect negative cycles (would mean inconsistent cost model).
    bool hasNegCycle = false;
    for (const auto& e : g.edges) {
        if (dist[e.u] < INF && dist[e.u] + e.w < dist[e.v]) {
            hasNegCycle = true;
            break;
        }
    }

    if (hasNegCycle) {
        cout << "Bellman-Ford: Negative cycle detected in cost model\n";
    } else {
        cout << "Bellman-Ford costs from rider zone " << src << ":\n";
        for (int i = 0; i < g.V; ++i) {
            cout << "  to zone " << i << " : " << dist[i] << "\n";
        }
    }
}

// =================================================================
// Queue: Ride request buffer (FIFO)
// =================================================================
void demoRequestQueue() {
    queue<int> rideRequests; // holds request IDs in arrival order

    int n;
    cout << "Enter number of incoming ride requests: ";
    cin >> n;

    cout << "Enter " << n << " ride request IDs:\n";
    for (int i = 0; i < n; ++i) {
        int id;
        cin >> id;
        rideRequests.push(id);
    }

    cout << "Serving ride requests (FIFO): ";
    while (!rideRequests.empty()) {
        int reqId = rideRequests.front();
        rideRequests.pop();
        cout << reqId << " ";
        // Here dispatch logic would pick a driver for reqId.
    }
    cout << "\n";
}

// =================================================================
// Hashing: Fast driver lookup (unordered_map)
// =================================================================
void demoHashing() {
    unordered_map<int, string> driverStatus;

    int n;
    cout << "Enter number of drivers for hash map demo: ";
    cin >> n;

    cout << "Enter driverID and status string for " << n << " drivers:\n";
    // note: status may contain spaces; use getline carefully
    for (int i = 0; i < n; ++i) {
        int id;
        string status;
        cin >> id;
        getline(cin, status);       // read rest of line (may be empty first)
        if (status.empty()) getline(cin, status);
        driverStatus[id] = status;
    }

    cout << "Driver status using hash map:\n";
    for (auto& p : driverStatus) {
        cout << "  Driver " << p.first << " :" << p.second << "\n";
    }

    cout << "Enter a driverID to lookup: ";
    int queryId;
    cin >> queryId;
    auto it = driverStatus.find(queryId);
    if (it != driverStatus.end()) {
        cout << "Lookup: Driver " << queryId << " ->" << it->second << "\n";
    } else {
        cout << "Lookup: Driver " << queryId << " not found\n";
    }
}

// =================================================================
// Min-heap (priority_queue): choose closest / fastest driver by ETA
// =================================================================
void demoMinHeapForDrivers() {
    // (ETA_to_rider, driverID) min-heap
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> minHeap;

    int n;
    cout << "Enter number of candidate drivers for min-heap demo: ";
    cin >> n;

    cout << "Enter " << n << " pairs: <ETA_in_minutes> <driverID>:\n";
    for (int i = 0; i < n; ++i) {
        int eta, id;
        cin >> eta >> id;
        minHeap.push({eta, id});
    }

    cout << "Choosing drivers by smallest ETA:\n";
    while (!minHeap.empty()) {
        auto [eta, driverId] = minHeap.top();
        minHeap.pop();
        cout << "  Assign driver " << driverId << " (ETA " << eta << " min)\n";
    }
}

// =================================================================
// Main: tie everything to an input-driven example
// =================================================================
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Graph g;

    int V, E_unweighted, E_weighted;
    cout << "Enter number of zones (V): ";
    cin >> V;
    g.resize(V);

    cout << "Enter number of unweighted edges (for BFS/DFS): ";
    cin >> E_unweighted;
    cout << "Enter " << E_unweighted << " edges as: u v (0-indexed zones):\n";
    for (int i = 0; i < E_unweighted; ++i) {
        int u, v;
        cin >> u >> v;
        g.addEdge(u, v);
    }

    cout << "Enter number of weighted edges (for Dijkstra/Bellman-Ford): ";
    cin >> E_weighted;
    cout << "Enter " << E_weighted << " edges as: u v w (w = travel time or cost):\n";
    for (int i = 0; i < E_weighted; ++i) {
        int u, v, w;
        cin >> u >> v >> w;
        g.addWeightedEdge(u, v, w);
    }

    int riderZone;
    cout << "Enter rider zone (source node index): ";
    cin >> riderZone;

    // Graph algorithms in the ride-sharing context
    BFS(g, riderZone);
    DFS(g, riderZone);
    Dijkstra(g, riderZone);
    BellmanFord(g, riderZone);

    // Data-structure demos for dispatch logic
    demoRequestQueue();
    demoHashing();
    demoMinHeapForDrivers();

    return 0;
}
