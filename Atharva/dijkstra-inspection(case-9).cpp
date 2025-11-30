// File: Atharva/dijkstra-inspection(case-9).cpp
// Compile with: g++ -std=c++17 -O2 dijkstra-inspection(case-9).cpp -o dijkstra_inspection

#include <bits/stdc++.h>
using namespace std;

const double INF = 1e18;

// ============================================================================
// Node structure: represents warehouses, food hubs, data centers, markets, etc.
// ============================================================================

struct Node {
    int id;
    string name;
    string type;   // "warehouse", "shop", "datahub", "inspection-point"
};

// ============================================================================
// Graph (adjacency list) for inspector routing
// ============================================================================

class Graph {
public:
    struct Edge {
        int v;
        double w;
    };

    Graph(int n = 0) { init(n); }

    void init(int n) {
        N = n;
        adj.assign(N, vector<Edge>());
    }

    void addEdge(int u, int v, double w) {
        if (valid(u) && valid(v)) {
            adj[u].push_back({v, w});
        }
    }

    int size() const { return N; }

    const vector<vector<Edge>>& getAdj() const { return adj; }

private:
    int N;
    vector<vector<Edge>> adj;

    bool valid(int x) const { return x >= 0 && x < N; }
};

// ============================================================================
// Dijkstra Implementation
// ============================================================================

struct DijkstraResult {
    vector<double> dist;
    vector<int> parent;
};

class Dijkstra {
public:
    static DijkstraResult run(const Graph &g, int source) {
        int n = g.size();
        DijkstraResult res;
        res.dist.assign(n, INF);
        res.parent.assign(n, -1);

        res.dist[source] = 0.0;

        using P = pair<double,int>;  // (dist,node)
        priority_queue<P, vector<P>, greater<P>> pq;
        pq.push({0.0, source});

        const auto &adj = g.getAdj();

        while (!pq.empty()) {
            auto [cd, u] = pq.top();
            pq.pop();
            if (cd > res.dist[u]) continue;

            for (auto &e : adj[u]) {
                double nd = cd + e.w;
                if (nd < res.dist[e.v]) {
                    res.dist[e.v] = nd;
                    res.parent[e.v] = u;
                    pq.push({nd, e.v});
                }
            }
        }
        return res;
    }

    static vector<int> path(int target, const vector<int> &parent) {
        vector<int> p;
        if (target < 0) return p;
        int cur = target;
        while (cur != -1) {
            p.push_back(cur);
            cur = parent[cur];
        }
        reverse(p.begin(), p.end());
        return p;
    }
};

// ============================================================================
// Inspector Routing System for Aroha Nagar
// ============================================================================

class InspectionSystem {
public:
    void addNode(const string &name, const string &type) {
        int id = nodes.size();
        nodes.push_back({id, name, type});
    }

    void allocateGraph() {
        g.init(nodes.size());
    }

    bool addRoute(int u, int v, double cost) {
        if (!valid(u) || !valid(v)) return false;
        g.addEdge(u, v, cost);
        g.addEdge(v, u, cost); // undirected roads
        return true;
    }

    int count() const { return nodes.size(); }

    vector<Node> listNodes() const { return nodes; }

    const Node& getNode(int id) const { return nodes.at(id); }

    bool valid(int id) const { return id >= 0 && id < (int)nodes.size(); }

    DijkstraResult computeFrom(int src) {
        return Dijkstra::run(g, src);
    }

    vector<int> buildPath(int target, const vector<int> &parent) {
        return Dijkstra::path(target, parent);
    }

    // Export full distance array
    void exportDistances(const DijkstraResult &res, const string &file) {
        ofstream out(file);
        if (!out.is_open()) return;

        out << "nodeId,nodeName,distance\n";
        for (int i = 0; i < count(); ++i) {
            out << i << "," << nodes[i].name << ",";
            if (res.dist[i] >= INF/2) out << "INF";
            else out << fixed << setprecision(3) << res.dist[i];
            out << "\n";
        }
        out.close();
    }

    // Export path to CSV
    void exportPath(const vector<int> &path, const string &file) {
        ofstream out(file);
        if (!out.is_open()) return;
        out << "order,nodeId,nodeName\n";
        for (int i = 0; i < (int)path.size(); i++) {
            int id = path[i];
            out << i << "," << id << "," << nodes[id].name << "\n";
        }
        out.close();
    }

    // Decide next inspection targets based on type
    vector<int> allInspectionTargets() {
        vector<int> res;
        for (auto &n : nodes) {
            if (n.type == "inspection-point" || n.type == "warehouse") {
                res.push_back(n.id);
            }
        }
        return res;
    }

private:
    vector<Node> nodes;
    Graph g;
};

// ============================================================================
// CLI
// ============================================================================

static string readTrim(const string &prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b-a+1);
}

static int readInt(const string &prompt, int def) {
    string s = readTrim(prompt);
    if (s.empty()) return def;
    try { return stoi(s); } catch (...) { return def; }
}

