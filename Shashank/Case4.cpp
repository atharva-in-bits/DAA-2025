
// Waste Processing & Recycling Operations Demo
// Purpose: Sort incoming waste and manage conveyor workflows.
//
// Algorithms / DS used:
// - Heap (priority_queue): prioritize perishable / urgent loads
// - Queue: conveyor / truck buffer
// - Selection / Insertion / Bubble Sort: simple classification ordering
// - Union-Find: grouping recyclables by compatible material type
// - DFS: detect process bottlenecks in a plant flow graph
// - Arrays / Structures: store batch data (weight, type, timestamps)

#include <bits/stdc++.h>
using namespace std;

// -------------------- Basic Structures --------------------

enum WasteType {
    ORGANIC = 0,
    PLASTIC = 1,
    METAL   = 2,
    PAPER   = 3,
    GLASS   = 4
};

struct WasteBatch {
    int id;
    WasteType type;
    double weightKg;
    int arrivalTime;   // time index when batch arrives
    int perishScore;   // higher = more urgent (e.g., rotting organic)
};

// For DFS process graph: each node is a processing station.
struct StationNode {
    int id;
    string name;
};

// =================================================================
// HEAP: prioritize perishable loads
// =================================================================
struct HeapItem {
    int batchId;
    int perishScore;  // higher = more urgent
    int arrivalTime;  // earlier gets priority if perishScore ties

    bool operator<(const HeapItem& other) const {
        if (perishScore != other.perishScore)
            return perishScore < other.perishScore;   // max-heap by perishScore
        return arrivalTime > other.arrivalTime;       // earlier arrival first
    }
};

void demoPerishableHeap() {
    priority_queue<HeapItem> pq;

    // Sample loads: organic and other waste with different perishability.
    pq.push({1, 90,  5});   // highly perishable
    pq.push({2, 40,  2});   // less perishable
    pq.push({3, 90,  1});   // same perishScore as batch 1, but earlier
    pq.push({4, 10, 10});   // almost non-perishable

    cout << "Processing perishable loads in priority order:\n";
    while (!pq.empty()) {
        auto top = pq.top(); pq.pop();
        cout << "  Batch " << top.batchId
             << " perishScore=" << top.perishScore
             << " arrivalTime=" << top.arrivalTime << "\n";
    }
    cout << "\n";
}

/*
Heap comments (waste context):
- Uses a max-heap to always pick the most perishable / urgent batch first.
- Breaks ties by earlier arrival time to reduce spoilage and odor issues.
- Ideal for organic or time-sensitive loads that must not wait long in queues.
*/

// =================================================================
// QUEUE: conveyor / truck buffer
// =================================================================
void demoConveyorQueue() {
    queue<WasteBatch> conveyor;  // batches move in FIFO order

    conveyor.push({10, ORGANIC, 120.0, 1, 80});
    conveyor.push({11, PLASTIC,  80.0, 2, 10});
    conveyor.push({12, METAL,   200.0, 3,  5});

    cout << "Conveyor buffer (FIFO processing order):\n";
    while (!conveyor.empty()) {
        WasteBatch b = conveyor.front(); conveyor.pop();
        cout << "  Batch " << b.id
             << " type=" << b.type
             << " weight=" << b.weightKg << "kg\n";
    }
    cout << "\n";
}

/*
Queue comments (waste context):
- Models conveyor lines or truck unloading queues as FIFO buffers.
- New batches enter at the tail and are processed in arrival order.
- Simple, fair structure when no explicit prioritization is needed.
*/

// =================================================================
// SIMPLE SORTS: selection, insertion, bubble for waste classification
// =================================================================

// Sort by weight using Selection Sort
void selectionSortByWeight(vector<WasteBatch>& a) {
    int n = a.size();
    for (int i = 0; i < n; ++i) {
        int minIdx = i;
        for (int j = i + 1; j < n; ++j) {
            if (a[j].weightKg < a[minIdx].weightKg)
                minIdx = j;
        }
        if (minIdx != i) swap(a[i], a[minIdx]);
    }
}

// Sort by arrival time using Insertion Sort
void insertionSortByArrival(vector<WasteBatch>& a) {
    int n = a.size();
    for (int i = 1; i < n; ++i) {
        WasteBatch key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j].arrivalTime > key.arrivalTime) {
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = key;
    }
}

// Sort by type using Bubble Sort
void bubbleSortByType(vector<WasteBatch>& a) {
    int n = a.size();
    bool swapped;
    do {
        swapped = false;
        for (int i = 1; i < n; ++i) {
            if (a[i - 1].type > a[i].type) {
                swap(a[i - 1], a[i]);
                swapped = true;
            }
        }
    } while (swapped);
}

