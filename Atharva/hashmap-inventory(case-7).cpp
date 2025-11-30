#include <bits/stdc++.h>
using namespace std;

// ============================================================================
// Product and Warehouse Structures
// ============================================================================

struct Product {
    int id;
    string name;
    string category;
    int quantity;
};

struct Warehouse {
    int id;
    string name;
    string zone;
};

// ============================================================================
// Custom Hash Table (Open Addressing with Linear Probing)
// Key: productId
// Value: warehouseId
// ============================================================================

class HashMap {
public:
    struct Entry {
        int key;
        int value;
        bool used;
        bool deleted;
        Entry() : key(0), value(0), used(false), deleted(false) {}
    };

    HashMap(int cap = 128) {
        capacity = cap;
        table.assign(cap, Entry());
        usedCount = 0;
    }

    void put(int key, int value) {
        if ((double)usedCount / capacity > 0.6) {
            rehash(capacity * 2);
        }
        insertInternal(key, value);
    }

    bool get(int key, int &outValue) {
        int idx = findIndex(key);
        if (idx == -1) return false;
        outValue = table[idx].value;
        return true;
    }

    void removeKey(int key) {
        int idx = findIndex(key);
        if (idx != -1) table[idx].deleted = true;
    }

    bool contains(int key) {
        return findIndex(key) != -1;
    }

    vector<pair<int,int>> allPairs() const {
        vector<pair<int,int>> out;
        for (auto &e : table) {
            if (e.used && !e.deleted) out.push_back({e.key, e.value});
        }
        return out;
    }

private:
    int capacity;
    int usedCount;
    vector<Entry> table;

    int hashFunc(int key) const {
        key ^= (key << 5);
        key ^= (key >> 7);
        return abs(key) % capacity;
    }

    void insertInternal(int key, int value) {
        int idx = hashFunc(key);
        for (int i = 0; i < capacity; i++) {
            int p = (idx + i) % capacity;
            if (!table[p].used || table[p].deleted) {
                table[p].key = key;
                table[p].value = value;
                table[p].used = true;
                table[p].deleted = false;
                usedCount++;
                return;
            }
        }
    }

    int findIndex(int key) {
        int idx = hashFunc(key);
        for (int i = 0; i < capacity; i++) {
            int p = (idx + i) % capacity;
            if (!table[p].used) return -1;
            if (table[p].used && !table[p].deleted && table[p].key == key) return p;
        }
        return -1;
    }

    void rehash(int newCap) {
        vector<Entry> old = table;
        table.assign(newCap, Entry());
        int oldCap = capacity;
        capacity = newCap;
        usedCount = 0;
        for (auto &e : old) {
            if (e.used && !e.deleted) insertInternal(e.key, e.value);
        }
    }
};

// ============================================================================
// Warehouse Registry & Product Registry
// ============================================================================

class WarehouseRegistry {
public:
    void addWarehouse(const Warehouse &w) {
        wh[w.id] = w;
    }

    bool exists(int id) const {
        return wh.count(id);
    }

    Warehouse get(int id) const {
        return wh.at(id);
    }

    vector<Warehouse> all() const {
        vector<Warehouse> v;
        for (auto &p : wh) v.push_back(p.second);
        return v;
    }

private:
    unordered_map<int,Warehouse> wh;
};

class ProductRegistry {
public:
    void addProduct(const Product &p) {
        items[p.id] = p;
    }

    bool exists(int id) const {
        return items.count(id);
    }

    Product get(int id) const {
        return items.at(id);
    }

    vector<Product> all() const {
        vector<Product> v;
        for (auto &p : items) v.push_back(p.second);
        return v;
    }

private:
    unordered_map<int,Product> items;
};

// ============================================================================
// Inventory Manager: Maps product → warehouse using HashMap
// ============================================================================

class InventoryMapper {
public:
    InventoryMapper(WarehouseRegistry &wr, ProductRegistry &pr)
        : warehouses(wr), products(pr), map(256) { }

    void assignProduct(int productId, int warehouseId) {
        if (!products.exists(productId)) return;
        if (!warehouses.exists(warehouseId)) return;
        map.put(productId, warehouseId);
    }

    bool getLocation(int productId, Warehouse &out) {
        int wid;
        if (!map.get(productId, wid)) return false;
        if (!warehouses.exists(wid)) return false;
        out = warehouses.get(wid);
        return true;
    }

    vector<pair<Product,Warehouse>> allAssignments() {
        vector<pair<Product,Warehouse>> out;
        auto pairs = map.allPairs();
        for (auto &p : pairs) {
            int pid = p.first;
            int wid = p.second;
            if (!products.exists(pid)) continue;
            if (!warehouses.exists(wid)) continue;
            out.push_back({products.get(pid), warehouses.get(wid)});
        }
        return out;
    }

private:
    WarehouseRegistry &warehouses;
    ProductRegistry &products;
    HashMap map;
};

// ============================================================================
// Product Assignment Simulator
// ============================================================================

