#include <bits/stdc++.h>
using namespace std;

// -------------------- Slot details --------------------

enum SlotType { COMPACT = 0, LARGE = 1, HANDICAP = 2 };

struct Slot {
    int id;
    int level;
    double distance;
    SlotType type;
    bool isFree;
};

// ================================================================
// AVL Tree (ordered by slot.id)
// ================================================================

struct AVLNode {
    Slot slot;
    AVLNode *left, *right;
    int height;
    AVLNode(const Slot& s) : slot(s), left(nullptr), right(nullptr), height(1) {}
};

int h(AVLNode* n) { return n ? n->height : 0; }

int getBalance(AVLNode* n) {
    return n ? h(n->left) - h(n->right) : 0;
}

AVLNode* rightRotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(h(y->left), h(y->right)) + 1;
    x->height = max(h(x->left), h(x->right)) + 1;
    return x;
}

AVLNode* leftRotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(h(x->left), h(x->right)) + 1;
    y->height = max(h(y->left), h(y->right)) + 1;
    return y;
}

AVLNode* avlInsert(AVLNode* node, const Slot& s) {
    if (!node) return new AVLNode(s);

    if (s.id < node->slot.id)
        node->left = avlInsert(node->left, s);
    else if (s.id > node->slot.id)
        node->right = avlInsert(node->right, s);
    else
        return node;

    node->height = 1 + max(h(node->left), h(node->right));
    int balance = getBalance(node);

    if (balance > 1 && s.id < node->left->slot.id)
        return rightRotate(node);
    if (balance < -1 && s.id > node->right->slot.id)
        return leftRotate(node);
    if (balance > 1 && s.id > node->left->slot.id) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balance < -1 && s.id < node->right->slot.id) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    return node;
}

void inorderSlots(AVLNode* root) {
    if (!root) return;
    inorderSlots(root->left);
    cout << root->slot.id << " "
         << root->slot.level << " "
         << root->slot.distance << " "
         << root->slot.type << " "
         << root->slot.isFree << "\n";
    inorderSlots(root->right);
}

// ================================================================
// Skip List (free slots ordered by distance)
// ================================================================

struct SkipNode {
    double key;
    int slotId;
    vector<SkipNode*> forward;

    SkipNode(int level, double k, int id)
        : key(k), slotId(id), forward(level + 1, nullptr) {}
};

class SkipList {
    int maxLevel;
    float prob;
    int level;
    SkipNode* header;

public:
    SkipList(int maxL = 4, float p = 0.5)
        : maxLevel(maxL), prob(p), level(0) {
        header = new SkipNode(maxLevel, -1e9, -1);
    }

    int randomLevel() {
        int lvl = 0;
        while ((float)rand() / RAND_MAX < prob && lvl < maxLevel)
            lvl++;
        return lvl;
    }

    void insert(double key, int slotId) {
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

        SkipNode* newNode = new SkipNode(rlvl, key, slotId);
        for (int i = 0; i <= rlvl; i++) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
    }

    int getNearestFree() const {
        SkipNode* x = header->forward[0];
        return x ? x->slotId : -1;
    }

    void display() const {
        SkipNode* x = header->forward[0];
        while (x) {
            cout << x->slotId << " " << x->key << "\n";
            x = x->forward[0];
        }
    }
};

// ================================================================
// Brute Force String Search (BFSS)
// ================================================================

int bruteForceSearch(const vector<pair<string,int>>& data,
                     const string& key) {
    for (int i = 0; i < (int)data.size(); i++) {
        if (data[i].first == key)
            return data[i].second;
    }
    return -1;
}

// ================================================================
// MAIN
// ================================================================

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    srand((unsigned)time(nullptr));

    // ---------- Slots ----------
    int slotCount;
    cin >> slotCount;

    vector<Slot> slots(slotCount);
    for (int i = 0; i < slotCount; i++) {
        int typeInt, freeInt;
        cin >> slots[i].id
            >> slots[i].level
            >> slots[i].distance
            >> typeInt
            >> freeInt;
        slots[i].type = static_cast<SlotType>(typeInt);
        slots[i].isFree = freeInt;
    }

    // ---------- AVL Tree ----------
    AVLNode* root = nullptr;
    for (auto& s : slots)
        root = avlInsert(root, s);

    inorderSlots(root);

    // ---------- Skip List ----------
    SkipList freeSlots;
    for (auto& s : slots)
        if (s.isFree)
            freeSlots.insert(s.distance, s.id);

    freeSlots.display();
    cout << freeSlots.getNearestFree() << "\n";

    // ---------- Vehicle → Slot Mapping ----------
    int mappingCount;
    cin >> mappingCount;

    vector<pair<string,int>> vehicleSlotList;
    for (int i = 0; i < mappingCount; i++) {
        string plate;
        int slotId;
        cin >> plate >> slotId;
        vehicleSlotList.push_back({plate, slotId});
    }

    // ---------- BFSS Lookup ----------
    string queryPlate;
    cin >> queryPlate;

    int slotFound = bruteForceSearch(vehicleSlotList, queryPlate);
    if (slotFound != -1)
        cout << slotFound << "\n";
    else
        cout << "NOT FOUND\n";

    return 0;
}
