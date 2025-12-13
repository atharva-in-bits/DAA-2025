// Ride-Sharing Dispatch Demo
// Algorithms: BFS, DFS, Dijkstra, Bellman-Ford
// Data structures: queue (request buffer), priority_queue (min-heap for closest driver),
// unordered_map (hashing for fast driver lookup)

#include <bits/stdc++.h>
using namespace std;

// -------------------- Shared Graph Structure --------------------
struct Edge {
    int u, v;
    int w;
};

class Graph {
public:
    int V;
    vector<vector<int>> adjList;
    vector<vector<pair<int,int>>> adjWeighted;
    vector<Edge> edges;

    Graph(int n = 0) {
        resize(n);
    }

    void resize(int n) {
        V = n;
        adjList.assign(V, {});
        adjWeighted.assign(V, {});
        edges.clear();
    }

    void addEdge(int u, int v) {
        adjList[u].push_back(v);
        adjList[v].push_back(u);
    }

    void addWeightedEdge(int u, int v, int w) {
        adjWeighted[u].push_back({v, w});
        edges.push_back({u, v, w});
    }
};

// =================================================================
// BFS
// =================================================================
void BFS(const Graph& g, int start) {
    vector<bool> visited(g.V, false);
    queue<int> q;

    visited[start] = true;
    q.push(start);

    cout << "BFS Reachable Zones: ";
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        cout << u << " ";
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
// DFS
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
    cout << "DFS Traversal: ";
    DFSUtil(g, start, visited);
    cout << "\n";
}

// =================================================================
// Dijkstra
// =================================================================
void Dijkstra(const Graph& g, int src) {
    const int INF = 1e9;
    vector<int> dist(g.V, INF);

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue;

        for (auto [v, w] : g.adjWeighted[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }

    cout << "Dijkstra Shortest Times:\n";
    for (int i = 0; i < g.V; ++i) {
        cout << "Zone " << i << " -> " << dist[i] << "\n";
    }
}

// =================================================================
// Bellman-Ford
// =================================================================
void BellmanFord(const Graph& g, int src) {
    const int INF = 1e9;
    vector<int> dist(g.V, INF);
    dist[src] = 0;

    for (int i = 1; i <= g.V - 1; ++i) {
        for (auto &e : g.edges) {
            if (dist[e.u] < INF && dist[e.u] + e.w < dist[e.v]) {
                dist[e.v] = dist[e.u] + e.w;
            }
        }
    }

    for (auto &e : g.edges) {
        if (dist[e.u] < INF && dist[e.u] + e.w < dist[e.v]) {
            cout << "Negative cycle detected\n";
            return;
        }
    }

    cout << "Bellman-Ford Costs:\n";
    for (int i = 0; i < g.V; ++i) {
        cout << "Zone " << i << " -> " << dist[i] << "\n";
    }
}

// =================================================================
// Queue: Ride Request Buffer
// =================================================================
void demoRequestQueue() {
    int requestCount;
    cin >> requestCount;

    queue<int> requests;
    for (int i = 0; i < requestCount; ++i) {
        int reqId;
        cin >> reqId;
        requests.push(reqId);
    }

    cout << "Ride Request Order: ";
    while (!requests.empty()) {
        cout << requests.front() << " ";
        requests.pop();
    }
    cout << "\n";
}

// =================================================================
// Hashing: Driver Lookup
// =================================================================
void demoHashing() {
    int driverCount;
    cin >> driverCount;

    unordered_map<int, string> driverMap;
    for (int i = 0; i < driverCount; ++i) {
        int id;
        string status;
        cin >> id;
        cin.ignore();
        getline(cin, status);
        driverMap[id] = status;
    }

    int queryId;
    cin >> queryId;

    if (driverMap.find(queryId) != driverMap.end()) {
        cout << "Driver " << queryId << ": " << driverMap[queryId] << "\n";
    } else {
        cout << "Driver not found\n";
    }
}

// =================================================================
// Min-Heap: Closest Driver Selection
// =================================================================
void demoMinHeapForDrivers() {
    int n;
    cin >> n;

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    for (int i = 0; i < n; ++i) {
        int eta, id;
        cin >> eta >> id;
        pq.push({eta, id});
    }

    cout << "Driver Assignment Order:\n";
    while (!pq.empty()) {
        auto [eta, id] = pq.top();
        pq.pop();
        cout << "Driver " << id << " ETA " << eta << "\n";
    }
}

// =================================================================
// MAIN
// =================================================================
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int V;
    cin >> V;

    Graph g(V);

    int E1;
    cin >> E1;
    for (int i = 0; i < E1; ++i) {
        int u, v;
        cin >> u >> v;
        g.addEdge(u, v);
    }

    int E2;
    cin >> E2;
    for (int i = 0; i < E2; ++i) {
        int u, v, w;
        cin >> u >> v >> w;
        g.addWeightedEdge(u, v, w);
    }

    int riderZone;
    cin >> riderZone;

    BFS(g, riderZone);
    DFS(g, riderZone);
    Dijkstra(g, riderZone);
    BellmanFord(g, riderZone);

    demoRequestQueue();
    demoHashing();
    demoMinHeapForDrivers();

    return 0;
}
