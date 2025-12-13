#include <bits/stdc++.h>
using namespace std;

// -------------------- Basic Graph Structures --------------------

struct Edge {
    int u, v;
    int cost;
};

struct Depot {
    int id;
    double x, y;
};

// -------------------- Union-Find --------------------

class UnionFind {
public:
    vector<int> parent, rankVec;

    UnionFind(int n = 0) { init(n); }

    void init(int n) {
        parent.resize(n);
        rankVec.assign(n, 0);
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]);
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (rankVec[a] < rankVec[b]) swap(a, b);
        parent[b] = a;
        if (rankVec[a] == rankVec[b]) rankVec[a]++;
        return true;
    }
};

// -------------------- Kruskal MST --------------------

vector<Edge> kruskalMST(int numDepots, vector<Edge>& edges) {
    sort(edges.begin(), edges.end(),
         [](const Edge& a, const Edge& b) { return a.cost < b.cost; });

    UnionFind uf(numDepots);
    vector<Edge> mst;
    int totalCost = 0;

    for (auto& e : edges) {
        if (uf.unite(e.u, e.v)) {
            mst.push_back(e);
            totalCost += e.cost;
        }
    }

    cout << totalCost << "\n";
    for (auto& e : mst) {
        cout << e.u << " " << e.v << " " << e.cost << "\n";
    }
    return mst;
}

// -------------------- Prim MST --------------------

vector<Edge> primMST(int numDepots, vector<vector<pair<int,int>>>& adj) {
    const int INF = 1e9;
    vector<int> key(numDepots, INF), parent(numDepots, -1);
    vector<bool> inMST(numDepots, false);

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    key[0] = 0;
    pq.push({0, 0});

    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();
        if (inMST[u]) continue;
        inMST[u] = true;

        for (auto& [v, w] : adj[u]) {
            if (!inMST[v] && w < key[v]) {
                key[v] = w;
                parent[v] = u;
                pq.push({key[v], v});
            }
        }
    }

    vector<Edge> mst;
    int totalCost = 0;
    for (int i = 1; i < numDepots; i++) {
        if (parent[i] != -1) {
            mst.push_back({parent[i], i, key[i]});
            totalCost += key[i];
        }
    }

    cout << totalCost << "\n";
    for (auto& e : mst) {
        cout << e.u << " " << e.v << " " << e.cost << "\n";
    }
    return mst;
}

// -------------------- Order Sorting --------------------

struct Order {
    int id;
    int depotId;
    int priority;
    int dueTime;
};

void sortOrders(vector<Order>& orders) {
    sort(orders.begin(), orders.end(), [](const Order& a, const Order& b) {
        if (a.priority != b.priority) return a.priority < b.priority;
        return a.dueTime < b.dueTime;
    });

    for (auto& o : orders) {
        cout << o.id << " " << o.depotId << " "
             << o.priority << " " << o.dueTime << "\n";
    }
}

// -------------------- Route List --------------------

void buildRoute(list<int>& route) {
    for (int stop : route) cout << stop << " ";
    cout << "\n";
}

// -------------------- MAIN --------------------

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // ---------- Depots ----------
    int numDepots;
    cin >> numDepots;

    vector<Depot> depots(numDepots);
    for (int i = 0; i < numDepots; i++) {
        cin >> depots[i].id >> depots[i].x >> depots[i].y;
    }

    // ---------- Edges ----------
    int edgeCount;
    cin >> edgeCount;

    vector<Edge> edges(edgeCount);
    for (int i = 0; i < edgeCount; i++) {
        cin >> edges[i].u >> edges[i].v >> edges[i].cost;
    }

    // ---------- Kruskal ----------
    kruskalMST(numDepots, edges);

    // ---------- Build adjacency for Prim ----------
    vector<vector<pair<int,int>>> adj(numDepots);
    for (auto& e : edges) {
        adj[e.u].push_back({e.v, e.cost});
        adj[e.v].push_back({e.u, e.cost});
    }

    // ---------- Prim ----------
    primMST(numDepots, adj);

    // ---------- Orders ----------
    int orderCount;
    cin >> orderCount;

    vector<Order> orders(orderCount);
    for (int i = 0; i < orderCount; i++) {
        cin >> orders[i].id
            >> orders[i].depotId
            >> orders[i].priority
            >> orders[i].dueTime;
    }

    sortOrders(orders);

    // ---------- Route ----------
    int routeSize;
    cin >> routeSize;

    list<int> route;
    for (int i = 0; i < routeSize; i++) {
        int stop;
        cin >> stop;
        route.push_back(stop);
    }

    buildRoute(route);

    int insertAfter, newStop;
    cin >> insertAfter >> newStop;

    auto it = find(route.begin(), route.end(), insertAfter);
    if (it != route.end()) {
        ++it;
        route.insert(it, newStop);
    }

    buildRoute(route);

    return 0;
}

