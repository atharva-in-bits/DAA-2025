#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using db = double;
const db INF = 1e18;

struct SegTree {
    int n;
    vector<db> mn;
    vector<db> mx;
    void init(int sz){
        n=1; while(n<sz) n<<=1;
        mn.assign(2*n, INF);
        mx.assign(2*n, -INF);
    }
    void build_from(const vector<db>&a){
        int sz=a.size();
        init(sz);
        for(int i=0;i<sz;i++){ mn[n+i]=a[i]; mx[n+i]=a[i]; }
        for(int i=n-1;i>0;i--){ mn[i]=min(mn[i<<1], mn[i<<1|1]); mx[i]=max(mx[i<<1], mx[i<<1|1]); }
    }
    void point_update(int idx, db val){
        idx += n;
        mn[idx]=mx[idx]=val;
        for(idx>>=1; idx; idx>>=1){
            mn[idx]=min(mn[idx<<1], mn[idx<<1|1]);
            mx[idx]=max(mx[idx<<1], mx[idx<<1|1]);
        }
    }
    pair<db,db> range_query(int l,int r){
        if(l>r) return {INF,-INF};
        l+=n; r+=n;
        db amn=INF, amx=-INF;
        while(l<=r){
            if(l&1){ amn=min(amn,mn[l]); amx=max(amx,mx[l]); l++; }
            if(!(r&1)){ amn=min(amn,mn[r]); amx=max(amx,mx[r]); r--; }
            l>>=1; r>>=1;
        }
        return {amn,amx};
    }
};

struct SparseTable {
    int n;
    vector<int> lg;
    vector<vector<db>> stmin;
    vector<vector<db>> stmax;
    void build(const vector<db>&a){
        n=a.size();
        lg.assign(n+1,0);
        for(int i=2;i<=n;i++) lg[i]=lg[i>>1]+1;
        int K = lg[n]+1;
        stmin.assign(K, vector<db>(n, INF));
        stmax.assign(K, vector<db>(n, -INF));
        for(int i=0;i<n;i++){ stmin[0][i]=a[i]; stmax[0][i]=a[i]; }
        for(int k=1;k<K;k++){
            for(int i=0;i + (1<<k) <= n; i++){
                stmin[k][i] = min(stmin[k-1][i], stmin[k-1][i + (1<<(k-1))]);
                stmax[k][i] = max(stmax[k-1][i], stmax[k-1][i + (1<<(k-1))]);
            }
        }
    }
    db query_min(int l,int r){
        if(l>r) return INF;
        int k = lg[r-l+1];
        return min(stmin[k][l], stmin[k][r-(1<<k)+1]);
    }
    db query_max(int l,int r){
        if(l>r) return -INF;
        int k = lg[r-l+1];
        return max(stmax[k][l], stmax[k][r-(1<<k)+1]);
    }
};

vector<int> build_lps(const string &p){
    int m=p.size();
    vector<int> lps(m,0);
    for(int i=1,len=0;i<m;){
        if(p[i]==p[len]) lps[i++]=++len;
        else if(len) len = lps[len-1];
        else lps[i++]=0;
    }
    return lps;
}

bool kmp_search(const string &s, const string &p){
    if(p.empty()) return true;
    auto lps = build_lps(p);
    int i=0,j=0;
    int n=s.size(), m=p.size();
    while(i<n){
        if(s[i]==p[j]){ i++; j++; if(j==m) return true; }
        else if(j) j = lps[j-1];
        else i++;
    }
    return false;
}

db mean_vec(const vector<db>&v){
    if(v.empty()) return 0.0;
    db s=0;
    for(db x:v) s+=x;
    return s / v.size();
}
db stddev_vec(const vector<db>&v, db mu){
    if(v.empty()) return 0.0;
    db s=0;
    for(db x:v){ db d=x-mu; s += d*d; }
    return sqrt(s / v.size());
}

db median_vec(vector<db> v){
    int n=v.size();
    if(n==0) return 0.0;
    nth_element(v.begin(), v.begin()+n/2, v.end());
    db med = v[n/2];
    if(n%2==0){
        nth_element(v.begin(), v.begin()+n/2-1, v.end());
        med = 0.5*(med + v[n/2-1]);
    }
    return med;
}

db iqr_vec(vector<db> v){
    int n=v.size();
    if(n<4) return 0.0;
    sort(v.begin(), v.end());
    int q1i = n/4;
    int q3i = (3*n)/4;
    db q1 = v[q1i];
    db q3 = v[q3i];
    return q3 - q1;
}

