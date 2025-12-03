
// Smart Toll Collection & Vehicle Lookup Demo
// Purpose: Automatic toll deduction and instant vehicle verification.
//
// DS / Algorithms used:
// - Trie: license plate lookup (prefix-based search)
// - Hashing (unordered_map): vehicle -> balance / info map
// - AVL-style balanced BST (simplified): ordered toll slabs
// - Skip List (simplified): fast ordered access to toll slabs
// - Arrays/Structures: store vehicle & toll data

#include <bits/stdc++.h>
using namespace std;

// ------------------------------------------------------------
// Basic structures: vehicle & toll slab
// ------------------------------------------------------------

struct Vehicle {
    string plate;     // license plate
    string owner;
    string vehicleType; // "CAR", "TRUCK", etc.
};

struct TollSlab {
    int id;
    string vehicleType; // which type this slab applies to
    double minWeight;
    double maxWeight;
    double rate;       // toll rate
};

// ------------------------------------------------------------
// TRIE: license plate lookup
// ------------------------------------------------------------

struct TrieNode {
    bool isEnd;
    int vehicleIndex;                    // index into vehicles array, -1 if none
    unordered_map<char, TrieNode*> nxt;  // next character nodes

    TrieNode() : isEnd(false), vehicleIndex(-1) {}
};

class PlateTrie {
public:
    TrieNode* root;

    PlateTrie() { root = new TrieNode(); }

    void insert(const string& plate, int index) {
        TrieNode* cur = root;
        for (char c : plate) {
            if (!cur->nxt.count(c)) cur->nxt[c] = new TrieNode();
            cur = cur->nxt[c];
        }
        cur->isEnd = true;
        cur->vehicleIndex = index;
    }

    // Exact match lookup by plate string
    int search(const string& plate) const {
        TrieNode* cur = root;
        for (char c : plate) {
            auto it = cur->nxt.find(c);
            if (it == cur->nxt.end()) return -1;
            cur = it->second;
        }
        return cur->isEnd ? cur->vehicleIndex : -1;
    }

    // Optional: prefix search (e.g., for partial plate matches / fraud checks)
    bool hasPrefix(const string& pref) const {
        TrieNode* cur = root;
        for (char c : pref) {
            auto it = cur->nxt.find(c);
            if (it == cur->nxt.end()) return false;
            cur = it->second;
        }
        return true;
    }
};

/*
Trie comments (toll context):
- Stores license plates character by character for fast exact or prefix lookup.
- Given an OCR-recognized plate string, search returns the index of the vehicle record.
- Helps quickly match millions of plates to accounts at highway-speed toll booths.
*/

// ------------------------------------------------------------
// HASHING: vehicle balance and info map
// ------------------------------------------------------------

struct AccountInfo {
    double balance;
    bool stolenFlag;
};

void demoHashBalances(const vector<Vehicle>& vehicles) {
    unordered_map<string, AccountInfo> accountMap; // key = plate

    // Sample balances
    accountMap["KA01AB1234"] = {500.0, false};
    accountMap["KA02XY9999"] = { 50.0, false};
    accountMap["DL10ZZ0001"] = {  0.0, true}; // stolen or flagged

    cout << "Checking balances and flags using hash map:\n";
    for (const auto& v : vehicles) {
        auto it = accountMap.find(v.plate);
        if (it != accountMap.end()) {
            cout << "  " << v.plate
                 << " balance=" << it->second.balance
                 << " stolen=" << (it->second.stolenFlag ? "YES" : "NO") << "\n";
        } else {
            cout << "  " << v.plate << " not found in account map\n";
        }
    }
    cout << "\n";
}

/*
Hashing comments (toll context):
- unordered_map provides O(1)-average lookup by license plate string.
- Stores balances, flags (stolen, blacklisted), and other account attributes.
- Used after plate recognition to quickly verify account status and deduct toll.
*/

// ------------------------------------------------------------
// AVL-like BST for ordered toll slabs (simplified)
// (For brevity, only insertion & in-order traversal are shown.)
// ------------------------------------------------------------

struct AVLNode {
    TollSlab slab;
    AVLNode *left, *right;
    int height;
    AVLNode(const TollSlab& s) : slab(s), left(nullptr), right(nullptr), height(1) {}
};

int height(AVLNode* n) { return n ? n->height : 0; }

int getBalance(AVLNode* n) { return n ? height(n->left) - height(n->right) : 0; }

AVLNode* rightRotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x;
}

AVLNode* leftRotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;
}

// Use slab.id as key just for demonstration (could be based on rate, weight, etc.)
AVLNode* avlInsert(AVLNode* node, const TollSlab& key) {
    if (!node) return new AVLNode(key);
    if (key.id < node->slab.id) node->left  = avlInsert(node->left, key);
    else if (key.id > node->slab.id) node->right = avlInsert(node->right, key);
    else return node; // duplicate IDs ignored

    node->height = 1 + max(height(node->left), height(node->right));

    int balance = getBalance(node);

    // Left Left
    if (balance > 1 && key.id < node->left->slab.id)
        return rightRotate(node);
    // Right Right
    if (balance < -1 && key.id > node->right->slab.id)
        return leftRotate(node);
    // Left Right
    if (balance > 1 && key.id > node->left->slab.id) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    // Right Left
    if (balance < -1 && key.id < node->right->slab.id) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    return node;
}

