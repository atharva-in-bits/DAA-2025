#include <bits/stdc++.h>
using namespace std;

// ===================== TRIE FOR TYPE TAGGING =====================
struct TrieNode {
    bool isEnd;
    string tag;                          // "GARBAGE", "WATER", etc.
    unordered_map<char, TrieNode*> nxt;
    TrieNode() : isEnd(false), tag("") {}
};

class Trie {
public:
    Trie() { root = new TrieNode(); }

    void insert(const string &word, const string &tag) {
        TrieNode *cur = root;
        for (char c : word) {
            c = tolower(c);
            if (!cur->nxt.count(c)) cur->nxt[c] = new TrieNode();
            cur = cur->nxt[c];
        }
        cur->isEnd = true;
        cur->tag = tag;
    }

    // classify first keyword in description; fallback "OTHER"
    string classify(const string &desc) const {
        string w;
        for (char c : desc) {
            if (isalpha(c)) {
                w.push_back(tolower(c));
            } else {
                if (!w.empty()) {
                    string t = findWord(w);
                    if (t != "") return t;
                    w.clear();
                }
            }
        }
        if (!w.empty()) {
            string t = findWord(w);
            if (t != "") return t;
        }
        return "OTHER";
    }

private:
    TrieNode *root;

    string findWord(const string &w) const {
        const TrieNode *cur = root;
        for (char c : w) {
            auto it = cur->nxt.find(c);
            if (it == cur->nxt.end()) return "";
            cur = it->second;
        }
        return cur->isEnd ? cur->tag : "";
    }
};

// ===================== GRAPH + DIJKSTRA ==========================
const int MAX_NODES = 100;
const int INF = 1e9;

int numWards;
vector<pair<int,int>> adj[MAX_NODES + 1];    // (neighbor, travelTime)

void addRoad(int u, int v, int w) {
    if (u < 1 || v < 1 || u > numWards || v > numWards) {
        cout << "Invalid road endpoints.\n";
        return;
    }
    adj[u].push_back({v, w});
    adj[v].push_back({u, w});
}

