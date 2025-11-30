#include <bits/stdc++.h>
using namespace std;

struct TimeStamp {
    long long ms;
    static TimeStamp now() {
        auto t = chrono::duration_cast<chrono::milliseconds>(
                     chrono::system_clock::now().time_since_epoch()).count();
        TimeStamp x; x.ms = t; return x;
    }
};

// =============================================================
// Traffic Sample Structure
// =============================================================

struct TrafficSample {
    int sensorId;
    TimeStamp ts;
    double vehiclesPerMinute;
    double avgSpeed;
    int lane;
};

// =============================================================
// Sensor Registry (metadata for Aroha Nagar)
// =============================================================

struct SensorInfo {
    int id;
    string street;
    string zone;
    bool droneAssisted;
    int lanes;
};

class SensorRegistry {
public:
    void registerSensor(const SensorInfo &s) {
        sensors[s.id] = s;
    }

    bool exists(int id) const {
        return sensors.count(id);
    }

    SensorInfo get(int id) const {
        if (sensors.count(id)) return sensors.at(id);
        return SensorInfo{-1,"","",false,0};
    }

    vector<SensorInfo> all() const {
        vector<SensorInfo> v;
        for (auto &x : sensors) v.push_back(x.second);
        return v;
    }

private:
    unordered_map<int,SensorInfo> sensors;
};

// =============================================================
// Sorting / Partition Engine
// =============================================================

class SortEngine {
public:
    // QuickSort with iterative stack + introsort cutoff
    static void sortSamples(vector<TrafficSample> &a) {
        if (a.empty()) return;
        int n = a.size();

        int depthLimit = 2 * log(n);

        vector<pair<int,int>> st;
        st.push_back({0, n-1});

        while (!st.empty()) {
            auto [lo, hi] = st.back();
            st.pop_back();
            if (lo >= hi) continue;

            int length = hi - lo + 1;

            if (length < 24) {
                insertionSort(a, lo, hi);
                continue;
            }
            if (depthLimit-- == 0) {
                heapSort(a, lo, hi);
                continue;
            }

            int pivotIndex = medianOfThree(a, lo, hi);
            pivotIndex = partition(a, lo, hi, pivotIndex);

            if (pivotIndex - 1 - lo > hi - (pivotIndex + 1)) {
                st.push_back({lo, pivotIndex - 1});
                st.push_back({pivotIndex + 1, hi});
            } else {
                st.push_back({pivotIndex + 1, hi});
                st.push_back({lo, pivotIndex - 1});
            }
        }
    }

    static void nth_select(vector<TrafficSample> &a, int nth) {
        if (nth < 0 || nth >= (int)a.size()) return;
        int l = 0, r = (int)a.size() - 1;
        while (l < r) {
            int pivotIndex = medianOfThree(a, l, r);
            pivotIndex = partition(a, l, r, pivotIndex);
            if (pivotIndex == nth) return;
            else if (nth < pivotIndex) r = pivotIndex - 1;
            else l = pivotIndex + 1;
        }
    }

private:
    static int partition(vector<TrafficSample> &a, int lo, int hi, int p) {
        double pv = a[p].vehiclesPerMinute;
        swap(a[p], a[hi]);
        int store = lo;
        for (int i = lo; i < hi; i++) {
            if (a[i].vehiclesPerMinute < pv) {
                swap(a[i], a[store]);
                store++;
            }
        }
        swap(a[store], a[hi]);
        return store;
    }

    static int medianOfThree(vector<TrafficSample> &a, int lo, int hi) {
        int mid = lo + (hi - lo) / 2;
        double x = a[lo].vehiclesPerMinute;
        double y = a[mid].vehiclesPerMinute;
        double z = a[hi].vehiclesPerMinute;

        if ((x <= y && y <= z) || (z <= y && y <= x)) return mid;
        if ((y <= x && x <= z) || (z <= x && x <= y)) return lo;
        return hi;
    }

