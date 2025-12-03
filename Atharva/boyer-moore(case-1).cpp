#include <bits/stdc++.h>
using namespace std;
static vector<vector<string>> read_csv(const string &path) {
    ifstream in(path);
    if (!in.is_open()) {
        cerr << "Cannot open CSV: " << path << "\n";
        return {};
    }
    vector<vector<string>> rows;
    string line;
    // Read header
    if (!getline(in, line)) return rows;
    vector<string> header;
    {
        string token;
        stringstream ss(line);
        while (getline(ss, token, ',')) header.push_back(token);
    }
    // find index of encoded_stream (case-insensitive)
    int enc_idx = -1;
    for (int i = 0; i < (int)header.size(); ++i) {
        string h = header[i];
        for (auto &c : h) c = (char)tolower(c);
        if (h == "encoded_stream" || h == "encodedstream") { enc_idx = i; break; }
    }
    while (getline(in, line)) {
        vector<string> cols;
        string token;
        string cur;
        bool inquote = false;
        for (char ch : line) {
            if (ch == '"' ) { inquote = !inquote; continue; }
            if (ch == ',' && !inquote) {
                cols.push_back(cur);
                cur.clear();
            } else cur.push_back(ch);
        }
        cols.push_back(cur);
        // If header not found, just push line tokens
        if (enc_idx == -1) rows.push_back(cols);
        else {
            // normalize to ensure enc_idx exists
            if (enc_idx < (int)cols.size()) {
                rows.push_back({cols[enc_idx]});
            } else rows.push_back({""});
        }
    }
    in.close();
    return rows;
}

// KMP prefix function
static vector<int> kmp_prefix(const string &p) {
    int m = p.size();
    vector<int> pi(m);
    pi[0] = 0;
    for (int i = 1; i < m; ++i) {
        int j = pi[i-1];
        while (j > 0 && p[i] != p[j]) j = pi[j-1];
        if (p[i] == p[j]) ++j;
        pi[i] = j;
    }
    return pi;
}

// KMP search; returns positions (0-based)
static vector<int> kmp_search(const string &s, const string &p) {
    vector<int> res;
    if (p.empty() || s.size() < p.size()) return res;
    auto pi = kmp_prefix(p);
    int j = 0;
    for (int i = 0; i < (int)s.size(); ++i) {
        while (j > 0 && s[i] != p[j]) j = pi[j-1];
        if (s[i] == p[j]) ++j;
        if (j == (int)p.size()) {
            res.push_back(i - j + 1);
            j = pi[j-1];
        }
    }
    return res;
}

// Build bad character table for ASCII (256)
static array<int, 256> build_badchar(const string &p) {
    array<int,256> table;
    table.fill(-1);
    for (int i = 0; i < (int)p.size(); ++i) table[(unsigned char)p[i]] = i;
    return table;
}

// Z algorithm to build good suffix shift
static vector<int> z_array(const string &s) {
    int n = s.size();
    vector<int> z(n);
    int l=0,r=0;
    for (int i=1;i<n;i++){
        if (i<=r) z[i]=min(r-i+1,z[i-l]);
        while (i+z[i]<n && s[z[i]]==s[i+z[i]]) ++z[i];
        if (i+z[i]-1>r) l=i,r=i+z[i]-1;
    }
    return z;
}

static vector<int> build_good_suffix(const string &p) {
    int m = p.size();
    vector<int> suff(m);
    // compute suffixes using reverse z
    string rev = p; reverse(rev.begin(), rev.end());
    auto z = z_array(rev);
    for (int i = 0; i < m; ++i) {
        suff[i] = 0;
    }
    for (int i = 0; i < m; ++i) {
        int j = m - z[i];
        if (z[i] > 0 && j >= 0 && j < m) {
            suff[m - z[i]] = z[i];
        }
    }
    // convert suff to shift table
    vector<int> shift(m+1, m);
    int last = m;
    for (int i = m-1; i>=0; --i) {
        if (suff[i] == i+1) last = i+1;
        shift[m-1-i] = last + (m-1-i);
    }
    // note: this is a simplified good-suffix; BM variants exist
    // fallback to m if no better shift found
    vector<int> res(m);
    for (int i = 0; i < m; ++i) res[i] = shift[i];
    return res;
}

static vector<int> boyer_moore_search(const string &text, const string &pat) {
    vector<int> res;
    int n = (int)text.size(), m = (int)pat.size();
    if (m == 0 || n < m) return res;
    auto bad = build_badchar(pat);
    auto good = build_good_suffix(pat);
    int s = 0;
    while (s <= n - m) {
        int j = m - 1;
        while (j >= 0 && pat[j] == text[s+j]) --j;
        if (j < 0) {
            res.push_back(s);
            s += (m - bad[(unsigned char)text[s+m]] >= 1) ? m - bad[(unsigned char)text[s+m]] : 1;
        } else {
            int bc_shift = j - bad[(unsigned char)text[s+j]];
            int gs_shift = 1;
            if (j < m-1) {
                gs_shift = good[m-1-j];
            }
            s += max(1, max(bc_shift, gs_shift));
        }
    }
    return res;
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <csv-path> <pattern> [--kmp-fallback]\n";
        return 1;
    }
    string csv = argv[1];
    string pattern = argv[2];
    bool kmp_fallback = false;
    for (int i=3;i<argc;i++) if (string(argv[i]) == "--kmp-fallback") kmp_fallback = true;

    auto rows = read_csv(csv);
    if (rows.empty()) { cerr << "No rows loaded or file missing\n"; return 1; }

    cout << "Loaded " << rows.size() << " records. Searching for pattern: " << pattern << "\n";
    for (size_t i = 0; i < rows.size(); ++i) {
        string s = rows[i].empty() ? string() : rows[i][0];
        if (s.empty()) continue;
        auto found = boyer_moore_search(s, pattern);
        if (found.empty() && kmp_fallback) found = kmp_search(s, pattern);
        if (!found.empty()) {
            cout << "Row " << (i+1) << " matches at positions: ";
            for (auto pos : found) cout << pos << " ";
            cout << "  [stream=" << s << "]\n";
        }
    }
    return 0;
}
