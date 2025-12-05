#include <bits/stdc++.h>
using namespace std;

struct Fenwick {
    int n; vector<long long> bit;
    Fenwick(int sz=0){ init(sz); }
    void init(int sz){ n=sz; bit.assign(n+1,0); }
    void add(int idx,long long val){ for(++idx; idx<=n; idx+=idx&-idx) bit[idx]+=val; }
    long long sum(int idx){ long long r=0; for(++idx; idx>0; idx-=idx&-idx) r+=bit[idx]; return r; }
    long long range(int l,int r){ if(r<l) return 0; return sum(r)-(l?sum(l-1):0); }
};

struct Segment {
    int n; vector<double> mn, mx;
    Segment(int sz=0){ init(sz); }
    void init(int sz){ n=1; while(n<sz) n<<=1; mn.assign(2*n,1e18); mx.assign(2*n,-1e18); }
    void update(int i,double v){ i+=n; mn[i]=mx[i]=v; for(i>>=1;i;i>>=1){ mn[i]=min(mn[i<<1], mn[i<<1|1]); mx[i]=max(mx[i<<1], mx[i<<1|1]); } }
    pair<double,double> query(int l,int r){ double a=1e18,b=-1e18; for(l+=n,r+=n;l<=r;l>>=1,r>>=1){ if(l&1){ a=min(a,mn[l]); b=max(b,mx[l++]); } if(!(r&1)){ a=min(a,mn[r]); b=max(b,mx[r--]); } } return {a,b}; }
};

double median(vector<double> v){ sort(v.begin(), v.end()); int n=v.size(); if(n==0) return 0; if(n%2) return v[n/2]; return 0.5*(v[n/2-1]+v[n/2]); }

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n; if(!(cin>>n)) return 0;
    vector<pair<double,double>> coords(n);
    for(int i=0;i<n;i++) cin>>coords[i].first>>coords[i].second;
    int T; cin>>T;
    Segment seg(T);
    seg.init(T);
    Fenwick fw(T);
    vector<double> window;
    for(int t=0;t<T;t++){
        int idx; double val; cin>>idx>>val;
        seg.update(t, val);
        fw.add(t, (long long)round(val));
        window.push_back(val);
        if(t>=10){
            vector<double> sub(window.end()-10, window.end());
            double med = median(sub);
            double sumsq=0; for(double x: sub) sumsq += (x-med)*(x-med);
            double sd = sqrt(sumsq/sub.size());
            if(abs(window.back()-med) > 3*max(1.0, sd)) cout<<"ALERT "<<t<<" "<<idx<<"\n";
        }
    }
    int q; cin>>q;
    while(q--){
        int l,r; cin>>l>>r;
        auto pr = seg.query(l,r);
        cout.setf(std::ios::fixed); cout<<setprecision(6)<<pr.first<<" "<<pr.second<<"\n";
    }
    int k; cin>>k;
    vector<pair<double,int>> arr;
    for(int t=0;t<T;t++){
        auto pr = seg.query(t,t);
        arr.push_back({pr.second, t});
    }
    sort(arr.begin(), arr.end(), greater<pair<double,int>>());
    for(int i=0;i<k && i<(int)arr.size(); ++i) cout<<arr[i].second<<" "<<arr[i].first<<"\n";
    return 0;
}