void inorderAVL(AVLNode* root) {
    if (!root) return;
    inorderAVL(root->left);
    cout << "  Slab " << root->slab.id
         << " type=" << root->slab.vehicleType
         << " rate=" << root->slab.rate << "\n";
    inorderAVL(root->right);
}

/*
AVL-tree comments (toll context):
- Maintains toll slabs in sorted order (e.g., by ID, rate, or limit).
- Self-balancing ensures O(log n) insertion and lookup even with many slabs.
- Used for selecting correct slab when considering ordered rules (e.g., ranges by weight or time).
*/

// ------------------------------------------------------------
// Simple Skip List for ordered toll rates (very minimal demo)
// ------------------------------------------------------------

struct SkipNode {
    double key;               // here: toll rate or some ordered key
    TollSlab slab;
    vector<SkipNode*> forward;

    SkipNode(int level, double k, const TollSlab& s)
        : key(k), slab(s), forward(level + 1, nullptr) {}
};

class SkipList {
    int maxLevel;
    float prob;
    int level;
    SkipNode* header;

public:
    SkipList(int maxL = 4, float p = 0.5) :
        maxLevel(maxL), prob(p), level(0) {
        TollSlab dummy{ -1, "", 0, 0, 0.0 };
        header = new SkipNode(maxLevel, -1e9, dummy);
    }

    int randomLevel() {
        int lvl = 0;
        while ((float)rand() / RAND_MAX < prob && lvl < maxLevel) lvl++;
        return lvl;
    }

    void insert(double key, const TollSlab& s) {
        vector<SkipNode*> update(maxLevel + 1);
        SkipNode* x = header;
        for (int i = level; i >= 0; --i) {
            while (x->forward[i] && x->forward[i]->key < key) {
                x = x->forward[i];
            }
            update[i] = x;
        }
        x = x->forward[0];

        if (!x || x->key != key) {
            int rlvl = randomLevel();
            if (rlvl > level) {
                for (int i = level + 1; i <= rlvl; ++i)
                    update[i] = header;
                level = rlvl;
            }
            SkipNode* newNode = new SkipNode(rlvl, key, s);
            for (int i = 0; i <= rlvl; ++i) {
                newNode->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = newNode;
            }
        }
    }

    void display() const {
        cout << "Skip list (level 0 view, ordered by key):\n  ";
        SkipNode* x = header->forward[0];
        while (x) {
            cout << "(" << x->key << "," << x->slab.id << ") ";
            x = x->forward[0];
        }
        cout << "\n";
    }
};

/*
Skip List comments (toll context):
- Provides ordered access similar to a balanced BST, but implemented using multiple â€œlevelsâ€ of linked lists.
- Average O(log n) search/insert, good for large or dynamically changing toll rules.
- Can be keyed by toll rate or threshold to quickly find applicable slab for a given transaction.
*/

// ------------------------------------------------------------
// Arrays / Structures: main vehicle & toll data store
// ------------------------------------------------------------

void demoArraysAndStructures() {
    // Fixed array of vehicles (could be a DB table in real systems).
    vector<Vehicle> vehicles = {
        {"KA01AB1234", "Alice", "CAR"},
        {"KA02XY9999", "Bob",   "TRUCK"},
        {"DL10ZZ0001", "Eve",   "CAR"}
    };

    // Array of toll slabs
    vector<TollSlab> slabs = {
        {1, "CAR",   0.0,  2.5,  30.0},
        {2, "CAR",   2.5,  7.5,  50.0},
        {3, "TRUCK", 0.0, 20.0, 120.0}
    };

    cout << "Vehicles array:\n";
    for (const auto& v : vehicles) {
        cout << "  " << v.plate << " owner=" << v.owner
             << " type=" << v.vehicleType << "\n";
    }
    cout << "\nToll slabs array:\n";
    for (const auto& s : slabs) {
        cout << "  Slab " << s.id << " type=" << s.vehicleType
             << " rate=" << s.rate << "\n";
    }
    cout << "\n";

    // Build trie for plates
    PlateTrie trie;
    for (int i = 0; i < (int)vehicles.size(); ++i)
        trie.insert(vehicles[i].plate, i);

    // Build AVL tree and Skip list for slabs
    AVLNode* avlRoot = nullptr;
    SkipList sk;

    for (const auto& s : slabs) {
        avlRoot = avlInsert(avlRoot, s);
        sk.insert(s.rate, s);
    }

    cout << "In-order traversal of AVL (ordered slabs):\n";
    inorderAVL(avlRoot);
    cout << "\n";
    sk.display();
    cout << "\n";

    // Hash-based balance demo
    demoHashBalances(vehicles);

    // Quick example: toll deduction for a passing plate.
    string seenPlate = "KA01AB1234";
    int idx = trie.search(seenPlate);
    if (idx != -1) {
        cout << "Plate " << seenPlate << " matched vehicle index " << idx
             << " (owner " << vehicles[idx].owner << ")\n";
    } else {
        cout << "Plate " << seenPlate << " not recognized in trie.\n";
    }
    cout << "\n";
}

/*
Arrays/Structures comments (toll context):
- Vectors / arrays hold static or cached data: vehicles, slabs, and relationships.
- Structs provide a clean schema shared across trie, hash map, trees, and skip list.
- Form the backbone of the toll systemâ€™s in-memory representation.
*/

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------

int main() {
    srand((unsigned)time(nullptr)); // for skip list random levels

    cout << "=== Smart Toll Collection & Vehicle Lookup Demo ===\n\n";
    demoArraysAndStructures();

    return 0;
}
