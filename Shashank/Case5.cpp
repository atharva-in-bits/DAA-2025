#include <bits/stdc++.h>
using namespace std;

// ------------------------------------------------------------
// Basic structures
// ------------------------------------------------------------

struct Vehicle {
    string plate;
    string owner;
    string vehicleType;
};

struct TollSlab {
    int id;
    string vehicleType;
    double minWeight;
    double maxWeight;
    double rate;
};

struct AccountInfo {
    double balance;
    bool stolenFlag;
};

// ------------------------------------------------------------
// TRIE
// ------------------------------------------------------------

struct TrieNode {
    bool isEnd;
    int vehicleIndex;
    unordered_map<char, TrieNode*> nxt;

    TrieNode() : isEnd(false), vehicleIndex(-1) {}
};

class PlateTrie {
public:
    TrieNode* root;

    PlateTrie() { root = new TrieNode(); }

    void insert(const string& plate, int index) {
        TrieNode* cur = root;
        for (char c : plate) {
            if (!cur->nxt.count(c))
                cur->nxt[c] = new TrieNode();
            cur = cur->nxt[c];
        }
        cur->isEnd = true;
        cur->vehicleIndex = index;
    }

    int search(const string& plate) const {
        TrieNode* cur = root;
        for (char c : plate) {
            auto it = cur->nxt.find(c);
            if (it == cur->nxt.end()) return -1;
            cur = it->second;
        }
        return cur->isEnd ? cur->vehicleIndex : -1;
    }
};

// ------------------------------------------------------------
// AVL TREE (simplified)
// ------------------------------------------------------------

struct AVLNode {
    TollSlab slab;
    AVLNode *left, *right;
    int height;
    AVLNode(const TollSlab& s) : slab(s), left(nullptr), right(nullptr), height(1) {}
};

int height(AVLNode* n) { return n ? n->height : 0; }

int getBalance(AVLNode* n) {
    return n ? height(n->left) - height(n->right) : 0;
}

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

AVLNode* avlInsert(AVLNode* node, const TollSlab& key) {
    if (!node) return new AVLNode(key);

    if (key.id < node->slab.id)
        node->left = avlInsert(node->left, key);
    else if (key.id > node->slab.id)
        node->right = avlInsert(node->right, key);
    else
        return node;

    node->height = 1 + max(height(node->left), height(node->right));
    int balance = getBalance(node);

    if (balance > 1 && key.id < node->left->slab.id)
        return rightRotate(node);
    if (balance < -1 && key.id > node->right->slab.id)
        return leftRotate(node);
    if (balance > 1 && key.id > node->left->slab.id) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balance < -1 && key.id < node->right->slab.id) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    return node;
}

void inorderAVL(AVLNode* root) {
    if (!root) return;
    inorderAVL(root->left);
    cout << root->slab.id << " " << root->slab.vehicleType
         << " " << root->slab.rate << "\n";
    inorderAVL(root->right);
}

// ------------------------------------------------------------
// SKIP LIST (simplified)
// ------------------------------------------------------------

struct SkipNode {
    double key;
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
        TollSlab dummy{-1,"",0,0,0};
        header = new SkipNode(maxLevel, -1e9, dummy);
    }

    int randomLevel() {
        int lvl = 0;
        while ((float)rand()/RAND_MAX < prob && lvl < maxLevel)
            lvl++;
        return lvl;
    }

    void insert(double key, const TollSlab& s) {
        vector<SkipNode*> update(maxLevel + 1);
        SkipNode* x = header;

        for (int i = level; i >= 0; i--) {
            while (x->forward[i] && x->forward[i]->key < key)
                x = x->forward[i];
            update[i] = x;
        }

        x = x->forward[0];
        if (!x || x->key != key) {
            int rlvl = randomLevel();
            if (rlvl > level) {
                for (int i = level + 1; i <= rlvl; i++)
                    update[i] = header;
                level = rlvl;
            }

            SkipNode* newNode = new SkipNode(rlvl, key, s);
            for (int i = 0; i <= rlvl; i++) {
                newNode->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = newNode;
            }
        }
    }

    void display() const {
        SkipNode* x = header->forward[0];
        while (x) {
            cout << x->key << " " << x->slab.id << "\n";
            x = x->forward[0];
        }
    }
};

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    srand((unsigned)time(nullptr));

    // -------- Vehicles --------
    int vehicleCount;
    cin >> vehicleCount;
    vector<Vehicle> vehicles(vehicleCount);
    for (int i = 0; i < vehicleCount; i++) {
        cin >> vehicles[i].plate
            >> vehicles[i].owner
            >> vehicles[i].vehicleType;
    }

    // -------- Trie --------
    PlateTrie trie;
    for (int i = 0; i < vehicleCount; i++)
        trie.insert(vehicles[i].plate, i);

    // -------- Account Map --------
    int accountCount;
    cin >> accountCount;
    unordered_map<string, AccountInfo> accounts;
    for (int i = 0; i < accountCount; i++) {
        string plate;
        AccountInfo a;
        cin >> plate >> a.balance >> a.stolenFlag;
        accounts[plate] = a;
    }

    // -------- Toll Slabs --------
    int slabCount;
    cin >> slabCount;
    vector<TollSlab> slabs(slabCount);
    for (int i = 0; i < slabCount; i++) {
        cin >> slabs[i].id
            >> slabs[i].vehicleType
            >> slabs[i].minWeight
            >> slabs[i].maxWeight
            >> slabs[i].rate;
    }

    // -------- AVL & SkipList --------
    AVLNode* avlRoot = nullptr;
    SkipList sk;
    for (auto& s : slabs) {
        avlRoot = avlInsert(avlRoot, s);
        sk.insert(s.rate, s);
    }

    inorderAVL(avlRoot);
    sk.display();

    // -------- Lookup Plate --------
    string queryPlate;
    cin >> queryPlate;
    int idx = trie.search(queryPlate);

    if (idx != -1 && accounts.count(queryPlate)) {
        cout << vehicles[idx].plate << " "
             << vehicles[idx].owner << " "
             << accounts[queryPlate].balance << " "
             << accounts[queryPlate].stolenFlag << "\n";
    } else {
        cout << "NOT FOUND\n";
    }

    return 0;
}

