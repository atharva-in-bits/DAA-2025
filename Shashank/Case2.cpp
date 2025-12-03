
// EV Charging Network – Slot Scheduling Demo
// Purpose: Find nearest charger and manage time-slot reservations.
//
// Algorithms / DS used:
// - Segment Tree: manage availability for time slots (range updates/queries)
// - Fenwick Tree (Binary Indexed Tree): alternative for prefix sums of load
// - Min-Heap (priority_queue): next-free / nearest charger
// - Hashing (unordered_map): reservation lookup by ID
// - Arrays/Structures: station & charger data

#include <bits/stdc++.h>
using namespace std;

// -------------------- Basic Structures --------------------
// One charger with a location (1D for simplicity) and an ID.
struct Charger {
    int id;
    int stationId;
    double position; // coordinate on a line or distance index
};

// Station with list of chargers.
struct Station {
    int id;
    double position;
    vector<int> chargerIds; // references into chargers array
};

// -------------------- Segment Tree --------------------
// We keep an array timeSlots[0..T-1]:
//  - 0 = free, 1 = reserved (simple model)
// Segment Tree supports:
//  - build: from initial availability
//  - rangeQuery: number of free slots in [l, r]
//  - pointUpdate: mark one slot as reserved / free.

class SegmentTree {
public:
    int n;
    vector<int> tree; // store sum of reserved flags in range

    SegmentTree(int size = 0) { init(size); }

    void init(int size) {
        n = size;
        tree.assign(4 * n, 0);
    }

    void build(const vector<int>& a, int v, int tl, int tr) {
        if (tl == tr) {
            tree[v] = a[tl];
        } else {
            int tm = (tl + tr) / 2;
            build(a, v * 2, tl, tm);
            build(a, v * 2 + 1, tm + 1, tr);
            tree[v] = tree[v * 2] + tree[v * 2 + 1];
        }
    }

    void build(const vector<int>& a) {
        if (n > 0) build(a, 1, 0, n - 1);
    }

    // Update single slot idx to value val (0 free, 1 reserved)
    void update(int v, int tl, int tr, int idx, int val) {
        if (tl == tr) {
            tree[v] = val;
        } else {
            int tm = (tl + tr) / 2;
            if (idx <= tm)
                update(v * 2, tl, tm, idx, val);
            else
                update(v * 2 + 1, tm + 1, tr, idx, val);
            tree[v] = tree[v * 2] + tree[v * 2 + 1];
        }
    }

    void update(int idx, int val) {
        update(1, 0, n - 1, idx, val);
    }

    // Query how many reserved slots in [l, r]
    int query(int v, int tl, int tr, int l, int r) {
        if (l > r) return 0;
        if (l == tl && r == tr) return tree[v];
        int tm = (tl + tr) / 2;
        return query(v * 2, tl, tm, l, min(r, tm))
             + query(v * 2 + 1, tm + 1, tr, max(l, tm + 1), r);
    }

    int query(int l, int r) {
        return query(1, 0, n - 1, l, r);
    }

    // Helper: is there any free slot in [l, r]?
    bool hasFreeSlot(int l, int r) {
        int reservedCount = query(l, r);
        int total = r - l + 1;
        return reservedCount < total;
    }
};

/*
Segment Tree comments (EV-charging context):
- Represents time slots for one charger or station.
- Each leaf is a time slot; value 0/1 shows free or reserved.
- Supports fast queries like “is any slot free between 10:00–11:00?” and updates on reservation/cancellation.
*/

// -------------------- Fenwick Tree (Binary Indexed Tree) --------------------
// Fenwick Tree maintains prefix sums of load, e.g. total number of cars
// scheduled up to a certain time index, to check grid or station load.

class FenwickTree {
public:
    int n;
    vector<int> bit; // 1-indexed

    FenwickTree(int size = 0) { init(size); }

    void init(int size) {
        n = size;
        bit.assign(n + 1, 0);
    }

    // Add value at index idx (0-based externally)
    void update(int idx, int delta) {
        for (int i = idx + 1; i <= n; i += i & -i) {
            bit[i] += delta;
        }
    }

    // Prefix sum [0..idx]
    int prefixSum(int idx) {
        int res = 0;
        for (int i = idx + 1; i > 0; i -= i & -i) {
            res += bit[i];
        }
        return res;
    }

    // Range sum [l..r]
    int rangeSum(int l, int r) {
        if (l > r) return 0;
        return prefixSum(r) - (l ? prefixSum(l - 1) : 0);
    }
};

/*
Fenwick Tree comments (EV-charging context):
- Tracks cumulative load: how many vehicles are charging up to a given time.
- Fast updates when new reservations are added or removed.
- Helps enforce grid/feeder capacity constraints across all chargers.
*/

// -------------------- Min-Heap for Next-Free / Nearest Charger --------------------
// We use a min-heap keyed by (nextAvailableTime, distanceToUser).

struct ChargerState {
    int chargerId;
    int nextFreeTime;  // earliest free time index
    double distance;   // distance from user

    bool operator>(const ChargerState& other) const {
        if (nextFreeTime != other.nextFreeTime)
            return nextFreeTime > other.nextFreeTime; // earlier is better
        return distance > other.distance;             // closer is better
    }
};