void demoSorting() {
    vector<WasteBatch> batches = {
        {20, METAL,   400.0, 6,  5},
        {21, ORGANIC,  50.0, 2, 90},
        {22, PLASTIC, 120.0, 1, 20},
        {23, PAPER,    80.0, 4, 15}
    };

    cout << "Original batches (id, type, weight, arrival):\n";
    for (auto& b : batches)
        cout << "  " << b.id << " t=" << b.type
             << " w=" << b.weightKg
             << " a=" << b.arrivalTime << "\n";
    cout << "\n";

    // Selection sort by weight
    auto byWeight = batches;
    selectionSortByWeight(byWeight);
    cout << "After Selection Sort by weightKg:\n";
    for (auto& b : byWeight)
        cout << "  " << b.id << " w=" << b.weightKg << "\n";
    cout << "\n";

    // Insertion sort by arrival time
    auto byArrival = batches;
    insertionSortByArrival(byArrival);
    cout << "After Insertion Sort by arrivalTime:\n";
    for (auto& b : byArrival)
        cout << "  " << b.id << " a=" << b.arrivalTime << "\n";
    cout << "\n";

    // Bubble sort by type
    auto byType = batches;
    bubbleSortByType(byType);
    cout << "After Bubble Sort by type (grouping similar waste types):\n";
    for (auto& b : byType)
        cout << "  " << b.id << " type=" << b.type << "\n";
    cout << "\n";
}

/*
Sorting comments (waste context):
- Selection sort by weight helps form batches of small vs large loads.
- Insertion sort by arrival time is easy to maintain as new batches arrive.
- Bubble sort by type can group similar materials before feeding specialized lines (plastic, metal, etc.).
*/

// =================================================================
// UNION-FIND: recyclable grouping
// =================================================================
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

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (rankVec[a] < rankVec[b]) swap(a, b);
        parent[b] = a;
        if (rankVec[a] == rankVec[b]) rankVec[a]++;
    }
};

void demoRecyclableGrouping() {
    // Example: 6 items, with compatibility relations.
    int n = 6;
    UnionFind uf(n);

    // Suppose items (0,1,2) are compatible plastic grades; (3,4) metals; (5) alone.
    uf.unite(0, 1);
    uf.unite(1, 2);
    uf.unite(3, 4);

    // Group items by their root.
    unordered_map<int, vector<int>> groups;
    for (int i = 0; i < n; ++i) {
        int root = uf.find(i);
        groups[root].push_back(i);
    }

    cout << "Recyclable compatibility groups (Union-Find):\n";
    for (auto& g : groups) {
        cout << "  Group root " << g.first << " -> items: ";
        for (int id : g.second) cout << id << " ";
        cout << "\n";
    }
    cout << "\n";
}

/*
Union-Find comments (waste context):
- Represents compatibility relations between recyclable items or material grades.
- Items that can be processed together are merged into the same component.
- Helps ensure mixed loads sent to a recycling line are actually compatible.
*/

// =================================================================
// DFS: detect process bottlenecks in a plant graph
// =================================================================
// Directed graph: nodes = processing stations, edges = material flow.
// DFS can be used to find strongly loaded paths, cycles, or unreachable stations.
// Here we just perform DFS and print traversal; in practice you might
// track in-degrees or visit counts to highlight bottleneck nodes.

void dfsUtil(int u, const vector<vector<int>>& graph, vector<bool>& visited) {
    visited[u] = true;
    cout << u << " ";
    for (int v : graph[u]) {
        if (!visited[v]) dfsUtil(v, graph, visited);
    }
}

void demoDFSProcessGraph() {
    int n = 5;
    // 0: input station, 1: primary sorter, 2: organic line, 3: recyclables, 4: landfill
    vector<vector<int>> g(n);
    g[0] = {1};
    g[1] = {2, 3};
    g[2] = {4}; // organics -> landfill/compost
    g[3] = {4}; // recyclables -> baling/landfill
    // 4 is sink

    vector<bool> visited(n, false);
    cout << "DFS over process graph from input station (0):\n  ";
    dfsUtil(0, g, visited);
    cout << "\n\n";
}

/*
DFS comments (waste context):
- Traverses processing stages from input to final destinations.
- Helps reveal unreachable stations (never visited) or long chains where delays may build up.
- Combined with node statistics (throughput, utilization) it can highlight bottleneck stages.
*/

// =================================================================
// Arrays / Structures: batch data store
// =================================================================
void demoBatchArray() {
    array<WasteBatch, 3> store = {{
        {100, ORGANIC,  60.0, 1, 80},
        {101, PLASTIC, 150.0, 2, 10},
        {102, METAL,   300.0, 3,  5}
    }};

    cout << "Stored batch data (array of structs):\n";
    for (const auto& b : store) {
        cout << "  Batch " << b.id
             << " type=" << b.type
             << " weight=" << b.weightKg
             << " arrival=" << b.arrivalTime << "\n";
    }
    cout << "\n";
}

/*
Arrays/Structures comments (waste context):
- Arrays/vectors of WasteBatch hold core operational data: id, type, weight, time, perishability.
- Simple struct layout makes it easy to pass batches between algorithms (heap, queue, sorts, etc.).
- Forms the central â€œrecordâ€ representation used across the plant control system.
*/

// =================================================================
// MAIN: run all demos
// =================================================================
int main() {
    cout << "=== Heap: Perishable load prioritization ===\n";
    demoPerishableHeap();

    cout << "=== Queue: Conveyor / truck buffer ===\n";
    demoConveyorQueue();

    cout << "=== Sorting: Classification and ordering ===\n";
    demoSorting();

    cout << "=== Union-Find: Recyclable grouping ===\n";
    demoRecyclableGrouping();

    cout << "=== DFS: Process bottleneck exploration ===\n";
    demoDFSProcessGraph();

    cout << "=== Arrays/Structures: Batch data store ===\n";
    demoBatchArray();

    return 0;
}