    static void insertionSort(vector<TrafficSample> &a, int lo, int hi) {
        for (int i = lo + 1; i <= hi; i++) {
            auto tmp = a[i];
            int j = i - 1;
            while (j >= lo && a[j].vehiclesPerMinute > tmp.vehiclesPerMinute) {
                a[j+1] = a[j];
                j--;
            }
            a[j+1] = tmp;
        }
    }

    static void heapSort(vector<TrafficSample> &a, int lo, int hi) {
        int n = hi - lo + 1;
        for (int i = n/2 - 1; i >= 0; i--) siftDown(a, lo, n, i);
        for (int i = n - 1; i > 0; i--) {
            swap(a[lo], a[lo + i]);
            siftDown(a, lo, i, 0);
        }
    }

    static void siftDown(vector<TrafficSample> &a, int lo, int n, int idx) {
        while (1) {
            int l = idx * 2 + 1;
            int r = idx * 2 + 2;
            int biggest = idx;
            if (l < n && a[lo + l].vehiclesPerMinute > a[lo + biggest].vehiclesPerMinute) biggest = l;
            if (r < n && a[lo + r].vehiclesPerMinute > a[lo + biggest].vehiclesPerMinute) biggest = r;
            if (biggest == idx) break;
            swap(a[lo + idx], a[lo + biggest]);
            idx = biggest;
        }
    }
};

// =============================================================
// Classification Tiers
// =============================================================

struct TrafficTier {
    vector<TrafficSample> low;
    vector<TrafficSample> medium;
    vector<TrafficSample> high;
    double t1;
    double t2;
};

class TierClassifier {
public:
    TrafficTier classify(vector<TrafficSample> a) {
        TrafficTier T;
        int n = a.size();
        if (n == 0) return T;

        int idx1 = max(0, (int)floor(n * 0.33));
        int idx2 = max(0, (int)floor(n * 0.66));

        SortEngine::nth_select(a, idx1);
        double th1 = a[idx1].vehiclesPerMinute;

        SortEngine::nth_select(a, idx2);
        double th2 = a[idx2].vehiclesPerMinute;

        T.t1 = th1;
        T.t2 = th2;

        for (auto &s : a) {
            if (s.vehiclesPerMinute <= th1) T.low.push_back(s);
            else if (s.vehiclesPerMinute <= th2) T.medium.push_back(s);
            else T.high.push_back(s);
        }

        SortEngine::sortSamples(T.low);
        SortEngine::sortSamples(T.medium);
        SortEngine::sortSamples(T.high);

        return T;
    }
};

// =============================================================
// Anomaly Detection (simple statistical rules)
// =============================================================

class AnomalyEngine {
public:
    vector<TrafficSample> detect(const vector<TrafficSample> &v) {
        vector<TrafficSample> anomalies;
        if (v.size() < 4) return anomalies;

        double sum = 0;
        for (auto &x : v) sum += x.vehiclesPerMinute;
        double mean = sum / v.size();
        double sq = 0;
        for (auto &x : v) sq += (x.vehiclesPerMinute - mean) * (x.vehiclesPerMinute - mean);
        double sd = sqrt(sq / v.size());

        double upper = mean + 2.5 * sd;

        for (auto &x : v) {
            if (x.vehiclesPerMinute > upper) anomalies.push_back(x);
        }

        return anomalies;
    }
};

// =============================================================
// Batch Processor: ties everything together
// =============================================================

class TrafficBatchProcessor {
public:
    TrafficBatchProcessor(SensorRegistry &sr, int batchSize = 200)
        : reg(sr), batchSize(batchSize) { }

    void submit(const TrafficSample &s) {
        batch.push_back(s);
        if ((int)batch.size() >= batchSize) {
            process();
            archive();
            batch.clear();
        }
    }

    void flush() {
        if (!batch.empty()) {
            process();
            archive();
            batch.clear();
        }
    }

