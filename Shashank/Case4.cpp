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
    int arrivalTime;
    int perishScore;
};

struct HeapItem {
    int batchId;
    int perishScore;
    int arrivalTime;

    bool operator<(const HeapItem& other) const {
        if (perishScore != other.perishScore)
            return perishScore < other.perishScore;
        return arrivalTime > other.arrivalTime;
    }
};

// -------------------- Heap --------------------

void processPerishableHeap(priority_queue<HeapItem>& pq) {
    while (!pq.empty()) {
        auto h = pq.top(); pq.pop();
        cout << h.batchId << " " << h.perishScore << " " << h.arrivalTime << "\n";
    }
}

// -------------------- Queue --------------------

void processConveyorQueue(queue<WasteBatch>& q) {
    while (!q.empty()) {
        auto b = q.front(); q.pop();
        cout << b.id << " " << b.type << " "
             << b.weightKg << " " << b.arrivalTime
             << " " << b.perishScore << "\n";
    }
}

// -------------------- Sorting Algorithms --------------------

void selectionSortByWeight(vector<WasteBatch>& a) {
    int n = a.size();
    for (int i = 0; i < n; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++)
            if (a[j].weightKg < a[minIdx].weightKg)
                minIdx = j;
        swap(a[i], a[minIdx]);
    }
}

void insertionSortByArrival(vector<WasteBatch>& a) {
    for (int i = 1; i < (int)a.size(); i++) {
        WasteBatch key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j].arrivalTime > key.arrivalTime) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

void bubbleSortByType(vector<WasteBatch>& a) {
    int n = a.size();
    bool swapped;
    do {
        swapped = false;
        for (int i = 1; i < n; i++) {
            if (a[i - 1].type > a[i].type) {
                swap(a[i - 1], a[i]);
                swapped = true;
            }
        }
    } while (swapped);
}

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

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (rankVec[a] < rankVec[b]) swap(a, b);
        parent[b] = a;
        if (rankVec[a] == rankVec[b]) rankVec[a]++;
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

    // ---------- Heap Input ----------
    int heapCount;
    cin >> heapCount;
    priority_queue<HeapItem> heapPQ;
    for (int i = 0; i < heapCount; i++) {
        HeapItem h;
        cin >> h.batchId >> h.perishScore >> h.arrivalTime;
        heapPQ.push(h);
    }
    processPerishableHeap(heapPQ);

    // ---------- Queue Input ----------
    int queueCount;
    cin >> queueCount;
    queue<WasteBatch> conveyor;
    for (int i = 0; i < queueCount; i++) {
        WasteBatch b;
        int typeInt;
        cin >> b.id >> typeInt >> b.weightKg >> b.arrivalTime >> b.perishScore;
        b.type = static_cast<WasteType>(typeInt);
        conveyor.push(b);
    }
    processConveyorQueue(conveyor);

    // ---------- Sorting Input ----------
    int batchCount;
    cin >> batchCount;
    vector<WasteBatch> batches(batchCount);
    for (int i = 0; i < batchCount; i++) {
        int typeInt;
        cin >> batches[i].id >> typeInt
            >> batches[i].weightKg
            >> batches[i].arrivalTime
            >> batches[i].perishScore;
        batches[i].type = static_cast<WasteType>(typeInt);
    }

    selectionSortByWeight(batches);
    for (auto& b : batches) cout << b.id << " " << b.weightKg << "\n";

    insertionSortByArrival(batches);
    for (auto& b : batches) cout << b.id << " " << b.arrivalTime << "\n";

    bubbleSortByType(batches);
    for (auto& b : batches) cout << b.id << " " << b.type << "\n";

    // ---------- Union-Find Input ----------
    int ufSize, ufOps;
    cin >> ufSize >> ufOps;
    UnionFind uf(ufSize);
    for (int i = 0; i < ufOps; i++) {
        int a, b;
        cin >> a >> b;
        uf.unite(a, b);
    }

    unordered_map<int, vector<int>> groups;
    for (int i = 0; i < ufSize; i++)
        groups[uf.find(i)].push_back(i);

    for (auto& g : groups) {
        for (int id : g.second) cout << id << " ";
        cout << "\n";
    }

    // ---------- DFS Graph Input ----------
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

    // ---------- Array Store ----------
    int arraySize;
    cin >> arraySize;
    vector<WasteBatch> store(arraySize);
    for (int i = 0; i < arraySize; i++) {
        int typeInt;
        cin >> store[i].id >> typeInt
            >> store[i].weightKg
            >> store[i].arrivalTime
            >> store[i].perishScore;
        store[i].type = static_cast<WasteType>(typeInt);
    }

    for (auto& b : store) {
        cout << b.id << " " << b.type << " "
             << b.weightKg << " "
             << b.arrivalTime << " "
             << b.perishScore << "\n";
    }

    return 0;
}
