#include <bits/stdc++.h>
using namespace std;

struct Record {
    string timestamp;
    int location_id;
    int drone_count;
    int vehicle_count;
    double density;
    int index;
};

static vector<vector<string>> read_csv_rows(const string &path){
    ifstream in(path);
    vector<vector<string>> rows;
    if(!in.is_open()) return rows;
    string line;
    if(!getline(in,line)) return rows;
    while(getline(in,line)){
        vector<string> cols;
        string cur; bool inq=false;
        for(char c: line){
            if(c=='"'){ inq = !inq; continue; }
            if(c==',' && !inq){ cols.push_back(cur); cur.clear(); } else cur.push_back(c);
        }
        cols.push_back(cur);
        rows.push_back(cols);
    }
    return rows;
}

static vector<Record> load_records(const string &path){
    auto rows = read_csv_rows(path);
    vector<Record> out; out.reserve(rows.size());
    int idx=0;
    for(auto &r: rows){
        if(r.size() < 5) continue;
        Record rec;
        rec.timestamp = r[0];
        rec.location_id = stoi(r[1]);
        rec.drone_count = stoi(r[2]);
        rec.vehicle_count = stoi(r[3]);
        rec.density = stod(r[4]);
        rec.index = idx++;
        out.push_back(rec);
    }
    return out;
}

struct Timer {
    chrono::high_resolution_clock::time_point s;
    void start(){ s = chrono::high_resolution_clock::now(); }
    long long ms(){ auto e = chrono::high_resolution_clock::now(); return chrono::duration_cast<chrono::milliseconds>(e-s).count(); }
};

static void swap_rec(Record &a, Record &b){ swap(a,b); }

static int median_of_three(vector<Record> &a, int i, int j, int k){
    double ai = a[i].density, aj = a[j].density, ak = a[k].density;
    if(ai < aj){
        if(aj < ak) return j;
        if(ai < ak) return k;
        return i;
    } else {
        if(ai < ak) return i;
        if(aj < ak) return k;
        return j;
    }
}

static void insertion_sort(vector<Record> &a, int l, int r){
    for(int i=l+1;i<=r;++i){
        Record key = a[i];
        int j=i-1;
        while(j>=l && a[j].density > key.density){ a[j+1]=a[j]; --j; }
        a[j+1]=key;
    }
}

static int partition_hoare(vector<Record> &a, int l, int r, double pivot){
    int i = l-1, j = r+1;
    while(true){
        do{ ++i; } while(a[i].density < pivot);
        do{ --j; } while(a[j].density > pivot);
        if(i>=j) return j;
        swap_rec(a[i], a[j]);
    }
}

static void quicksort_inplace(vector<Record> &a, int l, int r){
    while(l < r){
        if(r-l < 24){ insertion_sort(a,l,r); return; }
        int m = l + ((r-l)>>1);
        int med = median_of_three(a, l, m, r);
        swap_rec(a[med], a[r]);
        double pivot = a[r].density;
        int p = partition_hoare(a, l, r, pivot);
        if(p - l < r - (p+1)){
            quicksort_inplace(a, l, p);
            l = p+1;
        } else {
            quicksort_inplace(a, p+1, r);
            r = p;
        }
    }
}

struct Fenwick {
    int n;
    vector<double> bit;
    Fenwick(): n(0) {}
    Fenwick(int n_): n(n_), bit(n_+1, 0.0) {}
    void init(int n_){ n = n_; bit.assign(n+1, 0.0); }
    void add(int i, double v){ for(++i;i<=n;i+=i&-i) bit[i]+=v; }
    double sum(int i){ double s=0; for(++i;i>0;i-=i&-i) s+=bit[i]; return s; }
    double range(int l,int r){ if(r<l) return 0.0; return sum(r) - (l?sum(l-1):0.0); }
};

struct SlidingWindow {
    deque<double> dq;
    double sum = 0.0;
    void push(double v){ dq.push_back(v); sum += v; }
    void pop(){ if(!dq.empty()){ sum -= dq.front(); dq.pop_front(); } }
    double mean() const { if(dq.empty()) return 0.0; return sum / (double)dq.size(); }
    int size() const { return (int)dq.size(); }
};