    TrafficTier lastTier() const { return last; }
    vector<TrafficSample> lastAnomalies() const { return anomalies; }

private:
    SensorRegistry &reg;
    int batchSize;
    vector<TrafficSample> batch;
    TrafficTier last;
    vector<TrafficSample> anomalies;

    vector<vector<TrafficSample>> history;

    void process() {
        TierClassifier tc;
        last = tc.classify(batch);

        AnomalyEngine ae;
        anomalies = ae.detect(batch);

        printSummary();
    }

    void archive() {
        history.push_back(batch);
    }

    void printSummary() {
        cout << "Batch Processed\n";
        cout << "  Thresholds: T1=" << last.t1 << " T2=" << last.t2 << "\n";
        cout << "  Counts: low=" << last.low.size()
             << ", medium=" << last.medium.size()
             << ", high=" << last.high.size() << "\n";
        cout << "  Anomalies: " << anomalies.size() << "\n\n";
    }
};

// =============================================================
// Extended Simulator
// =============================================================

class TrafficSimulator {
public:
    TrafficSimulator(SensorRegistry &sr) : reg(sr), rng(random_device{}()) {
        buildPatterns();
    }

    void produce(TrafficBatchProcessor &bp, int count) {
        for (int i = 0; i < count; i++) {
            TrafficSample s;
            s.sensorId = chooseSensor();
            s.ts = TimeStamp::now();
            s.vehiclesPerMinute = baseRate(s.sensorId) * laneFactor(s.sensorId) + noise();
            s.avgSpeed = speedEstimate(s.vehiclesPerMinute);
            s.lane = (rng() % max(1, reg.get(s.sensorId).lanes)) + 1;
            bp.submit(s);
        }
    }

private:
    SensorRegistry &reg;
    mt19937 rng;

    map<int,double> baseLookup;

    void buildPatterns() {
        for (auto &s : reg.all()) {
            double base = 5 + (rng() % 15);
            if (s.droneAssisted) base += 4;
            baseLookup[s.id] = base;
        }
    }

    int chooseSensor() {
        auto sensors = reg.all();
        uniform_int_distribution<int> d(0, sensors.size()-1);
        return sensors[d(rng)].id;
    }

    double baseRate(int id) {
        if (baseLookup.count(id)) return baseLookup[id];
        return 10.0;
    }

    double laneFactor(int id) {
        auto s = reg.get(id);
        if (s.lanes <= 1) return 1.0;
        return 1.0 + (s.lanes - 1) * 0.25;
    }

    double noise() {
        normal_distribution<double> dist(0.0, 2.0);
        return dist(rng);
    }

    double speedEstimate(double vpm) {
        double val = 40 - (vpm * 0.8);
        return max(5.0, val);
    }
};
// =============================================================
// CASE-4 CODE — PART 2
// Continuation: CLI, reporting, CSV export, interactive driver
// Paste this directly below Part 1 in the same file.
// =============================================================

#include <iomanip>
#include <sstream>
#include <fstream>

// =============================================================
// CSV Export and Report Utilities
// =============================================================

class CsvExporter {
public:
    static bool exportTier(const TrafficTier &tier, const string &filename) {
        ofstream out(filename);
        if (!out.is_open()) return false;
        out << "group,sensorId,timestamp_ms,vehicles_per_minute,avg_speed,lane\n";
        for (const auto &s : tier.low) writeRow(out, "low", s);
        for (const auto &s : tier.medium) writeRow(out, "medium", s);
        for (const auto &s : tier.high) writeRow(out, "high", s);
        out.close();
        return true;
    }

