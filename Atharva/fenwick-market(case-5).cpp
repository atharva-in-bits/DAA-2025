#include <bits/stdc++.h>
using namespace std;

struct PriceRow { string date; int product_id; double price; };

static vector<PriceRow> load_csv(const string &path) {
    ifstream in(path);
    vector<PriceRow> out;
    if (!in.is_open()) return out;
    string line;
    if (!getline(in, line)) return out;
    while (getline(in, line)) {
        vector<string> cols; string cur; bool inq=false;
        for (char c:line) {
            if (c=='"') { inq=!inq; continue; }
            if (c==',' && !inq) { cols.push_back(cur); cur.clear(); } else cur.push_back(c);
        }
        cols.push_back(cur);
        if (cols.size() < 3) continue;
        PriceRow r; r.date = cols[0]; r.product_id = stoi(cols[1]); r.price = stod(cols[2]);
        out.push_back(r);
    }
    return out;
}

struct Fenwick {
    int n; vector<double> bit;
    Fenwick(): n(0) {}
    Fenwick(int n_): n(n_), bit(n_+1, 0.0) {}
    void add(int i, double delta){ for(++i;i<=n;i+=i&-i) bit[i]+=delta; }
    double sum(int i){ double s=0; for(++i;i>0;i-=i&-i) s+=bit[i]; return s; }
    double range(int l,int r){ if(l>r) return 0.0; return sum(r) - (l?sum(l-1):0.0); }
};

struct SegmentTree {
    int n; vector<double> seg;
    SegmentTree(): n(0) {}
    SegmentTree(int n_): n(n_), seg(4*n_, numeric_limits<double>::infinity()) {}
    void build(const vector<double> &a){ build_r(1,0,n-1,a); }
    void build_r(int p,int l,int r,const vector<double>&a){
        if(l==r){ seg[p]=a[l]; return; }
        int m=(l+r)/2; build_r(p<<1,l,m,a); build_r(p<<1|1,m+1,r,a);
        seg[p]=min(seg[p<<1], seg[p<<1|1]);
    }
    double query_min(int L,int R){ return query_r(1,0,n-1,L,R); }
    double query_r(int p,int l,int r,int L,int R){
        if(R<l||r<L) return numeric_limits<double>::infinity();
        if(L<=l&&r<=R) return seg[p];
        int m=(l+r)/2; return min(query_r(p<<1,l,m,L,R), query_r(p<<1|1,m+1,r,L,R));
    }
};

int main(int argc,char**argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(argc<2){ cerr<<"Usage: "<<argv[0]<<" <csv>\n"; return 1; }
    auto rows = load_csv(argv[1]);
    if(rows.empty()){ cerr<<"No data\n"; return 1; }
    unordered_map<int, vector<pair<string,double>>> perprod;
    for(auto &r:rows) perprod[r.product_id].push_back({r.date,r.price});
    unordered_map<int, vector<double>> pricevec;
    for(auto &kv:perprod){
        auto &v = kv.second;
        sort(v.begin(), v.end(), [](const pair<string,double>&a,const pair<string,double>&b){ return a.first < b.first; });
        for(auto &p:v) pricevec[kv.first].push_back(p.second);
    }
    unordered_map<int, Fenwick> fenw;
    unordered_map<int, SegmentTree> segt;
    for(auto &kv:pricevec){
        int pid = kv.first;
        int m = kv.second.size();
        Fenwick fw(m);
        for(int i=0;i<m;++i) fw.add(i, kv.second[i]);
        fenw[pid]=move(fw);
        SegmentTree st(m);
        st.build(kv.second);
        segt[pid]=move(st);
    }
    cout<<"Built for "<<pricevec.size()<<" products\n";
    cout<<"Commands:\nsummary\navg <pid> <l> <r>\nmin <pid> <l> <r>\nvolatility <pid> <l> <r>\nexit\n";
    string line;
    while(true){
        cout<<"> ";
        if(!getline(cin,line)) break;
        if(line.empty()) continue;
        stringstream ss(line); string cmd; ss>>cmd;
        if(cmd=="exit"||cmd=="quit") break;
        if(cmd=="summary"){
            cout<<"ProductID,Records\n";
            for(auto &kv:pricevec) cout<<kv.first<<","<<kv.second.size()<<"\n";
            continue;
        }
        if(cmd=="avg"){
            int pid,l,r; ss>>pid>>l>>r;
            if(fenw.find(pid)==fenw.end()){ cout<<"no product\n"; continue; }
            double s = fenw[pid].range(l,r);
            double avg = s / (r-l+1);
            cout<<"avg="<<avg<<"\n";
            continue;
        }
        if(cmd=="min"){
            int pid,l,r; ss>>pid>>l>>r;
            if(segt.find(pid)==segt.end()){ cout<<"no product\n"; continue; }
            double m = segt[pid].query_min(l,r);
            cout<<"min="<<m<<"\n";
            continue;
        }
        if(cmd=="volatility"){
            int pid,l,r; ss>>pid>>l>>r;
            if(pricevec.find(pid)==pricevec.end()){ cout<<"no product\n"; continue; }
            auto &v = pricevec[pid];
            if(l<0) l=0; if(r>= (int)v.size()) r=v.size()-1;
            double mean=0; int cnt=0;
            for(int i=l;i<=r;++i){ mean+=v[i]; ++cnt; }
            mean/=cnt;
            double var=0;
            for(int i=l;i<=r;++i){ double d=v[i]-mean; var+=d*d; }
            var/=cnt;
            double sd = sqrt(var);
            cout<<"stddev="<<sd<<"\n";
            continue;
        }
        cout<<"unknown\n";
    }
    return 0;
}