vector<int> dijkstra(int src) {
    vector<int> dist(numWards + 1, INF);
    priority_queue<pair<int,int>, vector<pair<int,int>>,
                   greater<pair<int,int>>> pq;
    dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d != dist[u]) continue;
        for (auto &e : adj[u]) {
            int v = e.first, w = e.second;
            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

// ===================== COMPLAINT / TEAM MODELS ===================
struct Complaint {
    int id;
    int ward;              // ward id
    string desc;
    string typeTag;        // GARBAGE / WATER / ROAD / OTHER
    int severity;          // 1–5
    int waitTime;          // minutes since logged (for priority)
};

struct Team {
    int id;
    string name;
    int wardBase;
    string typeHandled;    // e.g. "GARBAGE"
};

// priority for complaints: higher severity, then longer wait
struct ComplaintPriority {
    bool operator()(const Complaint &a, const Complaint &b) const {
        if (a.severity != b.severity)
            return a.severity < b.severity;      // max-heap by severity
        return a.waitTime < b.waitTime;          // then by wait time
    }
};

// global storage
unordered_map<int, Complaint> complaintMap;      // id -> complaint
unordered_map<int, Team> teamMap;               // id -> team
unordered_map<int,
    priority_queue<Complaint, vector<Complaint>, ComplaintPriority>>
    teamQueues;                                  // teamId -> heap

Trie typeTrie;
int nextComplaintId = 1;

// ===================== SETUP FUNCTIONS ===========================
void setupCityGraph() {
    cout << "Enter number of wards (nodes, <= " << MAX_NODES << "): ";
    cin >> numWards;
    if (numWards < 1 || numWards > MAX_NODES) {
        cout << "Invalid. Setting numWards = 10.\n";
        numWards = 10;
    }
    int m;
    cout << "Enter number of roads (edges): ";
    cin >> m;
    for (int i = 1; i <= numWards; ++i) adj[i].clear();
    cout << "Enter each road as: u v travel_time\n";
    for (int i = 0; i < m; ++i) {
        int u, v, w;
        cin >> u >> v >> w;
        addRoad(u, v, w);
    }
}

void setupComplaintTypes() {
    // simple fixed dictionary (you can extend)
    typeTrie.insert("garbage", "GARBAGE");
    typeTrie.insert("waste",   "GARBAGE");
    typeTrie.insert("water",   "WATER");
    typeTrie.insert("leak",    "WATER");
    typeTrie.insert("road",    "ROAD");
    typeTrie.insert("pothole", "ROAD");
    cout << "Complaint type dictionary loaded into Trie.\n";
}

void addTeam() {
    Team t;
    cout << "Enter team id: ";
    cin >> t.id;
    cin.ignore();
    cout << "Enter team name: ";
    getline(cin, t.name);
    cout << "Enter base ward id: ";
    cin >> t.wardBase;
    cin.ignore();
    cout << "Enter type handled (GARBAGE/WATER/ROAD/OTHER): ";
    getline(cin, t.typeHandled);

    teamMap[t.id] = t;
    cout << "Team added.\n";
}

// ===================== COMPLAINT OPS ============================
void logComplaint() {
    Complaint c;
    c.id = nextComplaintId++;
    cout << "Enter ward id where issue is reported: ";
    cin >> c.ward;
    cin.ignore();
    cout << "Enter description: ";
    getline(cin, c.desc);
    cout << "Enter severity (1-5): ";
    cin >> c.severity;
    cout << "Enter waiting time (minutes since issue): ";
    cin >> c.waitTime;

    c.typeTag = typeTrie.classify(c.desc);
    complaintMap[c.id] = c;

    cout << "Logged complaint ID " << c.id
         << " | Type: " << c.typeTag << "\n";
}

// choose nearest compatible team using Dijkstra
int findBestTeamFor(const Complaint &c) {
    if (teamMap.empty()) return -1;

    vector<int> dist = dijkstra(c.ward);
    int bestTeamId = -1;
    int bestCost   = INF;

    for (auto &p : teamMap) {
        const Team &t = p.second;
        // team can handle complaint if type matches or handles "OTHER"
        if (t.typeHandled != c.typeTag && t.typeHandled != "OTHER") continue;
        int d = dist[t.wardBase];
        if (d < bestCost) {
            bestCost = d;
            bestTeamId = t.id;
        }
    }
    return bestTeamId;
}

void routeComplaints() {
    if (complaintMap.empty()) {
        cout << "No complaints logged.\n";
        return;
    }
    if (teamMap.empty()) {
        cout << "No teams configured.\n";
        return;
    }

    // clear old queues
    teamQueues.clear();

    cout << "Routing complaints to nearest capable teams...\n";
    for (auto &p : complaintMap) {
        Complaint c = p.second;
        int tid = findBestTeamFor(c);
        if (tid == -1) {
            cout << "Complaint " << c.id << " (type " << c.typeTag
                 << ") has no suitable team.\n";
        } else {
            teamQueues[tid].push(c);
        }
    }

    cout << "Routing complete.\n";
}

void showTeamQueues() {
    if (teamQueues.empty()) {
        cout << "No routed complaints yet. Run routing first.\n";
        return;
    }
    for (auto &p : teamQueues) {
        int tid = p.first;
        Team &t = teamMap[tid];
        cout << "=== Team " << t.id << " (" << t.name
             << ") handling " << t.typeHandled << " ===\n";
        auto pq = p.second;    // copy to display
        if (pq.empty()) {
            cout << "  No complaints assigned.\n";
        }
        while (!pq.empty()) {
            Complaint c = pq.top(); pq.pop();
            cout << "  Complaint " << c.id
                 << " | Ward " << c.ward
                 << " | Type " << c.typeTag
                 << " | Sev " << c.severity
                 << " | Wait " << c.waitTime
                 << " | Desc: " << c.desc << "\n";
        }
    }
}

// ===================== MENU / MAIN ============================
void printMenu() {
    cout << "\n=== Government Complaint Routing System ===\n";
    cout << "1. Setup city ward graph (roads)\n";
    cout << "2. Load complaint type dictionary (Trie)\n";
    cout << "3. Add response team\n";
    cout << "4. Log new complaint\n";
    cout << "5. Route all complaints to nearest capable teams (Dijkstra + heap)\n";
    cout << "6. Show per-team priority queues\n";
    cout << "0. Exit\n";
    cout << "Choice: ";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int choice;
    do {
        printMenu();
        cin >> choice;
        switch (choice) {
            case 1: setupCityGraph(); break;
            case 2: setupComplaintTypes(); break;
            case 3: addTeam(); break;
            case 4: logComplaint(); break;
            case 5: routeComplaints(); break;
            case 6: showTeamQueues(); break;
            case 0: cout << "Exiting.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}
