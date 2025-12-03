
// Smart Parking – Slot Allocation System Demo
// Purpose: Efficient slot assignment and reservation management.
//
// DS / Algorithms used:
// - BST/AVL (balanced BST): ordered slot data (by slotId, distance, etc.)
// - Skip List: fast ordered access/updates to free slots
// - Hashing (unordered_map): vehicle -> slot lookup
// - Structures: slot details (id, level, type, status)

#include <bits/stdc++.h>
using namespace std;

// -------------------- Slot details structure --------------------

enum SlotType { COMPACT, LARGE, HANDICAP };

struct Slot {
    int id;          // unique slot id
    int level;       // parking level
    double distance; // distance from entrance
    SlotType type;
    bool isFree;
};

/*
Slot struct comments (parking context):
- Encapsulates core information used by allocation logic: id, level, distance, type, and free/occupied state.
- Shared across tree, skip list, and hash map so all structures refer to consistent slot metadata.
*/

// ================================================================
// AVL Tree (BST) for ordered slot data (by id here)
// ================================================================

struct AVLNode {
    Slot slot;
    AVLNode *left, *right;
    int height;
    AVLNode(const Slot& s) : slot(s), left(nullptr), right(nullptr), height(1) {}
};

int h(AVLNode* n) { return n ? n->height : 0; }

int getBalance(AVLNode* n) { return n ? h(n->left) - h(n->right) : 0; }

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

// Insert slot ordered by slot.id
AVLNode* avlInsert(AVLNode* node, const Slot& s) {
    if (!node) return new AVLNode(s);
    if (s.id < node->slot.id) node->left = avlInsert(node->left, s);
    else if (s.id > node->slot.id) node->right = avlInsert(node->right, s);
    else return node; // ignore duplicate id

    node->height = 1 + max(h(node->left), h(node->right));
    int balance = getBalance(node);

    // LL
    if (balance > 1 && s.id < node->left->slot.id)
        return rightRotate(node);
    // RR
    if (balance < -1 && s.id > node->right->slot.id)
        return leftRotate(node);
    // LR
    if (balance > 1 && s.id > node->left->slot.id) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    // RL
    if (balance < -1 && s.id < node->right->slot.id) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    return node;
}

void inorderSlots(AVLNode* root) {
    if (!root) return;
    inorderSlots(root->left);
    cout << "  Slot " << root->slot.id
         << " level=" << root->slot.level
         << " dist=" << root->slot.distance
         << " free=" << (root->slot.isFree ? "Y" : "N") << "\n";
    inorderSlots(root->right);
}

/*
AVL/BST comments (parking context):
- Keeps all slots ordered by some key (id, distance, or (level, distance)).
- Balanced structure guarantees log-time insert/search to find specific slots or ranges.
- Useful for quickly scanning all slots in order, for monitoring or reporting.
*/

// ================================================================
// Simple Skip List for ordered free slots (by distance here)
// ================================================================

struct SkipNode {
    double key;   // distance or other ordering key
    int slotId;   // reference to slot.id
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
    SkipList(int maxL = 4, float p = 0.5) :
        maxLevel(maxL), prob(p), level(0) {
        header = new SkipNode(maxLevel, -1e9, -1);
    }

    int randomLevel() {
        int lvl = 0;
        while ((float)rand() / RAND_MAX < prob && lvl < maxLevel) lvl++;
        return lvl;
    }

    void insert(double key, int slotId) {
        vector<SkipNode*> update(maxLevel + 1);
        SkipNode* x = header;
        for (int i = level; i >= 0; --i) {
            while (x->forward[i] && x->forward[i]->key < key)
                x = x->forward[i];
            update[i] = x;
        }
        x = x->forward[0];

        if (!x || x->key != key || x->slotId != slotId) {
            int rlvl = randomLevel();
            if (rlvl > level) {
                for (int i = level + 1; i <= rlvl; ++i)
                    update[i] = header;
                level = rlvl;
            }
            SkipNode* newNode = new SkipNode(rlvl, key, slotId);
            for (int i = 0; i <= rlvl; ++i) {
                newNode->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = newNode;
            }
        }
    }

