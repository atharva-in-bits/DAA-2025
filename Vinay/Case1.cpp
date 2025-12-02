// cloud_kitchen.cpp
// Single-file simulation of a Cloud Kitchen & Food Delivery module
// Uses: unordered_map (hashing), priority_queue (scheduling), Dijkstra (routing),
//       Fenwick Tree (cumulative sales tracking).
//
// Compile: g++ -std=c++17 cloud_kitchen.cpp -O2 -o cloud_kitchen
// Run: ./cloud_kitchen
//
// Author: ChatGPT (tailored for Vinay's planned-city project)

#include <bits/stdc++.h>
using namespace std;

/* ---------------------------
   Fenwick Tree (Binary Indexed Tree)
   - Supports point updates and prefix sum queries in O(log N).
   - We'll use this to track cumulative item sales (popularity analytics).
   --------------------------- */
struct Fenwick {
    int n;
    vector<long long> bit;
    Fenwick() : n(0) {}
    Fenwick(int n_) : n(n_), bit(n_ + 1, 0) {}
    void init(int n_) { n = n_; bit.assign(n + 1, 0); }
    // add value to index i (1-indexed)
    void add(int i, long long delta) {
        for (; i <= n; i += i & -i) bit[i] += delta;
    }
    // prefix sum [1..i]
    long long sum(int i) const {
        long long s = 0;
        for (; i > 0; i -= i & -i) s += bit[i];
        return s;
    }
    // range sum [l..r]
    long long rangeSum(int l, int r) const {
        if (r < l) return 0;
        return sum(r) - sum(l - 1);
    }
};

/* ---------------------------
   Graph + Dijkstra (routing)
   - Weighted directed or undirected graph using adjacency list.
   - Dijkstra with priority_queue (min-heap) O((V+E) log V)
   --------------------------- */

struct Edge {
    int to;
    double w;
    Edge(int _to, double _w) : to(_to), w(_w) {}
};

struct Graph {
    int V;
    vector<vector<Edge>> adj;
    Graph(int V = 0) { init(V); }
    void init(int n) { V = n; adj.assign(V, {}); }
    void addEdge(int u, int v, double w, bool undirected = true) {
        adj[u].push_back(Edge(v, w));
        if (undirected) adj[v].push_back(Edge(u, w));
    }

    // Dijkstra from source -> distances
    vector<double> dijkstra(int src) const {
        const double INF = 1e18;
        vector<double> dist(V, INF);
        dist[src] = 0;
        using P = pair<double, int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        pq.push({0.0, src});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;
            for (auto &e : adj[u]) {
                int v = e.to;
                double nd = d + e.w;
                if (nd < dist[v]) {
                    dist[v] = nd;
                    pq.push({nd, v});
                }
            }
        }
        return dist;
    }

    // single-pair shortest distance (runs dijkstra until target found)
    double shortestPath(int src, int target) const {
        const double INF = 1e18;
        vector<double> dist(V, INF);
        dist[src] = 0;
        using P = pair<double, int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        pq.push({0.0, src});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;
            if (u == target) return d;
            for (auto &e : adj[u]) {
                int v = e.to;
                double nd = d + e.w;
                if (nd < dist[v]) {
                    dist[v] = nd;
                    pq.push({nd, v});
                }
            }
        }
        return INF;
    }
};

/* ---------------------------
   Order representation and scheduling
   - Orders have priority (e.g., earlier ETA, or higher-value).
   - Kitchen processes up to capacity per tick.
   --------------------------- */
struct Order {
    int id;
    int customerNode;              // node ID on city graph
    vector<string> items;          // item names
    double price;
    int placedTime;                // simulation time when placed
    int priority;                  // lower => higher priority (e.g., earlier deadline)
    Order() {}
    Order(int id_, int node_, const vector<string>& it, double pr, int time_, int p_)
        : id(id_), customerNode(node_), items(it), price(pr), placedTime(time_), priority(p_) {}
};

