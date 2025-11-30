#include <bits/stdc++.h>
using namespace std;

// ===============================================================
// Fenwick Tree for Market Price Fluctuations
// ===============================================================
//
// Indexes represent days (1-indexed).
// Supports:
//  - point update (change in price)
//  - prefix sum
//  - range sum
//  - binary lifting to find first prefix >= value
//  - average in a date range
//
// ===============================================================

class Fenwick {
public:
    Fenwick(int n = 0) {
        init(n);
    }

    void init(int n) {
        size = n;
        tree.assign(n + 1, 0.0);
    }

    void update(int idx, double delta) {
        while (idx <= size) {
            tree[idx] += delta;
            idx += idx & -idx;
        }
    }

    double prefix(int idx) const {
        double s = 0.0;
        while (idx > 0) {
            s += tree[idx];
            idx -= idx & -idx;
        }
        return s;
    }

    double rangeQuery(int l, int r) const {
        if (l > r) return 0.0;
        return prefix(r) - prefix(l - 1);
    }

    double pointValue(int idx) const {
        // derive single index via difference of Fenwick sums
        return prefix(idx) - prefix(idx - 1);
    }

    // Find first index where prefix >= target (binary lifting)
    int lowerBound(double target) const {
        int pos = 0;
        double accumulated = 0.0;
        int pw = highestPowerOfTwo(size);

        for (int step = pw; step > 0; step >>= 1) {
            int nxt = pos + step;
            if (nxt <= size && accumulated + tree[nxt] < target) {
                accumulated += tree[nxt];
                pos = nxt;
            }
        }
        return pos + 1;
    }

private:
    int size;
    vector<double> tree;

    int highestPowerOfTwo(int x) const {
        int p = 1;
        while (p * 2 <= x) p <<= 1;
        return p;
    }
};

// ===============================================================
// Product Market (multiple products using separate Fenwick trees)
// ===============================================================

struct Product {
    string name;
    Fenwick ftree;
    int days;
    vector<double> history;

    Product() { }

    Product(const string &nm, int nDays) {
        name = nm;
        days = nDays;
        ftree.init(nDays);
        history.assign(nDays + 1, 0.0);
    }

    void setDayPrice(int day, double price) {
        double old = history[day];
        double delta = price - old;
        history[day] = price;
        ftree.update(day, delta);
    }

    double getDayPrice(int day) const {
        return history[day];
    }

    double average(int l, int r) const {
        if (l > r) return 0.0;
        double sum = ftree.rangeQuery(l, r);
        return sum / (r - l + 1);
    }

    double sum(int l, int r) const {
        return ftree.rangeQuery(l, r);
    }

    // price spike detection: day if price - avg > factor*stddev
    vector<int> detectSpikes() const {
        vector<int> res;
        if (days <= 5) return res;

        double mean = ftree.prefix(days) / days;
        double sq = 0.0;
        for (int i = 1; i <= days; i++) {
            double diff = history[i] - mean;
            sq += diff * diff;
        }
        double sd = sqrt(sq / days);
        double threshold = mean + 2.3 * sd;

        for (int i = 1; i <= days; i++) {
            if (history[i] > threshold) res.push_back(i);
        }
        return res;
    }

    // locate first day where cumulative price crosses target
    int cumulativeCross(double target) const {
        return ftree.lowerBound(target);
    }
};

// ===============================================================
// Market Database (Aroha Nagar digital retail)
// ===============================================================

class MarketDB {
public:
    MarketDB(int nDays = 200) : days(nDays) { }

    void addProduct(const string &name) {
        if (products.count(name)) return;
        products[name] = Product(name, days);
    }

    bool exists(const string &name) const {
        return products.count(name);
    }

    void setPrice(const string &name, int day, double price) {
        products[name].setDayPrice(day, price);
    }

    double getPrice(const string &name, int day) const {
        return products.at(name).getDayPrice(day);
    }

    vector<int> spikes(const string &name) const {
        return products.at(name).detectSpikes();
    }

    double avgPrice(const string &name, int l, int r) const {
        return products.at(name).average(l, r);
    }

    vector<string> listProducts() const {
        vector<string> v;
        for (auto &p : products) v.push_back(p.first);
        return v;
    }

    int cumulativeCross(const string &name, double target) const {
        return products.at(name).cumulativeCross(target);
    }

    int daysCount() const { return days; }

private:
    int days;
    unordered_map<string, Product> products;
};

// ===============================================================
// CSV Export for MarketDB
// ===============================================================

class MarketExporter {
public:
    static bool exportHistory(const MarketDB &db, const string &prod, const string &filename) {
        ofstream out(filename);
        if (!out.is_open()) return false;
        out << "day,price\n";
        int days = db.daysCount();
        for (int i = 1; i <= days; i++) {
            out << i << "," << db.getPrice(prod, i) << "\n";
        }
        out.close();
        return true;
    }