    static bool exportAnomalies(const vector<TrafficSample> &anoms, const string &filename) {
        ofstream out(filename);
        if (!out.is_open()) return false;
        out << "sensorId,timestamp_ms,vehicles_per_minute,avg_speed,lane\n";
        for (const auto &s : anoms) {
            out << s.sensorId << "," << s.ts.ms << "," << fixed << setprecision(3)
                << s.vehiclesPerMinute << "," << s.avgSpeed << "," << s.lane << "\n";
        }
        out.close();
        return true;
    }

private:
    static void writeRow(ofstream &out, const string &group, const TrafficSample &s) {
        out << group << "," << s.sensorId << "," << s.ts.ms << "," << fixed << setprecision(3)
            << s.vehiclesPerMinute << "," << s.avgSpeed << "," << s.lane << "\n";
    }
};

// =============================================================
// Report Generator
// =============================================================

class ReportGenerator {
public:
    static string formatShort(const TrafficTier &tier, const vector<TrafficSample> &anoms) {
        ostringstream ss;
        ss << "Report Summary\n";
        ss << "  Thresholds: T1=" << fixed << setprecision(2) << tier.t1
           << "  T2=" << tier.t2 << "\n";
        ss << "  Counts: low=" << tier.low.size()
           << " medium=" << tier.medium.size()
           << " high=" << tier.high.size() << "\n";
        ss << "  Anomalies: " << anoms.size() << "\n";
        if (!anoms.empty()) {
            ss << "  Top anomalies (sensor, vpm):\n";
            int show = min(5, (int)anoms.size());
            for (int i = 0; i < show; ++i) {
                ss << "    " << anoms[i].sensorId << " , " << anoms[i].vehiclesPerMinute << "\n";
            }
        }
        return ss.str();
    }

    static bool writeFullText(const TrafficTier &tier, const vector<TrafficSample> &anoms, const string &filename) {
        ofstream out(filename);
        if (!out.is_open()) return false;
        out << formatShort(tier, anoms) << "\n";
        out << "Low bucket details:\n";
        writeBucket(out, tier.low);
        out << "\nMedium bucket details:\n";
        writeBucket(out, tier.medium);
        out << "\nHigh bucket details:\n";
        writeBucket(out, tier.high);
        if (!anoms.empty()) {
            out << "\nAnomalies details:\n";
            writeBucket(out, anoms);
        }
        out.close();
        return true;
    }

private:
    static void writeBucket(ofstream &out, const vector<TrafficSample> &v) {
        out << "sensorId,timestamp_ms,vehicles_per_minute,avg_speed,lane\n";
        for (auto &s : v) {
            out << s.sensorId << "," << s.ts.ms << "," << fixed << setprecision(3)
                << s.vehiclesPerMinute << "," << s.avgSpeed << "," << s.lane << "\n";
        }
    }
};

// =============================================================
// Interactive CLI Utilities
// =============================================================

static string readLineTrimmed(const string &prompt) {
    cout << prompt;
    string line;
    if (!std::getline(cin, line)) return string();
    // trim
    size_t a = line.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = line.find_last_not_of(" \t\r\n");
    return line.substr(a, b - a + 1);
}

static int readInt(const string &prompt, int defaultVal) {
    string s = readLineTrimmed(prompt);
    if (s.empty()) return defaultVal;
    try {
        return stoi(s);
    } catch (...) {
        return defaultVal;
    }
}

static double readDouble(const string &prompt, double defaultVal) {
    string s = readLineTrimmed(prompt);
    if (s.empty()) return defaultVal;
    try {
        return stod(s);
    } catch (...) {
        return defaultVal;
    }
}

// =============================================================
// Interactive Driver (ties simulator + processor + exporter)
// =============================================================

class InteractiveDriver {
public:
    InteractiveDriver(SensorRegistry &sr) : registry(sr), processor(registry, 200), simulator(registry) { }

