#include <bits/stdc++.h>
using namespace std;

// =============== DATA MODELS ==================
struct Supplier {
    int id;
    string name;
    string category;
    double price;       // quoted base price
    double rating;      // 0–5
};

struct Buyer {
    int id;
    string name;
    string categoryNeed;
    double maxBudget;
};

struct Bid {
    int supplierId;
    int buyerId;
    double score;   // higher is better
};

// max-heap on score
struct BidCmp {
    bool operator()(const Bid &a, const Bid &b) const {
        return a.score < b.score;
    }
};

// =============== GLOBAL STATE =================
unordered_map<int, Supplier> suppliers;     // hash table of suppliers
unordered_map<int, Buyer> buyers;          // hash table of buyers
priority_queue<Bid, vector<Bid>, BidCmp> bidHeap;  // global bid queue

// =============== UTILS ========================

// Simple scoring: higher rating and lower price relative to budget
double computeScore(const Supplier &s, const Buyer &b) {
    if (s.category != b.categoryNeed) return -1e9; // incompatible
    if (s.price > b.maxBudget) return -1e9;        // too expensive

    double priceFactor  = 1.0 - (s.price / b.maxBudget); // 0..1
    double ratingFactor = s.rating / 5.0;                 // 0..1
    return 0.6 * ratingFactor + 0.4 * priceFactor;
}

// =============== INPUT OPS =====================
void addSupplier() {
    Supplier s;
    cout << "Enter supplier id: ";
    cin >> s.id;
    cin.ignore();
    cout << "Enter supplier name: ";
    getline(cin, s.name);
    cout << "Enter category (e.g., steel, cement, IT-hardware): ";
    getline(cin, s.category);
    cout << "Enter quoted price: ";
    cin >> s.price;
    cout << "Enter rating (0-5): ";
    cin >> s.rating;

    suppliers[s.id] = s;
    cout << "Supplier added.\n";
}

void addBuyer() {
    Buyer b;
    cout << "Enter buyer id: ";
    cin >> b.id;
    cin.ignore();
    cout << "Enter buyer name: ";
    getline(cin, b.name);
    cout << "Enter required category: ";
    getline(cin, b.categoryNeed);
    cout << "Enter max budget: ";
    cin >> b.maxBudget;

    buyers[b.id] = b;
    cout << "Buyer added.\n";
}

// =============== SORTING (QUICK via std::sort) ==============
void rankSuppliersByRating() {
    if (suppliers.empty()) {
        cout << "No suppliers.\n";
        return;
    }
    vector<Supplier> v;
    for (auto &p : suppliers) v.push_back(p.second);

    // QuickSort-style: std::sort with comparator (rating desc, price asc)
    sort(v.begin(), v.end(), [](const Supplier &a, const Supplier &b) {
        if (a.rating != b.rating) return a.rating > b.rating;
        return a.price < b.price;
    });

    cout << "Suppliers ranked by rating (QuickSort/std::sort):\n";
    for (auto &s : v) {
        cout << "ID " << s.id << " | " << s.name
             << " | Cat: " << s.category
             << " | Rating: " << s.rating
             << " | Price: " << s.price << "\n";
    }
}

// =============== MERGE SORT (stable) =========================
void merge(vector<Supplier> &arr, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;
    vector<Supplier> L(n1), R(n2);
    for (int i = 0; i < n1; ++i) L[i] = arr[l + i];
    for (int j = 0; j < n2; ++j) R[j] = arr[m + 1 + j];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        // stable: sort by price asc, then rating desc
        if (L[i].price < R[j].price ||
           (L[i].price == R[j].price && L[i].rating >= R[j].rating)) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSort(vector<Supplier> &arr, int l, int r) {
    if (l >= r) return;
    int m = (l + r) / 2;
    mergeSort(arr, l, m);
    mergeSort(arr, m + 1, r);
    merge(arr, l, m, r);
}

void stableRankSuppliersByPrice() {
    if (suppliers.empty()) {
        cout << "No suppliers.\n";
        return;
    }
    vector<Supplier> v;
    for (auto &p : suppliers) v.push_back(p.second);

    mergeSort(v, 0, (int)v.size() - 1);

    cout << "Suppliers stably ranked by price (Merge Sort):\n";
    for (auto &s : v) {
        cout << "ID " << s.id << " | " << s.name
             << " | Cat: " << s.category
             << " | Price: " << s.price
             << " | Rating: " << s.rating << "\n";
    }
}

// =============== BIPARTITE MATCHING (GREEDY) ==================
// For simplicity: each buyer required 1 supplier; each supplier can serve 1 buyer.
// We will build all compatible Bid scores, push them in a max-heap,
// then greedily pop best remaining bid where both sides are still unmatched.
void buildBids() {
    while (!bidHeap.empty()) bidHeap.pop();

    for (auto &bp : buyers) {
        const Buyer &b = bp.second;
        for (auto &sp : suppliers) {
            const Supplier &s = sp.second;
            double score = computeScore(s, b);
            if (score > 0) {
                bidHeap.push({s.id, b.id, score});
            }
        }
    }
    cout << "Bid heap built with compatible supplier-buyer pairs.\n";
}

void runMatching() {
    if (buyers.empty() || suppliers.empty()) {
        cout << "Need at least one buyer and one supplier.\n";
        return;
    }

    buildBids();

    unordered_map<int, int> buyerMatch;   // buyerId -> supplierId
    unordered_map<int, int> supplierMatch;// supplierId -> buyerId

    while (!bidHeap.empty()) {
        Bid b = bidHeap.top(); bidHeap.pop();
        if (buyerMatch.count(b.buyerId) || supplierMatch.count(b.supplierId))
            continue; // one of them already matched

        buyerMatch[b.buyerId] = b.supplierId;
        supplierMatch[b.supplierId] = b.buyerId;
    }

    cout << "Matching result (greedy bipartite using heap):\n";
    for (auto &bp : buyers) {
        int bid = bp.first;
        const Buyer &b = bp.second;
        cout << "Buyer " << b.id << " (" << b.name << "): ";
        if (!buyerMatch.count(bid)) {
            cout << "no supplier allocated.\n";
        } else {
            int sid = buyerMatch[bid];
            const Supplier &s = suppliers[sid];
            cout << "Supplier " << s.id << " (" << s.name
                 << "), Category: " << s.category
                 << ", Price: " << s.price
                 << ", Rating: " << s.rating << "\n";
        }
    }
}

// =============== MENU / MAIN ==============================
void printMenu() {
    cout << "\n=== Supplier–Buyer Allocation Platform (B2B Matching) ===\n";
    cout << "1. Add supplier profile\n";
    cout << "2. Add buyer requirement\n";
    cout << "3. Rank suppliers by rating (QuickSort/std::sort)\n";
    cout << "4. Stable rank suppliers by price (Merge Sort)\n";
    cout << "5. Run supplier–buyer matching (heap + greedy bipartite)\n";
    cout << "0. Exit\n";
    cout << "Choice: ";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int choice;
    do {
        printMenu();
        cin >> choice;
        switch (choice) {
            case 1: addSupplier(); break;
            case 2: addBuyer(); break;
            case 3: rankSuppliersByRating(); break;
            case 4: stableRankSuppliersByPrice(); break;
            case 5: runMatching(); break;
            case 0: cout << "Exiting.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}
