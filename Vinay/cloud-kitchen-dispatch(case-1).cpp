#include <bits/stdc++.h>
using namespace std;

// ================== CONFIG ==================
const int MAX_AREAS = 50;
const int INF = 1e9;

// ================== DATA MODELS =============
struct MenuItem {
    int id;
    string name;
    int prepTime;       // minutes
    double price;
};

struct Order {
    int orderId;
    int customerArea;   // node in graph
    vector<int> dishIds;
    int totalPrepTime;
    int promisedTime;   // minutes from "now"
};

// for min-heap on promisedTime
struct OrderCmp {
    bool operator()(const Order &a, const Order &b) const {
        return a.promisedTime > b.promisedTime;
    }
};

// ================== GLOBAL STATE ============
// hashing
unordered_map<int, MenuItem> menuMap;          // dishId -> MenuItem
unordered_map<int, Order>    orderMap;         // orderId -> Order

// heap of active orders (earliest promised first)
priority_queue<Order, vector<Order>, OrderCmp> orderPQ;

// graph for rider routing
int numAreas;
int kitchenArea = 1;
vector<pair<int,int>> adj[MAX_AREAS + 1];      // (neighbor, travelTime)

// incremental order id
int nextOrderId = 1;

// ================== GRAPH / DIJKSTRA ========
void addRoad(int u, int v, int w) {
    if (u < 1 || v < 1 || u > numAreas || v > numAreas) {
        cout << "Invalid road endpoints.\n";
        return;
    }
    adj[u].push_back({v, w});
    adj[v].push_back({u, w});
}

vector<int> dijkstra(int src) {
    vector<int> dist(numAreas + 1, INF);
    priority_queue<pair<int,int>, vector<pair<int,int>>,
                   greater<pair<int,int>>> pq;

    dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d != dist[u]) continue;
        for (auto &e : adj[u]) {
            int v = e.first, w = e.second;
            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

// ================== MENU OPS =================
void addMenuItem() {
    MenuItem m;
    cout << "Enter dish id: ";
    cin >> m.id;
    cin.ignore();
    cout << "Enter dish name: ";
    getline(cin, m.name);
    cout << "Enter prep time (minutes): ";
    cin >> m.prepTime;
    cout << "Enter price: ";
    cin >> m.price;

    menuMap[m.id] = m;
    cout << "Menu item added.\n";
}

void showMenu() {
    if (menuMap.empty()) {
        cout << "Menu is empty.\n";
        return;
    }
    cout << "----- MENU -----\n";
    for (auto &p : menuMap) {
        const MenuItem &m = p.second;
        cout << "ID " << m.id << " | " << m.name
             << " | Prep " << m.prepTime << " mins"
             << " | Rs " << m.price << "\n";
    }
}

// ================== ORDER OPS ================
int computePrepTime(const vector<int> &dishIds) {
    int total = 0;
    for (int id : dishIds) {
        if (menuMap.count(id)) total += menuMap[id].prepTime;
    }
    return total;
}

void placeOrder() {
    int area;
    cout << "Enter customer area node (1.." << numAreas << "): ";
    cin >> area;
    if (area < 1 || area > numAreas) {
        cout << "Invalid area.\n";
        return;
    }

    int k;
    cout << "Enter number of dishes: ";
    cin >> k;
    vector<int> dishIds;
    cout << "Enter dish IDs:\n";
    for (int i = 0; i < k; ++i) {
        int id;
        cin >> id;
        if (!menuMap.count(id)) {
            cout << "Dish " << id << " not in menu. Skipping.\n";
        } else {
            dishIds.push_back(id);
        }
    }
    if (dishIds.empty()) {
        cout << "No valid dishes; order cancelled.\n";
        return;
    }

    int promised;
    cout << "Enter promised delivery time from now (minutes): ";
    cin >> promised;

    Order o;
    o.orderId      = nextOrderId++;
    o.customerArea = area;
    o.dishIds      = dishIds;
    o.totalPrepTime = computePrepTime(dishIds);
    o.promisedTime = promised;

    orderPQ.push(o);
    orderMap[o.orderId] = o;

    cout << "Order placed. ID = " << o.orderId
         << ", total prep = " << o.totalPrepTime << " mins\n";
}

void viewPendingOrders() {
    if (orderPQ.empty()) {
        cout << "No pending orders.\n";
        return;
    }
    cout << "----- PENDING ORDERS (earliest promised first) -----\n";
    auto temp = orderPQ;
    while (!temp.empty()) {
        Order o = temp.top();
        temp.pop();
        cout << "Order " << o.orderId
             << " | Area " << o.customerArea
             << " | Prep " << o.totalPrepTime
             << " | Promised in " << o.promisedTime << " mins\n";
    }
}

// ================== RIDER DISPATCH ===========
void dispatchNextOrder() {
    if (orderPQ.empty()) {
        cout << "No orders to dispatch.\n";
        return;
    }

    // recompute distances from kitchen
    vector<int> dist = dijkstra(kitchenArea);

    Order o = orderPQ.top();
    orderPQ.pop();
    orderMap.erase(o.orderId);

    int travel = dist[o.customerArea];
    if (travel >= INF) {
        cout << "Order " << o.orderId << ": customer area unreachable.\n";
        return;
    }

    cout << "Dispatching order " << o.orderId << ":\n";
    cout << "Customer area: " << o.customerArea << "\n";
    cout << "Shortest travel time from kitchen: " << travel << " units\n";
    cout << "Total prep time: " << o.totalPrepTime << " mins\n";
    cout << "Promised delivery window: " << o.promisedTime << " mins\n";
}

// ================== CITY SETUP ===============
void setupCity() {
    cout << "Enter number of areas (nodes, <= " << MAX_AREAS << "): ";
    cin >> numAreas;
    if (numAreas < 1 || numAreas > MAX_AREAS) {
        cout << "Invalid, setting numAreas = 10.\n";
        numAreas = 10;
    }

    int m;
    cout << "Enter number of roads (edges): ";
    cin >> m;
    cout << "Enter roads as: u v travel_time\n";
    for (int i = 0; i < m; ++i) {
        int u, v, w;
        cin >> u >> v >> w;
        addRoad(u, v, w);
    }

    cout << "Enter kitchen area node (1.." << numAreas << "): ";
    cin >> kitchenArea;
    if (kitchenArea < 1 || kitchenArea > numAreas) {
        cout << "Invalid, setting kitchenArea = 1.\n";
        kitchenArea = 1;
    }
}

// ================== MAIN MENU ================
void printMenu() {
    cout << "\n==== Smart Cloud Kitchen & Rider Dispatch ====\n";
    cout << "1. Add menu item\n";
    cout << "2. Show menu\n";
    cout << "3. Place new order\n";
    cout << "4. View pending orders\n";
    cout << "5. Dispatch next order (heap + Dijkstra)\n";
    cout << "0. Exit\n";
    cout << "Choice: ";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    setupCity();

    int ch;
    do {
        printMenu();
        cin >> ch;
        switch (ch) {
            case 1: addMenuItem(); break;
            case 2: showMenu(); break;
            case 3: placeOrder(); break;
            case 4: viewPendingOrders(); break;
            case 5: dispatchNextOrder(); break;
            case 0: cout << "Exiting.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (ch != 0);

    return 0;
}