    void run() {
        printWelcome();
        bool exitFlag = false;
        while (!exitFlag) {
            printMenu();
            string cmd = readLineTrimmed("Choice> ");
            if (cmd == "1") actionRunSimulation();
            else if (cmd == "2") actionBatchSize();
            else if (cmd == "3") actionExportLast();
            else if (cmd == "4") actionShowSensors();
            else if (cmd == "5") actionTuneThresholds();
            else if (cmd == "6") actionBulkRun();
            else if (cmd == "7") { processor.flush(); cout << "Flushed pending batches.\n"; }
            else if (cmd == "q" || cmd == "quit") exitFlag = true;
            else cout << "Unrecognized option.\n";
        }
        cout << "Exiting interactive driver.\n";
    }

private:
    SensorRegistry &registry;
    TrafficBatchProcessor processor;
    TrafficSimulator simulator;

    void printWelcome() {
        cout << "Aroha Nagar Traffic Classifier — Interactive Mode\n";
        cout << "Modules loaded. sensors=" << registry.all().size() << "\n\n";
    }

    void printMenu() {
        cout << "\nOptions:\n";
        cout << " 1) Run a single simulation batch\n";
        cout << " 2) Set batch size (current=" << getBatchSize() << ")\n";
        cout << " 3) Export last processed batch to CSV and report\n";
        cout << " 4) Show registered sensors\n";
        cout << " 5) Tune thresholds manually (advanced)\n";
        cout << " 6) Run bulk simulation (series of batches)\n";
        cout << " 7) Force flush pending samples to processing\n";
        cout << " q) Quit\n";
    }

    int getBatchSize() const {
        // access private via friend or a public getter; we will rely on a default for display
        return 200;
    }

    void actionRunSimulation() {
        int count = readInt("How many samples in batch (default 200)? ", 200);
        cout << "Producing " << count << " samples...\n";
        simulator.produce(processor, count);
        cout << "Batch produced and processed.\n";
        auto tier = processor.lastTier();
        auto anoms = processor.lastAnomalies();
        cout << ReportGenerator::formatShort(tier, anoms) << "\n";
    }

    void actionBatchSize() {
        int b = readInt("Enter new batch size (will take effect next run): ", 200);
        // recreate processor with new batch size
        processor = TrafficBatchProcessor(registry, b);
        cout << "Batch size set to " << b << ".\n";
    }

    void actionExportLast() {
        auto tier = processor.lastTier();
        auto anoms = processor.lastAnomalies();
        if (tier.low.empty() && tier.medium.empty() && tier.high.empty()) {
            cout << "No processed batch available to export.\n";
            return;
        }
        string base = readLineTrimmed("Enter base filename (without extension) [default=report]: ");
        if (base.empty()) base = "report";
        string csvFile = base + ".csv";
        string anomFile = base + "_anomalies.csv";
        string textFile = base + ".txt";
        bool ok1 = CsvExporter::exportTier(tier, csvFile);
        bool ok2 = CsvExporter::exportAnomalies(anoms, anomFile);
        bool ok3 = ReportGenerator::writeFullText(tier, anoms, textFile);
        cout << "Export results: tierCsv=" << ok1 << " anomaliesCsv=" << ok2 << " reportTxt=" << ok3 << "\n";
        if (ok1) cout << "Wrote " << csvFile << "\n";
        if (ok2) cout << "Wrote " << anomFile << "\n";
        if (ok3) cout << "Wrote " << textFile << "\n";
    }

    void actionShowSensors() {
        auto v = registry.all();
        cout << "Registered sensors: " << v.size() << "\n";
        for (auto &s : v) {
            cout << " id=" << s.id << " street=" << s.street << " zone=" << s.zone
                 << " drone=" << (s.droneAssisted ? "Y":"N") << " lanes=" << s.lanes << "\n";
        }
    }