void demoChargerHeap() {
    priority_queue<ChargerState, vector<ChargerState>, greater<ChargerState>> pq;

    // Example charger states
    pq.push({1, 5, 2.0});  // charger 1 free at time 5, distance 2.0
    pq.push({2, 3, 4.5});  // charger 2 free at time 3, distance 4.5
    pq.push({3, 3, 1.2});  // charger 3 free at time 3, distance 1.2

    cout << "Selecting chargers by earliest free time, then distance:\n";
    while (!pq.empty()) {
        ChargerState cs = pq.top();
        pq.pop();
        cout << "  Charger " << cs.chargerId
             << " (nextFreeTime=" << cs.nextFreeTime
             << ", distance=" << cs.distance << ")\n";
    }
}

/*
Heap comments (EV-charging context):
- Prioritizes chargers that free up earlier, and among them, those that are nearer.
- Used by the scheduler to quickly pick the “best” charger for a new EV arrival.
- Automatically reorders as nextFreeTime/distances change and states are pushed.
*/

// -------------------- Hashing for Reservation Lookup --------------------
// Stores reservation records keyed by a reservation ID.

struct Reservation {
    int id;
    int chargerId;
    int startSlot;
    int endSlot;
};

void demoReservationHashing() {
    unordered_map<int, Reservation> reservations;

    // Create some sample reservations
    reservations[1001] = {1001, 1, 10, 12};
    reservations[1002] = {1002, 2, 5, 7};
    reservations[1003] = {1003, 3, 20, 22};

    cout << "Listing reservations:\n";
    for (auto& p : reservations) {
        const Reservation& r = p.second;
        cout << "  ResID " << r.id << ": charger " << r.chargerId
             << " slots [" << r.startSlot << "," << r.endSlot << "]\n";
    }

    // Lookup by ID
    int queryId = 1002;
    auto it = reservations.find(queryId);
    if (it != reservations.end()) {
        const Reservation& r = it->second;
        cout << "Lookup: Found reservation " << r.id
             << " on charger " << r.chargerId << "\n";
    } else {
        cout << "Lookup: Reservation " << queryId << " not found\n";
    }
}

/*
Hashing comments (EV-charging context):
- unordered_map gives O(1) average lookup by reservation ID.
- Used to modify or cancel a reservation quickly when a user updates their booking.
- Also supports queries like “show my current bookings” if keyed by user or vehicle ID.
*/

// -------------------- Arrays/Structures to Store Station Data --------------------
void demoStationData() {
    // Example stations and chargers array
    vector<Charger> chargers = {
        {1, 0, 0.0},
        {2, 0, 0.1},
        {3, 1, 5.0},
        {4, 1, 5.2}
    };

    vector<Station> stations(2);
    stations[0].id = 0;
    stations[0].position = 0.0;
    stations[0].chargerIds = {1, 2};

    stations[1].id = 1;
    stations[1].position = 5.0;
    stations[1].chargerIds = {3, 4};

    cout << "Station and charger layout:\n";
    for (const Station& s : stations) {
        cout << "  Station " << s.id << " at position " << s.position << " has chargers: ";
        for (int cid : s.chargerIds) cout << cid << " ";
        cout << "\n";
    }
}

/*
Arrays/Structures comments (EV-charging context):
- Arrays/vectors store stations and chargers with their coordinates and relationships.
- Forms the base data for computing nearest station/charger given an EV’s location.
- Simple structures keep code clear and efficient for lookup and iteration.
*/

// -------------------- Putting it together in main --------------------
int main() {
    // Example timeline of T time slots (e.g., every 15 minutes).
    int T = 24; // small example: 24 slots
    vector<int> initialSlots(T, 0); // all free initially

    // Segment tree for one charger’s daily schedule.
    SegmentTree seg(T);
    seg.build(initialSlots);

    // Make a few reservations on this charger:
    seg.update(3, 1);  // reserve slot 3
    seg.update(4, 1);  // reserve slot 4
    seg.update(10, 1); // reserve slot 10

    cout << "Segment Tree: reserved slots in [0, 5] = "
         << seg.query(0, 5) << "\n";
    cout << "Any free slot in [3, 4]? "
         << (seg.hasFreeSlot(3, 4) ? "Yes" : "No") << "\n";
    cout << "Any free slot in [0, 2]? "
         << (seg.hasFreeSlot(0, 2) ? "Yes" : "No") << "\n\n";

    // Fenwick tree to track total number of cars charging over time.
    FenwickTree fw(T);
    fw.update(3, 1);   // one car in slot 3
    fw.update(4, 1);   // one car in slot 4
    fw.update(10, 1);  // one car in slot 10

    cout << "Fenwick: total cars charging up to slot 10 = "
         << fw.prefixSum(10) << "\n\n";

    // Station & charger layout
    demoStationData();
    cout << "\n";

    // Min-heap for choosing next-free / nearest charger
    demoChargerHeap();
    cout << "\n";

    // Hash map for reservation lookup
    demoReservationHashing();
    cout << "\n";

    return 0;
}