struct SlidingStats {
    deque<db> window;
    int capacity;
    SlidingStats(int cap=50):capacity(cap){}
    void push(db x){
        window.push_back(x);
        if((int)window.size()>capacity) window.pop_front();
    }
    db mean(){ vector<db> tmp(window.begin(), window.end()); return mean_vec(tmp); }
    db stddev(){ vector<db> tmp(window.begin(), window.end()); return stddev_vec(tmp, mean_vec(tmp)); }
    db median(){ vector<db> tmp(window.begin(), window.end()); return median_vec(tmp); }
    db iqr(){ vector<db> tmp(window.begin(), window.end()); return iqr_vec(tmp); }
};

struct TimeSeriesStore {
    vector<db> data;
    TimeSeriesStore(){}
    void append(db x){ data.push_back(x); }
    int size() const { return (int)data.size(); }
    vector<db> tail(int k) const {
        vector<db> out;
        int n=data.size();
        if(k<=0) return out;
        for(int i=max(0,n-k); i<n; ++i) out.push_back(data[i]);
        return out;
    }
    db rolling_mean(int l,int r){
        if(l>r) return 0.0;
        db s=0; int c=0;
        for(int i=l;i<=r;i++){ s+=data[i]; c++; }
        return c? s/c : 0.0;
    }
};

struct Alert {
    int t;
    string type;
    db value;
    Alert():t(0),type(""),value(0){} 
    Alert(int tt,const string &ty, db v):t(tt),type(ty),value(v){}
};

vector<Alert> detect_anomalies_zscore(const vector<db>&stream, int window, db zthreshold){
    vector<Alert> out;
    SlidingStats s(window);
    for(int t=0;t<(int)stream.size(); ++t){
        db val=stream[t];
        s.push(val);
        if((int)s.window.size() < window) continue;
        db mu = s.mean();
        db sd = s.stddev();
        if(sd < 1e-9) sd = 1e-9;
        if(fabs(val - mu) > zthreshold * sd) out.emplace_back(t, "Z", val);
    }
    return out;
}

vector<Alert> detect_anomalies_iqr(const vector<db>&stream, int window, db factor){
    vector<Alert> out;
    SlidingStats s(window);
    for(int t=0;t<(int)stream.size(); ++t){
        db val=stream[t];
        s.push(val);
        if((int)s.window.size() < window) continue;
        db med = s.median();
        db iqr = s.iqr();
        if(iqr < 1e-9) iqr = 1e-9;
        if(fabs(val - med) > factor * iqr) out.emplace_back(t, "IQR", val);
    }
    return out;
}

vector<db> moving_average(const vector<db>&stream,int k){
    int n=stream.size();
    vector<db> res(n,0.0);
    db s=0;
    for(int i=0;i<n;i++){
        s += stream[i];
        if(i>=k) s -= stream[i-k];
        if(i>=k-1) res[i] = s / k;
        else res[i] = s / (i+1);
    }
    return res;
}

vector<pair<int,db>> top_k_spikes(const vector<db>&stream, int k){
    int n=stream.size();
    vector<pair<int,db>> arr;
    for(int i=0;i<n;i++) arr.emplace_back(i, stream[i]);
    sort(arr.begin(), arr.end(), [](const pair<int,db>&a,const pair<int,db>&b){ return a.second > b.second; });
    if(k > (int)arr.size()) k=arr.size();
    vector<pair<int,db>> out(arr.begin(), arr.begin()+k);
    return out;
}

struct StreamProcessor {
    TimeSeriesStore store;
    SegTree seg;
    SparseTable sp;
    int built_sp;
    StreamProcessor():built_sp(0){}
    void ingest(db x){
        store.append(x);
        int sz = store.size();
        if(sz==1){
            seg.build_from(vector<db>{x});
            vector<db> tmp = store.tail(sz);
            sp.build(tmp);
            built_sp = sz;
        } else {
            vector<db> tmp = store.tail(sz);
            seg.build_from(tmp);
            sp.build(tmp);
            built_sp = sz;
        }
    }
    pair<db,db> query_range(int l,int r){
        int sz = store.size();
        if(l<0) l=0;
        if(r>=sz) r=sz-1;
        return seg.range_query(l,r);
    }
    db sparse_min(int l,int r){
        int sz = store.size();
        if(l<0) l=0; if(r>=sz) r=sz-1;
        if(built_sp==0) return 0.0;
        return sp.query_min(l,r);
    }
    db sparse_max(int l,int r){
        int sz = store.size();
        if(l<0) l=0; if(r>=sz) r=sz-1;
        if(built_sp==0) return 0.0;
        return sp.query_max(l,r);
    }
};

