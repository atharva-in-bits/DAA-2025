// restaurant_system.cpp
// Extended Restaurant / Café Management Module for project simulation
// Implements: menu+inventory (raw ingredients + dishes), reservations, dine-in & takeaway orders,
// stock + ingredient checking, sales analytics, and a simple console interface.
// Uses: unordered_map (hashing), queue, priority_queue (heap), interval-based reservation checks,
// Fenwick Tree (BIT) for analytics per dish, modular design for easy extension.

#include <bits/stdc++.h>
using namespace std;

/* === Fenwick Tree (BIT) for sales analytics per dish === */
struct Fenwick {
    int n;
    vector<long long> bit;
    Fenwick(): n(0) {}
    Fenwick(int _n): n(_n), bit(_n+1, 0), n(_n) {}
    void init(int _n) { n = _n; bit.assign(n+1, 0); }
    void update(int i, long long delta) {
        for (; i <= n; i += i & -i) bit[i] += delta;
    }
    long long query(int i) const {
        long long s = 0;
        for (; i > 0; i -= i & -i) s += bit[i];
        return s;
    }
    long long rangeQuery(int l, int r) const {
        if (l > r) return 0;
        return query(r) - query(l-1);
    }
};

/* === Data structures for inventory and menu === */
// Represents a raw ingredient (for realistic stock tracking)
struct Ingredient {
    string name;
    double quantity;  // e.g. grams / units / liters
    // Optionally: expiry date, batch no., etc.
};

// Represents a dish/menu item: price + recipe (ingredients required)
struct Dish {
    string name;
    double price;
    unordered_map<string, double> recipe;  // ingredient name → quantity required per dish
};

/* === Order representation === */
struct Order {
    int orderId;
    bool isDelivery;     // delivery/takeaway order or dine-in
    vector<string> dishes;       // list of dish names
    int priority;         // only relevant if isDelivery == true
    Order(int id, bool del, const vector<string>& ds, int pr = 0)
        : orderId(id), isDelivery(del), dishes(ds), priority(pr) {}
};

// Comparison for delivery orders (min-heap by priority, lower first)
struct OrderCompare {
    bool operator()(const Order &a, const Order &b) const {
        return a.priority > b.priority;
    }
};

/* === Reservation record === */
struct Reservation {
    int tableId;
    int fromTime;   // simple integer-based timeslots (e.g. hours or fixed slots)
    int toTime;
    string customerName;
};

class Restaurant {
private:
    int numTables;

    // Ingredient inventory: ingredient name → Ingredient
    unordered_map<string, Ingredient> inventory;

    // Menu: dish name → Dish
    unordered_map<string, Dish> menu;

    // Mapping for analytics: dish name → index for Fenwick
    unordered_map<string,int> dishToIdx;
    vector<string> idxToDish;
    Fenwick salesFenw;

    // Order queues:
    queue<Order> dineInQueue;
    priority_queue<Order, vector<Order>, OrderCompare> deliveryQueue;

    // Reservations: tableId → list of reservations
    unordered_map<int, vector<Reservation>> reservations;

    int nextOrderId = 1;

public:
    Restaurant(int tables): numTables(tables) {}

    /* Add a raw ingredient or restock */
    void addIngredient(const string &iname, double qty) {
        inventory[iname].name = iname;
        inventory[iname].quantity += qty;
    }

    /* Define a new dish (menu item) with price and recipe */
    void addDish(const string &dname, double price, const unordered_map<string,double> &recipe) {
        menu[dname] = {dname, price, recipe};
        if (!dishToIdx.count(dname)) {
            int idx = idxToDish.size() + 1;
            dishToIdx[dname] = idx;
            idxToDish.push_back(dname);
            salesFenw.init(idxToDish.size());
        }
    }

    /* Check if dish can be prepared: all ingredients available in required quantities */
    bool canMakeDish(const string &dname) {
        if (!menu.count(dname)) return false;
        for (auto &p : menu[dname].recipe) {
            const string &ing = p.first;
            double needed = p.second;
            if (!inventory.count(ing) || inventory[ing].quantity < needed) {
                return false;
            }
        }
        return true;
    }

    /* Deduct ingredients on cooking */
    void useIngredients(const string &dname) {
        for (auto &p : menu[dname].recipe) {
            const string &ing = p.first;
            double needed = p.second;
            inventory[ing].quantity -= needed;
        }
    }

    /* Place dine-in order */
    bool placeDineInOrder(const vector<string> &dishes) {
        for (auto &d : dishes) {
            if (!canMakeDish(d)) return false;
        }
        for (auto &d : dishes) {
            useIngredients(d);
        }
        dineInQueue.push(Order(nextOrderId++, false, dishes));
        return true;
    }