    static bool exportSpikeReport(const vector<int> &spk, const string &filename) {
        ofstream out(filename);
        if (!out.is_open()) return false;
        out << "day\n";
        for (int d : spk) out << d << "\n";
        out.close();
        return true;
    }
};

// ===============================================================
// Market Price Simulator (Aroha Nagar digital retail + e-commerce)
// ===============================================================

class MarketSimulator {
public:
    MarketSimulator(MarketDB &db) : market(db) {
        rng.seed(time(nullptr));
    }

    void simulateProduct(const string &prod, int startDay, int endDay) {
        double base = basePrice(prod);
        normal_distribution<double> vol(0.0, 1.2);

        for (int day = startDay; day <= endDay; day++) {
            double drift = driftFactor(day);
            double noise = vol(rng);
            double price = base + drift + noise;
            if (price < 1) price = 1;
            market.setPrice(prod, day, price);
        }
    }

    void simulateAll(int startDay, int endDay) {
        for (auto &name : market.listProducts()) {
            simulateProduct(name, startDay, endDay);
        }
    }

private:
    MarketDB &market;
    mt19937 rng;

    double basePrice(const string &p) {
        if (p == "rice") return 40;
        if (p == "wheat") return 35;
        if (p == "groundnut") return 70;
        if (p == "dal") return 60;
        if (p == "electronics") return 120;
        return 50;
    }

    double driftFactor(int day) {
        return sin(day / 12.0) * 3.5;
    }
};


#include <iomanip>
#include <sstream>
#include <fstream>
#include <cmath>

// ===============================================================
// Analysis Utilities
// ===============================================================

static double stddevRange(const MarketDB &db, const string &prod, int l, int r) {
    int n = r - l + 1;
    if (n <= 1) return 0.0;
    double mean = db.avgPrice(prod, l, r);
    double s = 0.0;
    for (int day = l; day <= r; ++day) {
        double v = db.getPrice(prod, day);
        double d = v - mean;
        s += d * d;
    }
    return sqrt(s / n);
}

static vector<pair<string,double>> topKVolatile(const MarketDB &db, int l, int r, int k) {
    vector<pair<string,double>> list;
    for (auto &name : db.listProducts()) {
        double sd = stddevRange(db, name, l, r);
        list.emplace_back(name, sd);
    }
    sort(list.begin(), list.end(), [](const auto &a, const auto &b){
        return a.second > b.second;
    });
    if ((int)list.size() > k) list.resize(k);
    return list;
}

// ===============================================================
// Interactive Market Driver
// ===============================================================

class MarketDriver {
public:
    MarketDriver(MarketDB &db) : market(db), simulator(db) { }

    void run() {
        printWelcome();
        bool stop = false;
        while (!stop) {
            printMenu();
            string cmd = readLineTrimmed("Choice> ");
            if (cmd == "1") actionSimulate();
            else if (cmd == "2") actionShowProducts();
            else if (cmd == "3") actionQueryPrice();
            else if (cmd == "4") actionAvgRange();
            else if (cmd == "5") actionSpikes();
            else if (cmd == "6") actionExport();
            else if (cmd == "7") actionCumulativeCross();
            else if (cmd == "8") actionTopVolatile();
            else if (cmd == "q" || cmd == "quit") stop = true;
            else cout << "Unknown option\n";
        }
        cout << "Exiting market driver.\n";
    }

private:
    MarketDB &market;
    MarketSimulator simulator;

    void printWelcome() {
        cout << "Aroha Nagar — Market Price Tracker\n";
        cout << "Products tracked: " << market.listProducts().size() << "\n\n";
    }

    void printMenu() {
        cout << "\nOptions:\n";
        cout << " 1) Simulate product prices for day range\n";
        cout << " 2) Show product list\n";
        cout << " 3) Query price (product + day)\n";
        cout << " 4) Average price over range\n";
        cout << " 5) Show price spikes for a product\n";
        cout << " 6) Export product history or spike report\n";
        cout << " 7) Find first day cumulative price crosses target\n";
        cout << " 8) Top-K volatile products in a range\n";
        cout << " q) Quit\n";
    }

    void actionSimulate() {
        string prod = readLineTrimmed("Product name (or 'all'): ");
        int s = readInt("Start day (1-based) default 1: ", 1);
        int e = readInt("End day default 200: ", market.daysCount());
        if (prod.empty()) prod = "all";
        if (prod == "all") {
            simulator.simulateAll(s, e);
            cout << "Simulated all products from day " << s << " to " << e << ".\n";
        } else {
            if (!market.exists(prod)) {
                cout << "Product not found. Adding it now.\n";
                market.addProduct(prod);
            }
            simulator.simulateProduct(prod, s, e);
            cout << "Simulated " << prod << " from day " << s << " to " << e << ".\n";
        }
    }