class InventorySimulator {
public:
    InventorySimulator(InventoryMapper &im, WarehouseRegistry &wr, ProductRegistry &pr)
        : mapper(im), warehouses(wr), products(pr) {
        rng.seed(time(nullptr));
    }

    void streamAssignments(int count) {
        auto prd = products.all();
        auto whs = warehouses.all();
        if (prd.empty() || whs.empty()) return;

        uniform_int_distribution<int> p(0, prd.size()-1);
        uniform_int_distribution<int> w(0, whs.size()-1);

        for (int i = 0; i < count; i++) {
            int pid = prd[p(rng)].id;
            int wid = whs[w(rng)].id;
            mapper.assignProduct(pid, wid);
        }
    }

private:
    InventoryMapper &mapper;
    WarehouseRegistry &warehouses;
    ProductRegistry &products;
    mt19937 rng;
};

// ============================================================================
// CLI
// ============================================================================

static string trimRead(const string &prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static int readInt(const string &prompt, int def) {
    string s = trimRead(prompt);
    if (s.empty()) return def;
    try { return stoi(s); } catch (...) { return def; }
}

class InventoryCLI {
public:
    InventoryCLI(WarehouseRegistry &wr, ProductRegistry &pr, InventoryMapper &im, InventorySimulator &sim)
        : warehouses(wr), products(pr), mapper(im), simulator(sim) { }

    void run() {
        bool quit = false;
        while (!quit) {
            menu();
            string cmd = trimRead("Choice> ");
            if (cmd == "1") listWarehouses();
            else if (cmd == "2") listProducts();
            else if (cmd == "3") assignProduct();
            else if (cmd == "4") queryProductLocation();
            else if (cmd == "5") listAssignments();
            else if (cmd == "6") simulate();
            else if (cmd == "q") quit = true;
            else cout << "Unknown option\n";
        }
    }

private:
    WarehouseRegistry &warehouses;
    ProductRegistry &products;
    InventoryMapper &mapper;
    InventorySimulator &simulator;

    void menu() {
        cout << "\nInventory Mapping — Options\n";
        cout << " 1) List warehouses\n";
        cout << " 2) List products\n";
        cout << " 3) Assign product to warehouse\n";
        cout << " 4) Query product location\n";
        cout << " 5) List all assignments\n";
        cout << " 6) Simulate random assignments\n";
        cout << " q) Quit\n";
    }

    void listWarehouses() {
        auto all = warehouses.all();
        cout << "Warehouses:\n";
        for (auto &w : all) {
            cout << " " << w.id << " | " << w.name << " | " << w.zone << "\n";
        }
    }

    void listProducts() {
        auto all = products.all();
        cout << "Products:\n";
        for (auto &p : all) {
            cout << " " << p.id << " | " << p.name << " | " << p.category
                 << " | qty=" << p.quantity << "\n";
        }
    }

    void assignProduct() {
        int pid = readInt("Product ID: ", -1);
        int wid = readInt("Warehouse ID: ", -1);
        mapper.assignProduct(pid, wid);
        cout << "Assigned.\n";
    }

    void queryProductLocation() {
        int pid = readInt("Product ID: ", -1);
        Warehouse w;
        if (mapper.getLocation(pid, w)) {
            cout << "Located at: " << w.name << " (" << w.zone << ")\n";
        } else {
            cout << "Product not mapped.\n";
        }
    }

    void listAssignments() {
        auto all = mapper.allAssignments();
        cout << "Assignments:\n";
        for (auto &p : all) {
            cout << " Product " << p.first.name << " -> " << p.second.name << " (" << p.second.zone << ")\n";
        }
    }

    void simulate() {
        int count = readInt("How many random assignments? ", 10);
        simulator.streamAssignments(count);
        cout << "Simulation done.\n";
    }
};

// ============================================================================
// Default Data for Aroha Nagar
// ============================================================================

static void populateWarehouses(WarehouseRegistry &wr) {
    wr.addWarehouse({1,"Central Depot","Central"});
    wr.addWarehouse({2,"North Storage","North"});
    wr.addWarehouse({3,"East Hub","East"});
    wr.addWarehouse({4,"South ColdStore","South"});
    wr.addWarehouse({5,"West Reserve","West"});
}

static void populateProducts(ProductRegistry &pr) {
    pr.addProduct({101,"Processors","Electronics",500});
    pr.addProduct({102,"Mobile Screens","Electronics",300});
    pr.addProduct({103,"Charging Units","Electronics",700});
    pr.addProduct({201,"Wheat Bags","Agro",1200});
    pr.addProduct({202,"Rice Packets","Agro",900});
    pr.addProduct({203,"Groundnut Boxes","Agro",450});
    pr.addProduct({301,"Router Units","Networking",200});
    pr.addProduct({401,"Laptops","Computers",80});
}

// ============================================================================
// Main
// ============================================================================

int main() {
    WarehouseRegistry wr;
    ProductRegistry pr;
    populateWarehouses(wr);
    populateProducts(pr);

    InventoryMapper mapper(wr, pr);
    InventorySimulator simulator(mapper, wr, pr);
    InventoryCLI cli(wr, pr, mapper, simulator);

    cli.run();
    return 0;
}