    /* Place delivery/takeaway order with priority */
    bool placeDeliveryOrder(const vector<string> &dishes, int priority = 100) {
        for (auto &d : dishes) {
            if (!canMakeDish(d)) return false;
        }
        for (auto &d : dishes) {
            useIngredients(d);
        }
        deliveryQueue.push(Order(nextOrderId++, true, dishes, priority));
        return true;
    }

    /* Reserve a table for [fromT, toT) time slot */
    bool reserveTable(int tableId, int fromT, int toT, const string &custName) {
        if (tableId < 1 || tableId > numTables || fromT >= toT) return false;
        auto &v = reservations[tableId];
        for (auto &r : v) {
            if (!(toT <= r.fromTime || fromT >= r.toTime)) {
                return false;  // overlap
            }
        }
        v.push_back({tableId, fromT, toT, custName});
        return true;
    }

    /* Process next orders: one dine-in and one delivery per call */
    void processNext() {
        if (!dineInQueue.empty()) {
            Order ord = dineInQueue.front(); dineInQueue.pop();
            serveOrder(ord, false);
        }
        if (!deliveryQueue.empty()) {
            Order ord = deliveryQueue.top(); deliveryQueue.pop();
            serveOrder(ord, true);
        }
    }

    /* Serve an order: simple output + analytics update */
    void serveOrder(const Order &ord, bool delivery) {
        double total = 0.0;
        cout << (delivery ? "[Delivery]" : "[Dine-In]") << " Order #" << ord.orderId << " served. Items: ";
        for (auto &d : ord.dishes) {
            cout << d << " ";
            total += menu[d].price;
            int idx = dishToIdx[d];
            salesFenw.update(idx, 1);
        }
        cout << "| Total price: " << total << "\n";
    }

    /* Query how many times a dish was sold so far */
    long long getSalesCount(const string &dname) const {
        if (!dishToIdx.count(dname)) return 0;
        int idx = dishToIdx.at(dname);
        return salesFenw.query(idx);
    }

    /* Print current status: inventory, pending orders, reservations */
    void printStatus() const {
        cout << "\n=== Restaurant Status ===\n";
        cout << "-- Ingredients stock --\n";
        for (auto &p : inventory) {
            cout << "  " << p.first << " : " << p.second.quantity << "\n";
        }
        cout << "-- Menu dishes --\n";
        for (auto &p : menu) {
            cout << "  " << p.first << " : price = " << p.second.price << "\n";
        }
        cout << "-- Pending dine-in orders: " << dineInQueue.size()
             << "  delivery orders: " << deliveryQueue.size() << "\n";
        cout << "-- Reservations --\n";
        for (auto &pr : reservations) {
            for (auto &r : pr.second) {
                cout << "   Table " << r.tableId << " : [" << r.fromTime
                     << "," << r.toTime << ") by " << r.customerName << "\n";
            }
        }
        cout << "==========================\n\n";
    }
};

/* === Demo console usage === */
int main() {
    Restaurant cafe(5);  // 5 tables

    // Setup: add ingredients
    cafe.addIngredient("Bread", 1000);
    cafe.addIngredient("Cheese", 500);
    cafe.addIngredient("Tomato", 300);
    cafe.addIngredient("CoffeeBeans", 100);
    cafe.addIngredient("Milk", 200);
    cafe.addIngredient("PancakeMix", 200);
    // define dishes
    cafe.addDish("Sandwich", 80.0, {{"Bread",2}, {"Cheese",1}, {"Tomato",1}});
    cafe.addDish("Coffee", 50.0, {{"CoffeeBeans",5}, {"Milk",1}});
    cafe.addDish("Pancake", 100.0, {{"PancakeMix",1}, {"Milk",1}});

    cafe.printStatus();

    // Reserve a table
    bool r1 = cafe.reserveTable(2, 18, 20, "Alice");
    bool r2 = cafe.reserveTable(2, 19, 21, "Bob");  // should fail (overlap)
    cout << "Reservation for Alice: " << (r1 ? "OK" : "Failed") << "\n";
    cout << "Reservation for Bob: " << (r2 ? "OK" : "Failed (overlap)") << "\n";

    // Place some orders
    vector<string> o1 = {"Sandwich", "Coffee"};
    vector<string> o2 = {"Pancake"};
    vector<string> o3 = {"Coffee", "Coffee", "Sandwich"};

    cafe.placeDineInOrder(o1);
    cafe.placeDeliveryOrder(o2, 50);
    cafe.placeDeliveryOrder(o3, 30);

    cout << "\nProcessing orders...\n";
    cafe.processNext();
    cafe.processNext();
    cafe.processNext();

    cafe.printStatus();

    cout << "Sales stats:\n";
    cout << " Sandwich sold: " << cafe.getSalesCount("Sandwich") << "\n";
    cout << " Coffee sold: " << cafe.getSalesCount("Coffee") << "\n";
    cout << " Pancake sold: " << cafe.getSalesCount("Pancake") << "\n";

    return 0;
}