    void actionShowProducts() {
        auto v = market.listProducts();
        cout << "Products:\n";
        for (auto &p : v) cout << " - " << p << "\n";
    }

    void actionQueryPrice() {
        string prod = readLineTrimmed("Product: ");
        if (!market.exists(prod)) { cout << "Unknown product\n"; return; }
        int day = readInt("Day (1...): ", 1);
        cout << "Price of " << prod << " on day " << day << " = " << market.getPrice(prod, day) << "\n";
    }

    void actionAvgRange() {
        string prod = readLineTrimmed("Product: ");
        if (!market.exists(prod)) { cout << "Unknown product\n"; return; }
        int l = readInt("Start day: ", 1);
        int r = readInt("End day: ", market.daysCount());
        cout << "Average price of " << prod << " from " << l << " to " << r << " = "
             << fixed << setprecision(3) << market.avgPrice(prod, l, r) << "\n";
        cout << "Stddev = " << fixed << setprecision(3) << stddevRange(market, prod, l, r) << "\n";
    }

    void actionSpikes() {
        string prod = readLineTrimmed("Product: ");
        if (!market.exists(prod)) { cout << "Unknown product\n"; return; }
        auto sp = market.spikes(prod);
        cout << "Spike days for " << prod << ": ";
        if (sp.empty()) cout << "none\n";
        else {
            for (int d : sp) cout << d << " ";
            cout << "\n";
        }
    }

    void actionExport() {
        string prod = readLineTrimmed("Product (for history) or leave empty to cancel: ");
        if (prod.empty()) return;
        if (!market.exists(prod)) { cout << "Product not known, adding then exporting.\n"; market.addProduct(prod); }
        string base = readLineTrimmed("Base filename (without ext) default=" + prod + "_hist: ");
        if (base.empty()) base = prod + "_hist";
        string fileHist = base + ".csv";
        bool ok = MarketExporter::exportHistory(market, prod, fileHist);
        cout << "Export history: " << (ok ? "OK" : "FAILED") << " -> " << fileHist << "\n";

        string wantSp = readLineTrimmed("Export spikes too? (y/n): ");
        if (wantSp == "y" || wantSp == "Y") {
            auto sp = market.spikes(prod);
            string spfile = base + "_spikes.csv";
            bool ok2 = MarketExporter::exportSpikeReport(sp, spfile);
            cout << "Export spikes: " << (ok2 ? "OK" : "FAILED") << " -> " << spfile << "\n";
        }
    }

    void actionCumulativeCross() {
        string prod = readLineTrimmed("Product: ");
        if (!market.exists(prod)) { cout << "Unknown product\n"; return; }
        double target = readDouble("Cumulative price target (sum over days): ", 100.0);
        int day = market.cumulativeCross(prod, target);
        if (day <= market.daysCount())
            cout << "Cumulative sum crosses " << target << " on day " << day << "\n";
        else
            cout << "Cumulative sum does not cross target within tracked days.\n";
    }

    void actionTopVolatile() {
        int l = readInt("Start day: ", 1);
        int r = readInt("End day: ", market.daysCount());
        int k = readInt("Top K: ", 5);
        auto list = topKVolatile(market, l, r, k);
        cout << "Top " << k << " volatile products from day " << l << " to " << r << ":\n";
        for (auto &p : list) {
            cout << "  " << p.first << " : sd=" << fixed << setprecision(3) << p.second << "\n";
        }
    }
};

// ===============================================================
// Main — driver and quick demo
// ===============================================================

int main(int argc, char **argv) {
    // Default days tracked
    int days = 200;
    MarketDB db(days);

    // Add a sensible set of products (Aroha Nagar example)
    vector<string> initial = {"rice", "wheat", "groundnut", "dal", "electronics", "vegetables", "fruits"};
    for (auto &p : initial) db.addProduct(p);

    // Headless demo mode: if args given, run simulation and produce files
    if (argc >= 2 && string(argv[1]) == "--demo") {
        MarketSimulator sim(db);
        sim.simulateAll(1, days);
        for (auto &p : db.listProducts()) {
            MarketExporter::exportHistory(db, p, p + "_demo.csv");
            auto spikes = db.spikes(p);
            MarketExporter::exportSpikeReport(spikes, p + "_spikes_demo.csv");
        }
        cout << "Demo run complete. CSV files written.\n";
        return 0;
    }

    // Interactive driver
    MarketDriver driver(db);
    driver.run();
    return 0;
}