// comparator: min priority first, then earlier placed
struct OrderCompare {
    bool operator()(const Order &a, const Order &b) const {
        if (a.priority != b.priority) return a.priority > b.priority;
        return a.placedTime > b.placedTime;
    }
};

/* ---------------------------
   CloudKitchen: core business class
   - menu: item -> (price, stock)
   - inventory: item -> stock
   - order queue: priority_queue for scheduling
   - sales tracking: Fenwick tree over items mapped to indices
   - graph reference to compute delivery distances
   --------------------------- */
class CloudKitchen {
private:
    string name;
    int nodeLocation;                              // graph node where kitchen sits
    int capacity;                                  // concurrent orders per processing tick
    int currentTime = 0;
    int nextOrderId = 1;

    // menu and inventory using hashing
    struct MenuItem {
        double price;
        int stock; // available quantity
        MenuItem() {}
        MenuItem(double p, int s) : price(p), stock(s) {}
    };
    unordered_map<string, MenuItem> menu;          // O(1) average lookup
    unordered_map<string, int> itemIndex;          // item -> fenwick index (1-based)
    vector<string> indexToItem;                    // index -> item name
    Fenwick fenw;                                  // track total sold per item (by index)

    // pending orders
    priority_queue<Order, vector<Order>, OrderCompare> pending;

    Graph* cityGraph = nullptr;                    // pointer to global city routing graph

public:
    CloudKitchen(const string &n, int nodeLoc, int cap) 
        : name(n), nodeLocation(nodeLoc), capacity(cap) {}

    void attachGraph(Graph* g) { cityGraph = g; }

    // add or update menu item
    void addMenuItem(const string &item, double price, int stock) {
        menu[item] = MenuItem(price, stock);
        if (itemIndex.find(item) == itemIndex.end()) {
            int idx = (int)indexToItem.size();
            indexToItem.push_back(item);
            itemIndex[item] = idx + 1; // fenwick 1-indexed
            fenw.init((int)indexToItem.size());
        }
    }

    // restock item
    void restock(const string &item, int amount) {
        if (menu.find(item) != menu.end()) {
            menu[item].stock += amount;
        } else {
            cerr << "Restock: item not in menu: " << item << "\n";
        }
    }

    // Place an order; returns whether accepted (stock available)
    bool placeOrder(int customerNode, const vector<string>& items, int priority = 100) {
        double total = 0.0;
        // check availability
        unordered_map<string, int> counts;
        for (auto &it : items) counts[it]++;
        for (auto &p : counts) {
            auto it = menu.find(p.first);
            if (it == menu.end() || it->second.stock < p.second) {
                // cannot fulfill
                return false;
            }
            total += it->second.price * p.second;
        }
        // deduct stock immediately (reserve)
        for (auto &p : counts) {
            menu[p.first].stock -= p.second;
        }
        Order ord(nextOrderId++, customerNode, items, total, currentTime, priority);
        pending.push(ord);
        return true;
    }

    // Process up to capacity orders: assign deliveries and mark sales
    void processOrdersTick() {
        cout << "\n[" << name << "] Processing tick at time " << currentTime << "\n";
        int processed = 0;
        vector<Order> delivered; // keep delivered for stats
        while (!pending.empty() && processed < capacity) {
            Order ord = pending.top(); pending.pop();
            // compute distance/time using Dijkstra if graph attached
            double dist = -1;
            if (cityGraph) {
                double d = cityGraph->shortestPath(nodeLocation, ord.customerNode);
                if (d >= 1e17) dist = -1; else dist = d;
            }
            cout << "  -> Order#" << ord.id << " to node " << ord.customerNode
                 << " total=" << ord.price
                 << (dist >= 0 ? (" dist=" + to_string(dist)) : " dist=NA") << "\n";
            // mark sales in fenwick
            for (auto &it : ord.items) {
                int idx = itemIndex[it];
                fenw.add(idx, 1);
            }
            processed++;
            delivered.push_back(ord);
        }
        if (processed == 0) cout << "  No orders processed this tick.\n";
        currentTime++;
    }

