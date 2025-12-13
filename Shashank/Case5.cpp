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
// TRIE (Exact plate lookup)
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
// Rabin–Karp (String pattern verification)
// ------------------------------------------------------------

bool rabinKarpSearch(const string& text, const string& pattern) {
    const int d = 256;
    const int q = 101;

    int n = text.size();
    int m = pattern.size();
    if (m > n) return false;

    int h = 1;
    for (int i = 0; i < m - 1; i++)
        h = (h * d) % q;

    int p = 0, t = 0;
    for (int i = 0; i < m; i++) {
        p = (d * p + pattern[i]) % q;
        t = (d * t + text[i]) % q;
    }

    for (int i = 0; i <= n - m; i++) {
        if (p == t) {
            bool match = true;
            for (int j = 0; j < m; j++) {
                if (text[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) return true;
        }
        if (i < n - m) {
            t = (d * (t - text[i] * h) + text[i + m]) % q;
            if (t < 0) t += q;
        }
    }
    return false;
}

// ------------------------------------------------------------
// AVL TREE (Toll slab optimization)
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
    cout << root->slab.id << " "
         << root->slab.vehicleType << " "
         << root->slab.rate << "\n";
    inorderAVL(root->right);
}

// ------------------------------------------------------------
// SKIP LIST (Rate-based fast lookup)
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
    SkipList(int maxL = 4, float p = 0.5)
        : maxLevel(maxL), prob(p), level(0) {
        TollSlab dummy{-1,"",0,0,0};
        header = new SkipNode(maxLevel, -1e9, dummy);
    }

    int randomLevel() {
        int lvl = 0;
        while ((float)rand() / RAND_MAX < prob && lvl < maxLevel)
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

    int vehicleCount;
    cin >> vehicleCount;
    vector<Vehicle> vehicles(vehicleCount);
    for (int i = 0; i < vehicleCount; i++)
        cin >> vehicles[i].plate >> vehicles[i].owner >> vehicles[i].vehicleType;

    PlateTrie trie;
    for (int i = 0; i < vehicleCount; i++)
        trie.insert(vehicles[i].plate, i);

    int accountCount;
    cin >> accountCount;
    unordered_map<string, AccountInfo> accounts;
    for (int i = 0; i < accountCount; i++) {
        string plate;
        AccountInfo a;
        cin >> plate >> a.balance >> a.stolenFlag;
        accounts[plate] = a;
    }

    int slabCount;
    cin >> slabCount;
    vector<TollSlab> slabs(slabCount);
    for (int i = 0; i < slabCount; i++)
        cin >> slabs[i].id >> slabs[i].vehicleType
            >> slabs[i].minWeight >> slabs[i].maxWeight >> slabs[i].rate;

    AVLNode* root = nullptr;
    SkipList sk;
    for (auto& s : slabs) {
        root = avlInsert(root, s);
        sk.insert(s.rate, s);
    }

    inorderAVL(root);
    sk.display();

    string queryPlate;
    cin >> queryPlate;

    int idx = trie.search(queryPlate);
    bool rkMatch = false;
    for (auto& v : vehicles)
        if (rabinKarpSearch(v.plate, queryPlate))
            rkMatch = true;

    if (idx != -1 && rkMatch && accounts.count(queryPlate)) {
        cout << vehicles[idx].plate << " "
             << vehicles[idx].owner << " "
             << accounts[queryPlate].balance << " "
             << accounts[queryPlate].stolenFlag << "\n";
    } else {
        cout << "NOT FOUND\n";
    }

    return 0;
}