vector<Alert> windowed_detector(const vector<db>&stream, int window){
    vector<Alert> out1 = detect_anomalies_zscore(stream, window, 3.0);
    vector<Alert> out2 = detect_anomalies_iqr(stream, window, 2.0);
    vector<Alert> merged = out1;
    for(auto &a: out2) merged.push_back(a);
    sort(merged.begin(), merged.end(), [](const Alert&a,const Alert&b){ if(a.t!=b.t) return a.t<b.t; return a.type<b.type; });
    merged.erase(unique(merged.begin(), merged.end(), [](const Alert&a,const Alert&b){ return a.t==b.t && a.type==b.type; }), merged.end());
    return merged;
}

vector<db> generate_demo_signal(int n){
    vector<db> s; s.reserve(n);
    for(int i=0;i<n;i++){
        db v = 25.0 + 3.0 * sin(i/10.0) + ((i%7)==0 ? 10.0 : 0.0) + ( (i%23)==0 ? -8.0 : 0.0 );
        s.push_back(v);
    }
    return s;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int mode;
    if(!(cin>>mode)) return 0;
    if(mode==0){
        int n=600;
        auto signal = generate_demo_signal(n);
        auto ma = moving_average(signal, 5);
        auto spikes = top_k_spikes(signal, 10);
        auto alerts = windowed_detector(signal, 50);
        StreamProcessor proc;
        for(auto v: signal) proc.ingest(v);
        auto qr = proc.query_range(10, 30);
        cout.setf(std::ios::fixed);
        cout<<setprecision(6);
        cout<<"MA_LAST "<<ma.back()<<"\n";
        cout<<"SEG_RANGE_10_30 "<<qr.first<<" "<<qr.second<<"\n";
        cout<<"SP_MIN_10_30 "<<proc.sparse_min(10,30)<<"\n";
        cout<<"SP_MAX_10_30 "<<proc.sparse_max(10,30)<<"\n";
        cout<<"TOP_SPIKES ";
        for(auto &p: spikes) cout<<p.first<<":"<<p.second<<" ";
        cout<<"\n";
        cout<<"ALERT_COUNT "<<alerts.size()<<"\n";
        for(auto &a: alerts) cout<<a.t<<" "<<a.type<<" "<<a.value<<"\n";
        string doc = "pump malfunction contamination detected valve closed urgent contamination downstream";
        vector<string> patterns = {"contamination","malfunction","leak","urgent"};
        for(auto &pat: patterns) cout<<pat<<":"<<(kmp_search(doc, pat) ? "1":"0")<<" ";
        cout<<"\n";
        return 0;
    }
    if(mode==1){
        int n;
        cin>>n;
        vector<db> a(n);
        for(int i=0;i<n;i++) cin>>a[i];
        StreamProcessor proc;
        for(int i=0;i<n;i++) proc.ingest(a[i]);
        int q;
        cin>>q;
        cout.setf(std::ios::fixed);
        cout<<setprecision(6);
        while(q--){
            int l,r; string type;
            cin>>type;
            if(type=="range"){
                cin>>l>>r;
                auto p = proc.query_range(l,r);
                cout<<p.first<<" "<<p.second<<"\n";
            } else if(type=="sparse"){
                cin>>l>>r;
                cout<<proc.sparse_min(l,r)<<" "<<proc.sparse_max(l,r)<<"\n";
            } else if(type=="kmp"){
                string text,pat;
                getline(cin, text);
                getline(cin, text);
                getline(cin, pat);
                cout<<(kmp_search(text, pat) ? "1":"0")<<"\n";
            } else if(type=="detect"){
                cin>>l;
                auto alerts = windowed_detector(a, l);
                cout<<alerts.size()<<"\n";
                for(auto &at: alerts) cout<<at.t<<" "<<at.type<<" "<<at.value<<"\n";
            }
        }
        return 0;
    }
    if(mode==2){
        int n; cin>>n;
        vector<db> a(n);
        for(int i=0;i<n;i++) cin>>a[i];
        auto alerts = detect_anomalies_zscore(a, 30, 3.0);
        cout<<alerts.size()<<"\n";
        for(auto &al: alerts) cout<<al.t<<" "<<al.type<<" "<<al.value<<"\n";
        return 0;
    }
    return 0;
}