static double readDouble(const string &prompt, double def) {
    string s = readTrim(prompt);
    if (s.empty()) return def;
    try { return stod(s); } catch (...) { return def; }
}

static void printPath(const vector<int> &p, const InspectionSystem &sys) {
    if (p.empty()) { cout << "No path.\n"; return; }
    for (int i = 0; i < (int)p.size(); i++) {
        cout << sys.getNode(p[i]).name;
        if (i+1 < (int)p.size()) cout << " -> ";
    }
    cout << "\n";
}

// CLI driver
class InspectionCLI {
public:
    InspectionCLI(InspectionSystem &s) : sys(s) {}

    void run() {
        bool quit = false;
        while (!quit) {
            menu();
            string c = readTrim("Choice> ");
            if (c == "1") listNodes();
            else if (c == "2") addNode();
            else if (c == "3") addRoute();
            else if (c == "4") computeRoutes();
            else if (c == "5") queryPath();
            else if (c == "6") exportDistances();
            else if (c == "7") exportPathFile();
            else if (c == "8") listInspectionTargets();
            else if (c == "q") quit = true;
        }
    }

private:
    InspectionSystem &sys;
    DijkstraResult last;
    int lastSource = -1;

    void menu() {
        cout << "\nAroha Nagar Inspection — Options\n"
             << " 1) List nodes\n"
             << " 2) Add node\n"
             << " 3) Add route (bidirectional)\n"
             << " 4) Compute Dijkstra from a source\n"
             << " 5) Query path to a node\n"
             << " 6) Export distance table\n"
             << " 7) Export a path\n"
             << " 8) List recommended inspection targets\n"
             << " q) Quit\n";
    }

    void listNodes() {
        auto v = sys.listNodes();
        cout << "Nodes:\n";
        for (auto &n : v) {
            cout << n.id << " | " << n.name << " | " << n.type << "\n";
        }
    }

    void addNode() {
        string name = readTrim("Name: ");
        string type = readTrim("Type (warehouse/shop/datahub/inspection-point): ");
        if (type.empty()) type = "inspection-point";
        sys.addNode(name, type);
        sys.allocateGraph();
        cout << "Node added.\n";
    }

    void addRoute() {
        int u = readInt("From: ", -1);
        int v = readInt("To: ", -1);
        double w = readDouble("Cost: ", 1.0);
        if (sys.addRoute(u, v, w)) cout << "Route added.\n";
        else cout << "Invalid route\n";
    }

    void computeRoutes() {
        int src = readInt("Source id: ", 0);
        last = sys.computeFrom(src);
        lastSource = src;
        cout << "Computed Dijkstra from " << src << "\n";
    }

    void queryPath() {
        if (lastSource == -1) { cout << "Compute Dijkstra first.\n"; return; }
        int t = readInt("Target id: ", -1);
        auto p = sys.buildPath(t, last.parent);
        cout << "Cost: " 
             << (last.dist[t] >= INF/2 ? -1 : last.dist[t]) 
             << "\n";
        printPath(p, sys);
    }

    void exportDistances() {
        string f = readTrim("Filename: ");
        if (f.empty()) f = "distances.csv";
        sys.exportDistances(last, f);
        cout << "Exported.\n";
    }

    void exportPathFile() {
        int t = readInt("Target id: ", -1);
        auto p = sys.buildPath(t, last.parent);
        string f = readTrim("Filename: ");
        if (f.empty()) f = "path.csv";
        sys.exportPath(p, f);
        cout << "Exported path.\n";
    }

    void listInspectionTargets() {
        auto v = sys.allInspectionTargets();
        cout << "Inspection targets:\n";
        for (int id : v) {
            cout << id << " | " << sys.getNode(id).name << "\n";
        }
    }
};

// Default population
static void populateDefault(InspectionSystem &sys) {
    sys.addNode("Central Warehouse","warehouse");
    sys.addNode("South Warehouse","warehouse");
    sys.addNode("Food Hub 1","inspection-point");
    sys.addNode("Food Hub 2","inspection-point");
    sys.addNode("Data Center","datahub");
    sys.addNode("Repair Market","shop");
    sys.allocateGraph();
    sys.addRoute(0,2,4.5);
    sys.addRoute(2,1,6.0);
    sys.addRoute(0,3,8.0);
    sys.addRoute(3,4,3.0);
    sys.addRoute(4,5,5.0);
    sys.addRoute(1,5,7.5);
}

int main(int argc, char **argv) {
    InspectionSystem sys;

    if (argc >= 2 && string(argv[1]) == "--demo") {
        populateDefault(sys);
        DijkstraResult res = sys.computeFrom(0);
        sys.exportDistances(res, "demo_dijkstra.csv");
        auto p = sys.buildPath(5, res.parent);
        sys.exportPath(p, "demo_path.csv");
        cout << "Demo export done.\n";
        return 0;
    }

    populateDefault(sys);
    InspectionCLI cli(sys);
    cli.run();
    return 0;
}