struct Anomaly {
    int index;
    double value;
    double zscore;
};

static unordered_map<int, vector<Record>> group_by_location(const vector<Record> &rows){
    unordered_map<int, vector<Record>> mp;
    for(auto &r: rows) mp[r.location_id].push_back(r);
    return mp;
}

static unordered_map<int, Fenwick> build_fenwicks(const unordered_map<int, vector<Record>> &mp){
    unordered_map<int, Fenwick> ret;
    for(auto &kv: mp){
        int loc = kv.first;
        int m = (int)kv.second.size();
        Fenwick f(m);
        for(int i=0;i<m;++i) f.add(i, kv.second[i].density);
        ret[loc] = move(f);
    }
    return ret;
}

static vector<Anomaly> detect_anomalies_zscore(const vector<Record> &series, int window, double thresh){
    vector<Anomaly> out;
    SlidingWindow sw;
    for(size_t i=0;i<series.size();++i){
        double v = series[i].density;
        sw.push(v);
        if(sw.size() > window) sw.pop();
        if(sw.size() < 5) continue;
        int n = sw.size();
        double mean = sw.mean();
        double var = 0;
        for(auto x: sw.dq) { double d = x - mean; var += d*d; }
        var /= n;
        double sd = sqrt(var);
        if(sd <= 1e-9) continue;
        double z = (v - mean) / sd;
        if(fabs(z) >= thresh) out.push_back({(int)i, v, z});
    }
    return out;
}

static vector<pair<int,int>> compute_tiers(const vector<Record> &sorted, double low_quantile=0.33, double high_quantile=0.66){
    int n = (int)sorted.size();
    int low_idx = max(0, (int)floor(low_quantile * n) - 1);
    int high_idx = min(n-1, (int)floor(high_quantile * n) - 1);
    double low_thr = sorted[low_idx].density;
    double high_thr = sorted[high_idx].density;
    vector<pair<int,int>> tiers;
    for(auto &r: sorted){
        int tier = (r.density <= low_thr) ? 0 : (r.density <= high_thr ? 1 : 2);
        tiers.push_back({r.index, tier});
    }
    return tiers;
}

static void ascii_histogram(const vector<Record> &rows, int buckets=40){
    double mn = 1e18, mx = -1e18;
    for(auto &r: rows){ mn = min(mn, r.density); mx = max(mx, r.density); }
    if(mn >= mx){ cout<<"flat\n"; return; }
    vector<int> cnt(buckets,0);
    for(auto &r: rows){
        int bi = (int)((r.density - mn) / (mx - mn) * (buckets-1));
        bi = max(0,min(buckets-1,bi));
        cnt[bi]++; 
    }
    int maxc = *max_element(cnt.begin(), cnt.end());
    for(int i=0;i<buckets;++i){
        int w = maxc? (cnt[i]*50/maxc) : 0;
        cout<<setw(3)<<i<<": "<<string(w,'#')<<" ("<<cnt[i]<<")\n";
    }
}

