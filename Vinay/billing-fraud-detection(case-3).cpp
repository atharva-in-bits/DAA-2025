#include <bits/stdc++.h>
using namespace std;

// ==================== TRIE FOR SKU / BARCODE ====================
struct TrieNode {
    bool isEnd;
    unordered_map<char, TrieNode*> next;
    TrieNode() : isEnd(false) {}
};

class Trie {
public:
    Trie() { root = new TrieNode(); }

    void insert(const string &s) {
        TrieNode *cur = root;
        for (char c : s) {
            if (!cur->next.count(c)) cur->next[c] = new TrieNode();
            cur = cur->next[c];
        }
        cur->isEnd = true;
    }

    // check if full code exists
    bool exists(const string &s) const {
        const TrieNode *cur = root;
        for (char c : s) {
            auto it = cur->next.find(c);
            if (it == cur->next.end()) return false;
            cur = it->second;
        }
        return cur->isEnd;
    }

    // check if given prefix exists in any code
    bool hasPrefix(const string &s) const {
        const TrieNode *cur = root;
        for (char c : s) {
            auto it = cur->next.find(c);
            if (it == cur->next.end()) return false;
            cur = it->second;
        }
        return true;
    }

private:
    TrieNode *root;
};

// ==================== KMP FOR BILL TEXT SEARCH ==================
vector<int> buildLPS(const string &pat) {
    int n = pat.size();
    vector<int> lps(n, 0);
    int len = 0;
    for (int i = 1; i < n; ) {
        if (pat[i] == pat[len]) {
            lps[i++] = ++len;
        } else if (len != 0) {
            len = lps[len - 1];
        } else {
            lps[i++] = 0;
        }
    }
    return lps;
}

bool kmpSearch(const string &text, const string &pat) {
    if (pat.empty()) return true;
    vector<int> lps = buildLPS(pat);
    int i = 0, j = 0;
    while (i < (int)text.size()) {
        if (text[i] == pat[j]) {
            i++; j++;
            if (j == (int)pat.size()) return true;
        } else if (j != 0) {
            j = lps[j - 1];
        } else {
            i++;
        }
    }
    return false;
}

// ==================== BILLING MODELS ============================
struct BillLine {
    string sku;      // product code
    string desc;     // line description text
    int quantity;
    double amount;
};

vector<BillLine> currentBill;                     // array of lines
Trie skuTrie;                                     // valid SKU patterns
unordered_map<string, string> skuToName;          // hash table of valid SKUs
unordered_set<string> blacklistedPatterns;        // suspicious text patterns
unordered_set<string> usedBillIds;                // detect duplicate bills

// ==================== BILL OPS ==============================
void loadMasterSKUs() {
    int n;
    cout << "Enter number of master SKUs: ";
    cin >> n;
    cin.ignore();
    cout << "Enter each SKU and product name (code name):\n";
    for (int i = 0; i < n; ++i) {
        string code, name;
        cin >> code;
        getline(cin, name);
        if (!name.empty() && name[0] == ' ') name.erase(name.begin());
        skuTrie.insert(code);
        skuToName[code] = name;
    }
    cout << "Master SKU table loaded.\n";
}

void loadBlacklistedPatterns() {
    int n;
    cout << "Enter number of suspicious text patterns: ";
    cin >> n;
    cin.ignore();
    cout << "Enter patterns (e.g., FREE, MANUAL_OVERRIDE):\n";
    for (int i = 0; i < n; ++i) {
        string pat;
        getline(cin, pat);
        if (!pat.empty())
            blacklistedPatterns.insert(pat);
    }
}

void enterBill() {
    currentBill.clear();
    int lines;
    cout << "Enter number of bill lines: ";
    cin >> lines;
    cin.ignore();

    cout << "Enter each line as: SKU quantity amount description_text\n";
    cout << "(where description_text is the rest of the line)\n";
    for (int i = 0; i < lines; ++i) {
        BillLine bl;
        cin >> bl.sku >> bl.quantity >> bl.amount;
        getline(cin, bl.desc);
        if (!bl.desc.empty() && bl.desc[0] == ' ') bl.desc.erase(bl.desc.begin());
        currentBill.push_back(bl);
    }
    cout << "Bill captured.\n";
}

// ==================== FRAUD CHECKS ============================
void checkSKUValidity(vector<string> &issues) {
    for (auto &bl : currentBill) {
        if (!skuTrie.exists(bl.sku)) {
            issues.push_back("Unknown or invalid SKU: " + bl.sku);
        }
    }
}

void checkSuspiciousText(vector<string> &issues) {
    for (auto &bl : currentBill) {
        for (const string &pat : blacklistedPatterns) {
            if (kmpSearch(bl.desc, pat)) {
                issues.push_back("Suspicious pattern \"" + pat +
                                 "\" in line with SKU " + bl.sku);
            }
        }
    }
}

void checkBillIdDuplicate(vector<string> &issues) {
    string billId;
    cout << "Enter bill ID/reference: ";
    cin >> billId;

    if (usedBillIds.count(billId)) {
        issues.push_back("Duplicate bill ID detected: " + billId);
    } else {
        usedBillIds.insert(billId);
    }
}

void runFraudAnalysis() {
    if (currentBill.empty()) {
        cout << "No active bill. Enter a bill first.\n";
        return;
    }
    vector<string> issues;

    checkSKUValidity(issues);
    checkSuspiciousText(issues);
    checkBillIdDuplicate(issues);

    if (issues.empty()) {
        cout << "Bill passed all automated checks. No fraud detected.\n";
    } else {
        cout << "Potential issues detected:\n";
        for (auto &s : issues) cout << "- " << s << "\n";
    }
}

void showBill() {
    if (currentBill.empty()) {
        cout << "No active bill.\n";
        return;
    }
    cout << "---- Current Bill ----\n";
    for (auto &bl : currentBill) {
        cout << "SKU: " << bl.sku
             << " | Qty: " << bl.quantity
             << " | Amt: " << bl.amount
             << " | Desc: " << bl.desc << "\n";
    }
}

// ==================== MENU / MAIN ============================
void printMenu() {
    cout << "\n=== Automated Billing Code Matching & Fraud Detection ===\n";
    cout << "1. Load master SKU table (Trie + hash)\n";
    cout << "2. Load suspicious text patterns (for KMP)\n";
    cout << "3. Enter a new bill (array of lines)\n";
    cout << "4. Show current bill\n";
    cout << "5. Run fraud detection on current bill\n";
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
            case 1: loadMasterSKUs(); break;
            case 2: loadBlacklistedPatterns(); break;
            case 3: enterBill(); break;
            case 4: showBill(); break;
            case 5: runFraudAnalysis(); break;
            case 0: cout << "Exiting.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}