    // Query cumulative sales for an item or range of items
    long long getItemSales(const string &item) const {
        auto it = itemIndex.find(item);
        if (it == itemIndex.end()) return 0;
        return fenw.sum(it->second);
    }
    long long getRangeSales(int lIdx, int rIdx) const {
        return fenw.rangeSum(lIdx, rIdx);
    }

    // status
    void status() const {
        cout << "\nKitchen: " << name << " at node " << nodeLocation
             << " | capacity=" << capacity << " | pending=" << pending.size() << "\n";
        cout << "Menu (item:price:stock):\n";
        for (auto &p : menu) {
            cout << "  " << p.first << " : " << p.second.price << " : " << p.second.stock << "\n";
        }
    }

    // get next pending count
    int pendingCount() const { return (int)pending.size(); }
};

/* ---------------------------
   Demo / CLI-like example
   - Builds a small graph, creates a kitchen, demonstrates placing orders,
     processing ticks, and checking analytics (Fenwick).
   --------------------------- */

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // 1) Build small sample city graph (7 nodes)
    // Nodes: 0..6 representing zones/areas
    Graph city(7);
    city.addEdge(0,1,2.5);
    city.addEdge(1,2,1.2);
    city.addEdge(1,3,2.0);
    city.addEdge(0,4,4.0);
    city.addEdge(4,5,1.0);
    city.addEdge(5,6,1.5);
    city.addEdge(2,6,3.0);
    city.addEdge(3,6,2.2);

    // 2) Create CloudKitchen at node 0 with capacity 3 orders per tick
    CloudKitchen ck("QuickBites", 0, 3);
    ck.attachGraph(&city);

    // 3) Add menu and stock
    ck.addMenuItem("Burger", 80.0, 10);
    ck.addMenuItem("Fries", 30.0, 20);
    ck.addMenuItem("Noodles", 100.0, 6);
    ck.addMenuItem("Pizza", 200.0, 4);

    // 4) Show initial status
    ck.status();

    // 5) Place some orders (customer nodes vary across graph)
    vector<pair<int, vector<string>>> orders = {
        {2, {"Burger","Fries"}},
        {6, {"Pizza"}},
        {5, {"Noodles"}},
        {3, {"Burger"}},
        {6, {"Burger","Fries","Fries"}},
        {4, {"Fries","Fries","Fries"}},
        {1, {"Noodles","Fries"}},
    };

    cout << "\nPlacing orders:\n";
    int i=0;
    for (auto &o : orders) {
        bool ok = ck.placeOrder(o.first, o.second, 50 + (i%3)); // some priority tweak
        cout << "  Order to node " << o.first << " items {";
        for (auto &it : o.second) cout << it << ",";
        cout << "} => " << (ok ? "ACCEPTED" : "REJECTED (stock)") << "\n";
        i++;
    }

    // 6) Process a few ticks and show outputs
    ck.processOrdersTick();
    ck.processOrdersTick();
    ck.processOrdersTick();

    // 7) After processing, status and analytics
    ck.status();
    cout << "\nAnalytics: sales of Burger = " << ck.getItemSales("Burger") << "\n";
    cout << "Analytics: sales of Fries = " << ck.getItemSales("Fries") << "\n";
    cout << "Analytics: sales of Noodles = " << ck.getItemSales("Noodles") << "\n";
    cout << "Pending orders left = " << ck.pendingCount() << "\n";

    // 8) Example: shortest path distance from kitchen(0) to node 6 (for routing/recall)
    cout << "\nRouting demo: shortest distance from kitchen (node 0) to node 6 = "
         << city.shortestPath(0,6) << "\n";

    return 0;
}
