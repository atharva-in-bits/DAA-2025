#include <bits/stdc++.h>
using namespace std;
using ll = long long;
struct Fenwick {
    int n;
    vector<double> bit;
    Fenwick(): n(0) {}
    void init(int N){ n=N; bit.assign(n+1,0.0); }
    void add(int idx,double val){ idx++; while(idx<=n){ bit[idx]+=val; idx+=idx&-idx; } }
    double sumPrefix(int idx){ idx++; double s=0; while(idx>0){ s+=bit[idx]; idx-=idx&-idx; } return s; }
    double rangeSum(int l,int r){ if(r<l) return 0.0; return sumPrefix(r) - (l?sumPrefix(l-1):0.0); }
};
struct SegTree {
    int n;
    vector<double> mn, mx;
    SegTree(): n(0) {}
    void build(const vector<double>&arr){
        int N=arr.size();
        n=1; while(n<N) n<<=1;
        mn.assign(2*n, 1e300);
        mx.assign(2*n, -1e300);
        for(int i=0;i<N;++i){ mn[n+i]=arr[i]; mx[n+i]=arr[i]; }
        for(int i=n-1;i>=1;--i){ mn[i]=min(mn[2*i], mn[2*i+1]); mx[i]=max(mx[2*i], mx[2*i+1]); }
    }
    void update(int idx,double val){ int p=n+idx; mn[p]=val; mx[p]=val; p>>=1; while(p>=1){ mn[p]=min(mn[2*p], mn[2*p+1]); mx[p]=max(mx[2*p], mx[2*p+1]); p>>=1; } }
    pair<double,double> query(int l,int r){ if(l>r) return {1e300,-1e300}; double mnv=1e300,mxv=-1e300; l+=n; r+=n; while(l<=r){ if(l&1){ mnv=min(mnv,mn[l]); mxv=max(mxv,mx[l]); ++l;} if(!(r&1)){ mnv=min(mnv,mn[r]); mxv=max(mxv,mx[r]); --r;} l>>=1; r>>=1; } return {mnv,mxv}; }
};
struct Reading { long long ts; int sensor; double pm25, pm10, no2, so2, o3, temp; };
static inline vector<string> split_csv(const string &s){
    vector<string> out; string cur; bool inq=false;
    for(size_t i=0;i<s.size();++i){
        char c=s[i];
        if(inq){ if(c=='"'){ if(i+1<s.size() && s[i+1]=='"'){ cur.push_back('"'); ++i; } else inq=false; } else cur.push_back(c); }
        else { if(c==','){ out.push_back(cur); cur.clear(); } else if(c=='"') inq=true; else cur.push_back(c); }
    }
    out.push_back(cur);
    for(auto &t:out){ size_t a=0,b=t.size(); while(a<b && isspace((unsigned char)t[a])) ++a; while(b>a && isspace((unsigned char)t[b-1])) --b; t=t.substr(a,b-a); }
    return out;
}
int to_int(const string &s){ if(s.empty()) return 0; try{return stoi(s);}catch(...){return 0;} }
long long to_ll(const string &s){ if(s.empty()) return 0; try{return stoll(s);}catch(...){return 0;} }
double to_double(const string &s){ if(s.empty()) return 0.0; try{return stod(s);}catch(...){return 0.0;} }
struct SensorStore {
    vector<Reading> readings;
    Fenwick fen_pm25, fen_pm10, fen_no2, fen_so2, fen_o3, fen_temp;
    SegTree seg_pm25, seg_pm10, seg_no2, seg_so2, seg_o3, seg_temp;
    void build_indices(){
        int n = readings.size();
        vector<double> a;
        a.resize(n);
        for(int i=0;i<n;++i) a[i]=readings[i].pm25;
        fen_pm25.init(n); for(int i=0;i<n;++i) fen_pm25.add(i, a[i]);
        seg_pm25.build(a);
        a.resize(n); for(int i=0;i<n;++i) a[i]=readings[i].pm10;
        fen_pm10.init(n); for(int i=0;i<n;++i) fen_pm10.add(i, a[i]);
        seg_pm10.build(a);
        a.resize(n); for(int i=0;i<n;++i) a[i]=readings[i].no2;
        fen_no2.init(n); for(int i=0;i<n;++i) fen_no2.add(i,a[i]);
        seg_no2.build(a);
        a.resize(n); for(int i=0;i<n;++i) a[i]=readings[i].so2;
        fen_so2.init(n); for(int i=0;i<n;++i) fen_so2.add(i,a[i]);
        seg_so2.build(a);
        a.resize(n); for(int i=0;i<n;++i) a[i]=readings[i].o3;
        fen_o3.init(n); for(int i=0;i<n;++i) fen_o3.add(i,a[i]);
        seg_o3.build(a);
        a.resize(n); for(int i=0;i<n;++i) a[i]=readings[i].temp;
        fen_temp.init(n); for(int i=0;i<n;++i) fen_temp.add(i,a[i]);
        seg_temp.build(a);
    }
    void append(const Reading &r){
        int idx = readings.size();
        readings.push_back(r);
        fen_pm25.add(idx, r.pm25); fen_pm10.add(idx, r.pm10); fen_no2.add(idx, r.no2);
        fen_so2.add(idx, r.so2); fen_o3.add(idx, r.o3); fen_temp.add(idx, r.temp);
        int n = readings.size();
        vector<double> a(n);
        for(int i=0;i<n;++i) a[i]=readings[i].pm25;
        seg_pm25.build(a);
        for(int i=0;i<n;++i) a[i]=readings[i].pm10;
        seg_pm10.build(a);
        for(int i=0;i<n;++i) a[i]=readings[i].no2;
        seg_no2.build(a);
        for(int i=0;i<n;++i) a[i]=readings[i].so2;
        seg_so2.build(a);
        for(int i=0;i<n;++i) a[i]=readings[i].o3;
        seg_o3.build(a);
        for(int i=0;i<n;++i) a[i]=readings[i].temp;
        seg_temp.build(a);
    }
};
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    vector<string> lines; string row;
    while(getline(cin,row)) lines.push_back(row);
    if(lines.empty()) return 0;
    int first=0; while(first<(int)lines.size() && lines[first].find(',')==string::npos) ++first;
    if(first>=(int)lines.size()) return 0;
    vector<string> header = split_csv(lines[first]);
    unordered_map<string,int> h;
    for(int i=0;i<(int)header.size();++i){ string k=header[i]; for(auto &c:k) c=tolower((unsigned char)c); h[k]=i; }
    unordered_map<int, SensorStore> store;
    for(int i=first+1;i<(int)lines.size();++i){
        if(lines[i].find_first_not_of(" \t\r\n")==string::npos) continue;
        auto f = split_csv(lines[i]);
        string cmd = "";
        if(h.count("command")) cmd = f[h["command"]];
        string up = cmd; for(auto &c:up) c=toupper((unsigned char)c);
        if(up=="" || up=="SENSOR"){
            long long ts = to_ll(f[h.count("timestamp")?h["timestamp"]:1]);
            int sid = to_int(f[h.count("sensor_id")?h["sensor_id"]:2]);
            Reading r; r.ts=ts; r.sensor=sid; r.pm25=to_double(f[h.count("pm25")?h["pm25"]:3]); r.pm10=to_double(f[h.count("pm10")?h["pm10"]:4]);
            r.no2=to_double(f[h.count("no2")?h["no2"]:5]); r.so2=to_double(f[h.count("so2")?h["so2"]:6]); r.o3=to_double(f[h.count("o3")?h["o3"]:7]); r.temp=to_double(f[h.count("temp")?h["temp"]:8]);
            store[sid].readings.push_back(r);
        } else {
            // ignore other commands for now
        }
    }
    for(auto &p: store) p.second.build_indices();
    cout<<"Loaded sensors: "<<store.size()<<"\n";
    cout<<"Interactive commands:\n";
    cout<<"  SLIDING_WINDOW sensor_id window_size threshold_field threshold_value\n";
    cout<<"  TOP_K pollutant k\n";
    cout<<"  FENWICK_QUERY sensor l r field\n";
    cout<<"  SEG_QUERY sensor l r field\n";
    cout<<"  APPEND_SENSOR timestamp sensor pm25 pm10 no2 so2 o3 temp\n";
    cout<<"  QUIT\n";
    string linecmd;
    while(true){
        if(!getline(cin,linecmd)) break;
        if(linecmd.find_first_not_of(" \t\r\n")==string::npos) continue;
        stringstream ss(linecmd); string cmd; ss>>cmd;
        for(auto &c:cmd) c=toupper((unsigned char)c);
        if(cmd=="QUIT") break;
        else if(cmd=="SLIDING_WINDOW"){
            int sid; int w; string field; double thresh;
            if(!(ss>>sid>>w>>field>>thresh)){ cout<<"Usage: SLIDING_WINDOW sensor_id window_size field threshold\n"; continue; }
            if(!store.count(sid)){ cout<<"Sensor not found\n"; continue; }
            auto &S = store[sid];
            int n = S.readings.size();
            if(w<=0 || n==0){ cout<<"No data\n"; continue; }
            vector<double> vals(n);
            if(field=="pm25") for(int i=0;i<n;++i) vals[i]=S.readings[i].pm25;
            else if(field=="pm10") for(int i=0;i<n;++i) vals[i]=S.readings[i].pm10;
            else if(field=="no2") for(int i=0;i<n;++i) vals[i]=S.readings[i].no2;
            else if(field=="so2") for(int i=0;i<n;++i) vals[i]=S.readings[i].so2;
            else if(field=="o3") for(int i=0;i<n;++i) vals[i]=S.readings[i].o3;
            else if(field=="temp") for(int i=0;i<n;++i) vals[i]=S.readings[i].temp;
            else { cout<<"Unknown field\n"; continue; }
            int anomalies=0;
            for(int i=w-1;i<n;++i){
                double sum=0; for(int j=i-w+1;j<=i;++j) sum+=vals[j];
                double mean = sum / w;
                double var=0; for(int j=i-w+1;j<=i;++j) var += (vals[j]-mean)*(vals[j]-mean);
                double sd = sqrt(var / w);
                double z = sd < 1e-12 ? 0.0 : (vals[i]-mean)/sd;
                if(fabs(z) > thresh) anomalies++;
            }
            cout<<"Anomalies detected: "<<anomalies<<"\n";
        } else if(cmd=="TOP_K"){
            string pollutant; int k;
            if(!(ss>>pollutant>>k)){ cout<<"Usage: TOP_K pollutant k\n"; continue; }
            vector<pair<double,int>> agg;
            for(auto &p: store){
                int sid = p.first;
                auto &S = p.second;
                if(S.readings.empty()) continue;
                double val=0;
                if(pollutant=="pm25") val = S.readings.back().pm25;
                else if(pollutant=="pm10") val = S.readings.back().pm10;
                else if(pollutant=="no2") val = S.readings.back().no2;
                else if(pollutant=="so2") val = S.readings.back().so2;
                else if(pollutant=="o3") val = S.readings.back().o3;
                else { cout<<"Unknown pollutant\n"; break; }
                agg.emplace_back(val, sid);
            }
            sort(agg.begin(), agg.end(), greater<pair<double,int>>());
            for(int i=0;i<min(k,(int)agg.size());++i) cout<<i+1<<". sensor "<<agg[i].second<<" value="<<agg[i].first<<"\n";
        } else if(cmd=="FENWICK_QUERY"){
            int sid,l,r; string field;
            if(!(ss>>sid>>l>>r>>field)){ cout<<"Usage: FENWICK_QUERY sensor l r field\n"; continue; }
            if(!store.count(sid)){ cout<<"Sensor not found\n"; continue; }
            auto &S = store[sid];
            int n = S.readings.size();
            if(l<0) l=0; if(r>=n) r=n-1;
            double res=0;
            if(field=="pm25") res = S.fen_pm25.rangeSum(l,r);
            else if(field=="pm10") res = S.fen_pm10.rangeSum(l,r);
            else if(field=="no2") res = S.fen_no2.rangeSum(l,r);
            else if(field=="so2") res = S.fen_so2.rangeSum(l,r);
            else if(field=="o3") res = S.fen_o3.rangeSum(l,r);
            else if(field=="temp") res = S.fen_temp.rangeSum(l,r);
            else { cout<<"Unknown field\n"; continue; }
            cout<<"Sum="<<res<<"\n";
        } else if(cmd=="SEG_QUERY"){
            int sid,l,r; string field;
            if(!(ss>>sid>>l>>r>>field)){ cout<<"Usage: SEG_QUERY sensor l r field\n"; continue; }
            if(!store.count(sid)){ cout<<"Sensor not found\n"; continue; }
            auto &S = store[sid];
            int n = S.readings.size();
            if(l<0) l=0; if(r>=n) r=n-1;
            pair<double,double> pr;
            if(field=="pm25") pr = S.seg_pm25.query(l,r);
            else if(field=="pm10") pr = S.seg_pm10.query(l,r);
            else if(field=="no2") pr = S.seg_no2.query(l,r);
            else if(field=="so2") pr = S.seg_so2.query(l,r);
            else if(field=="o3") pr = S.seg_o3.query(l,r);
            else if(field=="temp") pr = S.seg_temp.query(l,r);
            else { cout<<"Unknown field\n"; continue; }
            cout<<"min="<<pr.first<<" max="<<pr.second<<"\n";
        } else if(cmd=="APPEND_SENSOR"){
            long long ts; int sid; double pm25,pm10,no2,so2,o3,temp;
            if(!(ss>>ts>>sid>>pm25>>pm10>>no2>>so2>>o3>>temp)){ cout<<"Usage: APPEND_SENSOR ts sid pm25 pm10 no2 so2 o3 temp\n"; continue; }
            Reading r; r.ts=ts; r.sensor=sid; r.pm25=pm25; r.pm10=pm10; r.no2=no2; r.so2=so2; r.o3=o3; r.temp=temp;
            store[sid].append(r);
            cout<<"Appended\n";
        } else cout<<"Unknown command\n";
    }
    return 0;
}