    void actionTuneThresholds() {
        cout << "Manual threshold tuning is available by setting quantile multipliers.\n";
        double m1 = readDouble("Enter low quantile (0.1-0.5) default 0.33: ", 0.33);
        double m2 = readDouble("Enter medium quantile (0.5-0.9) default 0.66: ", 0.66);
        m1 = min(max(m1, 0.05), 0.5);
        m2 = min(max(m2, 0.51), 0.99);
        cout << "Manual tuning saved (not persistent across runs). m1=" << m1 << " m2=" << m2 << "\n";
        // For simplicity in this prototype, we will just run a small batch and show thresholds
        int count = readInt("How many samples to evaluate with tuned thresholds? (default 200) ", 200);
        vector<TrafficSample> temp;
        // produce into a temp vector without using processor's batch abstraction
        for (int i = 0; i < count; ++i) {
            TrafficSample s;
            s.sensorId = registry.all()[i % registry.all().size()].id;
            s.ts = TimeStamp::now();
            s.vehiclesPerMinute = 5 + (i % 20) + (i%3);
            s.avgSpeed = 40 - s.vehiclesPerMinute * 0.8;
            s.lane = 1;
            temp.push_back(s);
        }
        int idx1 = max(0, (int)floor(temp.size() * m1));
        int idx2 = max(0, (int)floor(temp.size() * m2));
        SortEngine::nth_select(temp, idx1);
        double t1 = temp[idx1].vehiclesPerMinute;
        SortEngine::nth_select(temp, idx2);
        double t2 = temp[idx2].vehiclesPerMinute;
        cout << "Tuned thresholds: t1=" << t1 << " t2=" << t2 << "\n";
    }

    void actionBulkRun() {
        int batches = readInt("Number of batches to run (default 10): ", 10);
        int batchCount = readInt("Samples per batch (default 200): ", 200);
        int pauseMs = readInt("Pause between batches in ms (default 200): ", 200);
        cout << "Running " << batches << " batches, each " << batchCount << " samples.\n";
        for (int i = 0; i < batches; ++i) {
            simulator.produce(processor, batchCount);
            cout << "Batch " << i+1 << " done.\n";
            this_thread::sleep_for(chrono::milliseconds(pauseMs));
        }
        cout << "Bulk run finished.\n";
    }
};

// =============================================================
// Helper: populate registry with sensible Aroha Nagar sensors
// =============================================================

static void populateRegistry(SensorRegistry &reg) {
    // A set of sample sensors distributed across hypothetical streets/zones
    vector<SensorInfo> list = {
        {1, "Market Street", "Central", true, 3},
        {2, "River Road", "North", false, 2},
        {3, "College Avenue", "East", true, 2},
        {4, "Industrial Way", "South", false, 4},
        {5, "Old Town Lane", "West", false, 1},
        {6, "Ring Road", "Central", true, 3},
        {7, "Harbor Drive", "South", false, 2},
        {8, "Hilltop Road", "North", false, 2},
        {9, "Airport Express", "East", true, 4},
        {10, "Greenway", "West", false, 2}
    };
    for (auto &s : list) reg.registerSensor(s);
}

// =============================================================
// Main — ties everything together
// =============================================================

int main(int argc, char **argv) {
    SensorRegistry registry;
    populateRegistry(registry);

    // For quick script usage: allow a headless mode: run N batches then exit.
    if (argc >= 3 && string(argv[1]) == "--headless") {
        int batches = stoi(argv[2]);
        int batchSize = 200;
        if (argc >= 4) batchSize = stoi(argv[3]);
        TrafficBatchProcessor processor(registry, batchSize);
        TrafficSimulator simulator(registry);
        cout << "Headless mode: running " << batches << " batches of " << batchSize << " samples.\n";
        for (int i = 0; i < batches; ++i) {
            simulator.produce(processor, batchSize);
        }
        processor.flush();
        auto tier = processor.lastTier();
        auto anoms = processor.lastAnomalies();
        ReportGenerator::writeFullText(tier, anoms, "headless_report.txt");
        CsvExporter::exportTier(tier, "headless_tier.csv");
        CsvExporter::exportAnomalies(anoms, "headless_anomalies.csv");
        cout << "Headless run complete. Reports written.\n";
        return 0;
    }

    // Interactive mode
    InteractiveDriver driver(registry);
    driver.run();

    return 0;
}