static vector<Record> sample_uniform(const vector<Record> &rows, int k){
    if(k <= 0 || rows.empty()) return {};
    mt19937_64 rng((uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count());
    vector<Record> out;
    int n = rows.size();
    if(k >= n) return rows;
    unordered_set<int> sel;
    uniform_int_distribution<int> d(0,n-1);
    while((int)sel.size() < k) sel.insert(d(rng));
    for(auto idx: sel) out.push_back(rows[idx]);
    return out;
}

static vector<Record> sample_stratified(const vector<Record> &rows, int k){
    if(rows.empty()) return {};
    vector<Record> sorted = rows;
    quicksort_inplace(sorted, 0, (int)sorted.size()-1);
    int n = sorted.size();
    vector<Record> out;
    for(int i=0;i<k;++i){
        int pos = (int)floor((double)i * n / k);
        out.push_back(sorted[pos]);
    }
    return out;
}

static void export_csv_records(const string &path, const vector<Record> &rows){
    ofstream out(path);
    out<<"timestamp,location_id,drone_count,vehicle_count,density,index\n";
    for(auto &r: rows) out<<r.timestamp<<","<<r.location_id<<","<<r.drone_count<<","<<r.vehicle_count<<","<<r.density<<","<<r.index<<"\n";
    out.close();
}

static void build_index_per_location(const unordered_map<int, vector<Record>> &mp, unordered_map<int, vector<int>> &idxMap){
    idxMap.clear();
    for(auto &kv: mp){
        int loc = kv.first;
        const auto &vec = kv.second;
        vector<int> ids; ids.reserve(vec.size());
        for(auto &r: vec) ids.push_back(r.index);
        idxMap[loc] = move(ids);
    }
}

static vector<pair<int,double>> running_average_per_loc(const unordered_map<int, vector<Record>> &mp, int window){
    vector<pair<int,double>> out;
    for(auto &kv: mp){
        int loc = kv.first;
        const auto &v = kv.second;
        deque<double> dq; double sum=0;
        for(size_t i=0;i<v.size();++i){
            dq.push_back(v[i].density); sum += v[i].density;
            if(dq.size() > (size_t)window){ sum -= dq.front(); dq.pop_front(); }
        }
        double avg = dq.empty()?0.0:sum/dq.size();
        out.push_back({loc, avg});
    }
    return out;
}

static void multi_threaded_sort(vector<Record> &data, int threads){
    if(threads <= 1 || data.size() < 20000){ quicksort_inplace(data, 0, (int)data.size()-1); return; }
    int n = data.size();
    int chunk = (n + threads - 1) / threads;
    vector<thread> th;
    vector<int> L, R;
    for(int t=0;t<threads;++t){
        int l = t*chunk;
        int r = min(n-1, (t+1)*chunk - 1);
        if(l > r) break;
        L.push_back(l); R.push_back(r);
        th.emplace_back([&data,l,r](){ quicksort_inplace(data, l, r); });
    }
    for(auto &tt: th) tt.join();
    vector<Record> tmp;
    tmp.reserve(n);
    vector<int> ptr(L.size(), 0);
    for(size_t i=0;i<L.size();++i) ptr[i] = L[i];
    auto get_val = [&](int p)->double{ return data[p].density; };
    while(true){
        int who=-1; double best=0;
        for(size_t i=0;i<L.size();++i){
            if(ptr[i] <= R[i]){
                double val = get_val(ptr[i]);
                if(who==-1 || val < best){ who = (int)i; best = val; }
            }
        }
        if(who==-1) break;
        tmp.push_back(data[ptr[who]++]);
    }
    data.swap(tmp);
}

static void print_help(){
    cout<<"Commands:\ncount\nsummary\nhist\nsort [threads]\nsample u k\nsample s k\ntiers\nbuildfenwick\nrolling loc idx k\nanomaly loc window thresh\nexport out.csv\nbench sort iters threads\nstress n\nexit\n";
}

int main(int argc,char**argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(argc < 2){ cerr<<"Usage: "<<argv[0]<<" <csv-path>\n"; return 1; }
    string csv = argv[1];
    auto rows = load_records(csv);
    if(rows.empty()){ cerr<<"no data\n"; return 1; }
    cout<<"Loaded "<<rows.size()<<" records\n";
    unordered_map<int, vector<Record>> byloc = group_by_location(rows);
    unordered_map<int, Fenwick> fenw = build_fenwicks(byloc);
    unordered_map<int, vector<int>> idxMap;
    build_index_per_location(byloc, idxMap);
    vector<Record> copyrows = rows;
    print_help();
    string line;
    while(true){
        cout<<"> ";
        if(!getline(cin,line)) break;
        if(line.empty()) continue;
        stringstream ss(line);
        string cmd; ss>>cmd;
        if(cmd=="exit"||cmd=="quit") break;
        if(cmd=="count"){ cout<<"records="<<rows.size()<<" locations="<<byloc.size()<<"\n"; continue; }
        if(cmd=="summary"){
            double mn=1e18,mx=-1e18,mean=0; for(auto &r: rows){ mn=min(mn,r.density); mx=max(mx,r.density); mean+=r.density; }
            mean /= rows.size();
            cout<<"min="<<mn<<" mean="<<mean<<" max="<<mx<<"\n";
            continue;
        }
        if(cmd=="hist"){ ascii_histogram(rows); continue; }
        if(cmd=="sort"){
            int threads=1; ss>>threads;
            Timer t; t.start();
            multi_threaded_sort(copyrows, threads);
            cout<<"sorted in "<<t.ms()<<" ms\n";
            continue;
        }
        if(cmd=="sample"){
            string mode; ss>>mode;
            if(mode=="u"){ int k; ss>>k; auto s = sample_uniform(rows, k); export_csv_records("sampled_uniform.csv", s); cout<<"wrote sampled_uniform.csv\n"; continue; }
            if(mode=="s"){ int k; ss>>k; auto s = sample_stratified(rows, k); export_csv_records("sampled_stratified.csv", s); cout<<"wrote sampled_stratified.csv\n"; continue; }
            cout<<"unknown sample mode\n";
            continue;
        }
        if(cmd=="tiers"){
            vector<Record> tmp = rows;
            quicksort_inplace(tmp, 0, (int)tmp.size()-1);
            auto t = compute_tiers(tmp, 0.33, 0.66);
            cout<<"index,tier\n";
            for(auto &p: t) cout<<p.first<<","<<p.second<<"\n";
            continue;
        }
        if(cmd=="buildfenwick"){
            fenw = build_fenwicks(byloc);
            cout<<"built fenwick for "<<fenw.size()<<" locations\n";
            continue;
        }
        if(cmd=="rolling"){
            int loc; int idx; int k; ss>>loc>>idx>>k;
            if(fenw.find(loc)==fenw.end()){ cout<<"no loc\n"; continue; }
            auto &f = fenw[loc];
            int n = f.n;
            if(idx < 0) idx = n-1;
            int l = max(0, idx - k + 1), r = min(n-1, idx);
            double s = f.range(l,r);
            cout<<"loc="<<loc<<" sum="<<s<<"\n";
            continue;
        }
        if(cmd=="anomaly"){
            int loc; int window; double thresh; ss>>loc>>window>>thresh;
            if(byloc.find(loc)==byloc.end()){ cout<<"no loc\n"; continue; }
            auto &series = byloc[loc];
            auto hits = detect_anomalies_zscore(series, window, thresh);
            for(auto &h: hits) cout<<h.index<<","<<h.value<<","<<h.zscore<<"\n";
            continue;
        }
        if(cmd=="export"){
            string out; ss>>out;
            if(out.empty()) { cout<<"export <out.csv>\n"; continue; }
            export_csv_records(out, rows);
            cout<<"wrote "<<out<<"\n";
            continue;
        }
        if(cmd=="bench"){
            string target; ss>>target;
            if(target=="sort"){ int iters=1; int threads=1; ss>>iters>>threads;
                Timer t; t.start();
                for(int i=0;i<iters;++i){ vector<Record> tmp = rows; multi_threaded_sort(tmp, threads); }
                cout<<"bench sort total "<<t.ms()<<" ms\n"; continue;
            }
            cout<<"unknown bench\n";
            continue;
        }
        if(cmd=="stress"){
            int n; ss>>n;
            mt19937_64 rng((uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count());
            Timer t; t.start();
            for(int i=0;i<n;++i){
                int op = (int)(rng()%5);
                if(op==0){
                    int loc = (int)(rng()%100);
                    vector<Record> s = sample_uniform(rows, min(100, (int)rows.size()));
                    sort(s.begin(), s.end(), [](const Record&a,const Record&b){ return a.density < b.density; });
                } else if(op==1){
                    vector<Record> s = sample_stratified(rows, min(100, (int)rows.size()));
                } else if(op==2){
                    vector<Record> tmp = rows;
                    multi_threaded_sort(tmp, 2);
                } else if(op==3){
                    if(!byloc.empty()){
                        auto it = byloc.begin();
                        advance(it, rng()%byloc.size());
                        auto hits = detect_anomalies_zscore(it->second, 10, 3.0);
                    }
                } else {
                    vector<Record> s = sample_uniform(rows, 10);
                    ascii_histogram(s);
                }
            }
            cout<<"stress done "<<t.ms()<<" ms\n";
            continue;
        }
        cout<<"unknown\n";
    }
    return 0;
}