    // Remove a specific (key, slotId)
    void remove(double key, int slotId) {
        vector<SkipNode*> update(maxLevel + 1);
        SkipNode* x = header;
        for (int i = level; i >= 0; --i) {
            while (x->forward[i] &&
                   (x->forward[i]->key < key ||
                   (x->forward[i]->key == key && x->forward[i]->slotId < slotId)))
                x = x->forward[i];
            update[i] = x;
        }
        x = x->forward[0];
        if (x && x->key == key && x->slotId == slotId) {
            for (int i = 0; i <= level; ++i) {
                if (update[i]->forward[i] != x) break;
                update[i]->forward[i] = x->forward[i];
            }
            delete x;
            while (level > 0 && header->forward[level] == nullptr)
                level--;
        }
    }

    // Get best (nearest) free slot
    int getNearestFree() const {
        SkipNode* x = header->forward[0];
        return x ? x->slotId : -1;
    }

    void display() const {
        cout << "Skip list level 0 (free slots by distance):\n  ";
        SkipNode* x = header->forward[0];
        while (x) {
            cout << "(" << x->slotId << ",d=" << x->key << ") ";
            x = x->forward[0];
        }
        cout << "\n";
    }
};

/*
Skip List comments (parking context):
- Maintains free slots ordered by distance (or other keys) with fast insert/delete/search on average.
- Allocator can quickly pick the nearest free slot using the first node in level 0.
- More flexible to update frequently than some tree implementations, while giving similar complexities.
*/

// ================================================================
// Hashing: vehicle -> slot lookup
// ================================================================

void demoVehicleSlotMap() {
    unordered_map<string, int> vehicleToSlot; // plate -> slotId

    vehicleToSlot["KA01AB1234"] = 5;
    vehicleToSlot["KA02XY9999"] = 12;

    cout << "Vehicle -> slot lookup using hash map:\n";
    for (auto& p : vehicleToSlot) {
        cout << "  Vehicle " << p.first << " in slot " << p.second << "\n";
    }

    string query = "KA01AB1234";
    auto it = vehicleToSlot.find(query);
    if (it != vehicleToSlot.end()) {
        cout << "Lookup: " << query << " parked in slot " << it->second << "\n\n";
    } else {
        cout << "Lookup: " << query << " not currently parked\n\n";
    }
}

/*
Hashing comments (parking context):
- unordered_map maps each vehicle plate to its current slot in O(1) expected time.
- Used on entry (assign slot and record mapping) and exit (find slot quickly to free it).
- Also supports quick checks like “is this vehicle already parked somewhere?”.
*/

// ================================================================
// Demo wiring everything together
// ================================================================

int main() {
    srand((unsigned)time(nullptr));

    // Sample slots
    vector<Slot> slots = {
        {1,  0, 10.0, COMPACT, true},
        {2,  0,  8.0, COMPACT, true},
        {3,  1, 15.0, LARGE,   true},
        {4,  1, 20.0, HANDICAP,true},
        {5,  0,  5.0, COMPACT, true}
    };

    // Build AVL tree over all slots (ordered by slot id)
    AVLNode* root = nullptr;
    for (const auto& s : slots) root = avlInsert(root, s);

    cout << "=== AVL Tree: Ordered slot data (by id) ===\n";
    inorderSlots(root);
    cout << "\n";

    // Build skip list of only free slots ordered by distance
    SkipList freeSlots;
    for (const auto& s : slots)
        if (s.isFree) freeSlots.insert(s.distance, s.id);

    cout << "=== Skip List: Free slots by distance ===\n";
    freeSlots.display();
    int nearest = freeSlots.getNearestFree();
    cout << "Nearest free slot chosen for new vehicle: " << nearest << "\n\n";

    // Vehicle -> slot mapping demo
    cout << "=== Hash Map: Vehicle -> Slot ===\n";
    demoVehicleSlotMap();

    return 0;
}
