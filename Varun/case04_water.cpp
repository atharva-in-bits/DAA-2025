#include <bits/stdc++.h>
using namespace std;
using ll = long long;
struct SegTree {
    int n;
    vector<double> mins, maxs;
    SegTree(): n(0) {}
    void build(const vector<double>&a) {
        n = a.size();
        mins.assign(4*n, 1e300);
        maxs.assign(4*n, -1e300);
        if(n>0) build_rec(1,0,n-1,a);
    }
    void build_rec(int node,int l,int r,const vector<double>&a){
        if(l==r){ mins[node]=a[l]; maxs[node]=a[l]; return; }
        int m=(l+r)/2;
        build_rec(node*2,l,m,a);
        build_rec(node*2+1,m+1,r,a);
        mins[node]=min(mins[node*2], mins[node*2+1]);
        maxs[node]=max(maxs[node*2], maxs[node*2+1]);
    }
    void update(int idx,double val){ if(idx<0||idx>=n) return; update_rec(1,0,n-1,idx,val); }
    void update_rec(int node,int l,int r,int idx,double val){
        if(l==r){ mins[node]=val; maxs[node]=val; return; }
        int m=(l+r)/2;
        if(idx<=m) update_rec(node*2,l,m,idx,val); else update_rec(node*2+1,m+1,r,idx,val);
        mins[node]=min(mins[node*2], mins[node*2+1]);
        maxs[node]=max(maxs[node*2], maxs[node*2+1]);
    }
    pair<double,double> query(int L,int R){ if(L>R||n==0) return {1e300,-1e300}; return query_rec(1,0,n-1,L,R); }
    pair<double,double> query_rec(int node,int l,int r,int L,int R){
        if(L>r||R<l) return {1e300,-1e300};
        if(L<=l&&r<=R) return {mins[node], maxs[node]};
        int m=(l+r)/2;
        auto a=query_rec(node*2,l,m,L,R);
        auto b=query_rec(node*2+1,m+1,r,L,R);
        return { min(a.first,b.first), max(a.second,b.second) };
    }
};
struct SparseTable {
    int n,LOG;
    vector<vector<double>> st;
    vector<int> lg;
    void build(const vector<double>&a){
        n=a.size();
        lg.assign(n+1,0);
        for(int i=2;i<=n;i++) lg[i]=lg[i/2]+1;
        LOG = lg[n];
        st.assign(LOG+1, vector<double>(n));
        for(int i=0;i<n;i++) st[0][i]=a[i];
        for(int k=1;k<=LOG;k++){
            int len = 1<<k;
            int half = 1<<(k-1);
            for(int i=0;i+len<=n;i++){
                st[k][i]=min(st[k-1][i], st[k-1][i+half]);
            }
        }
    }
    double range_min(int L,int R){
        if(L>R||n==0) return 1e300;
        int j = lg[R-L+1];
        return min(st[j][L], st[j][R-(1<<j)+1]);
    }
};
vector<string> split_csv(const string &s){
    vector<string> out; string cur; bool inq=false;
    for(size_t i=0;i<s.size();++i){
        char c=s[i];
        if(inq){
            if(c=='"'){ if(i+1<s.size() && s[i+1]=='"'){ cur.push_back('"'); ++i; } else inq=false; }
            else cur.push_back(c);
        } else {
            if(c==','){ out.push_back(cur); cur.clear(); }
            else if(c=='"') inq=true;
            else cur.push_back(c);
        }
    }
    out.push_back(cur);
    for(auto &t:out){ size_t a=0,b=t.size(); while(a<b && isspace((unsigned char)t[a])) ++a; while(b>a && isspace((unsigned char)t[b-1])) --b; t=t.substr(a,b-a); }
    return out;
}
int parse_int(const string &s){ if(s.empty()) return 0; try{return stoi(s);}catch(...){return 0;} }
double parse_double(const string &s){ if(s.empty()) return 0.0; try{return stod(s);}catch(...){return 0.0;} }
vector<int> kmp_table(const string &p){ int m=p.size(); vector<int> t(m+1); t[0]=-1; int pos=1,cnd=0; while(pos<m){ if(p[pos]==p[cnd]){ t[pos]=t[cnd]; pos++; cnd++; } else { t[pos]=cnd; cnd=t[cnd]; if(cnd<0){ pos++; cnd=0; } } } t[pos]=cnd; return t; }
vector<int> kmp_search(const string &text,const string &pattern){
    vector<int> occ; if(pattern.empty()) return occ;
    auto t = kmp_table(pattern);
    int m=0,i=0; int n=text.size();
    while(m+i<n){
        if(pattern[i]==text[m+i]){ i++; if(i==pattern.size()){ occ.push_back(m); m = m + i - t[i]; i = (t[i] < 0) ? 0 : t[i]; } }
        else { m = m + i - t[i]; i = (t[i] < 0) ? 0 : t[i]; }
    }
    return occ;
}
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    vector<string> lines; string row;
    while(getline(cin,row)) lines.push_back(row);
    if(lines.empty()) return 0;
    int first=0; while(first<(int)lines.size() && split_csv(lines[first]).size()==1 && split_csv(lines[first])[0].find(',')==string::npos) first++;
    if(first>=(int)lines.size()) return 0;
    auto header = split_csv(lines[first]);
    unordered_map<string,int> hmap;
    for(int i=0;i<(int)header.size();++i){ string t=header[i]; for(auto &c:t) c=tolower((unsigned char)c); hmap[t]=i; }
    int max_sensors = 10000;
    vector<vector<double>> sensor_contamination;
    vector<SegTree> segs;
    vector<SparseTable> spars;
    vector<string> reports;
    for(int li=first+1; li<(int)lines.size(); ++li){
        string s = lines[li];
        if(s.find_first_not_of(" \t\r\n")==string::npos) continue;
        auto f = split_csv(s);
        string cmd = "";
        if(hmap.count("command")) cmd = f[hmap["command"]];
        if(cmd.empty()){
            if(hmap.count("timestamp") && hmap.count("sensor_id") && hmap.count("contamination")) cmd = "SENSOR";
        }
        string cu=cmd; for(auto &c:cu) c=toupper((unsigned char)c);
        if(cu=="SENSOR"){
            int sid = parse_int(f[hmap["sensor_id"]]);
            double cont = parse_double(f[hmap["contamination"]]);
            if(sid >= (int)sensor_contamination.size()){
                int old = sensor_contamination.size();
                sensor_contamination.resize(sid+1);
                segs.resize(sid+1);
                spars.resize(sid+1);
            }
            sensor_contamination[sid].push_back(cont);
        } else if(cu=="UPDATE"){
            int sid = parse_int(f[hmap["sensor_id"]]);
            double cont = parse_double(f[hmap["contamination"]]);
            int idx = -1;
            if(hmap.count("timestamp")) idx = parse_int(f[hmap["timestamp"]]); // treat as index if provided
            if(sid < (int)sensor_contamination.size()){
                if(idx>=0 && idx < (int)sensor_contamination[sid].size()){
                    sensor_contamination[sid][idx]=cont;
                    if(segs[sid].n>0) segs[sid].update(idx, cont);
                } else {
                    sensor_contamination[sid].push_back(cont);
                }
            } else {
                int old = sensor_contamination.size();
                sensor_contamination.resize(sid+1);
                segs.resize(sid+1);
                spars.resize(sid+1);
                sensor_contamination[sid].push_back(cont);
            }
        } else if(cu=="SEG_QUERY"){
            int sid = parse_int(f[hmap["sensor_id"]]);
            int L = parse_int(f[hmap["query_l"]]);
            int R = parse_int(f[hmap["query_r"]]);
            if(sid < (int)sensor_contamination.size()){
                if(segs[sid].n==0 && !sensor_contamination[sid].empty()) segs[sid].build(sensor_contamination[sid]);
                auto res = segs[sid].query(L,R);
                if(res.first>1e200) cout<<"SEG_QUERY "<<sid<<" ["<<L<<","<<R<<"] NO_DATA\n"; else cout<<"SEG_QUERY "<<sid<<" ["<<L<<","<<R<<"] min="<<res.first<<" max="<<res.second<<"\n";
            } else cout<<"SEG_QUERY "<<sid<<" NO_SENSOR\n";
        } else if(cu=="SPARSE_BUILD"){
            int sid = parse_int(f[hmap["sensor_id"]]);
            if(sid < (int)sensor_contamination.size()){
                spars[sid].build(sensor_contamination[sid]);
                cout<<"SPARSE_BUILD "<<sid<<" DONE\n";
            } else cout<<"SPARSE_BUILD "<<sid<<" NO_SENSOR\n";
        } else if(cu=="SPARSE_QUERY"){
            int sid = parse_int(f[hmap["sensor_id"]]);
            int L = parse_int(f[hmap["query_l"]]);
            int R = parse_int(f[hmap["query_r"]]);
            if(sid < (int)sensor_contamination.size()){
                double v = spars[sid].range_min(L,R);
                if(v>1e200) cout<<"SPARSE_QUERY "<<sid<<" ["<<L<<","<<R<<"] NO_DATA\n"; else cout<<"SPARSE_QUERY "<<sid<<" ["<<L<<","<<R<<"] min="<<v<<"\n";
            } else cout<<"SPARSE_QUERY "<<sid<<" NO_SENSOR\n";
        } else if(cu=="REPORT"){
            string text = "";
            if(hmap.count("report_text")) text = f[hmap["report_text"]];
            reports.push_back(text);
            cout<<"REPORT_RECEIVED size="<<text.size()<<"\n";
        } else if(cu=="KMP_SCAN"){
            string pattern = "";
            if(hmap.count("pattern")) pattern = f[hmap["pattern"]];
            int total=0;
            for(auto &txt: reports){
                auto occ = vector<int>();
                if(!pattern.empty()) {
                    // simple lowercase matching
                    string t=txt, p=pattern;
                    for(auto &c:t) c=tolower((unsigned char)c);
                    for(auto &c:p) c=tolower((unsigned char)c);
                    // kmp
                    int m=p.size(), n=t.size();
                    if(m>0 && n>0){
                        vector<int> lps(m,0);
                        for(int i=1,len=0;i<m;){
                            if(p[i]==p[len]) lps[i++]=++len; else if(len) len=lps[len-1]; else lps[i++]=0;
                        }
                        int i=0,j=0;
                        while(i<n){
                            if(t[i]==p[j]){ i++; j++; if(j==m){ total++; j=lps[j-1]; } }
                            else { if(j) j=lps[j-1]; else i++; }
                        }
                    }
                }
            }
            cout<<"KMP_SCAN pattern='"<<pattern<<"' occurrences="<<total<<"\n";
        } else if(cu=="QUIT"){
            break;
        } else {
            // ignore unknown
        }
    }
    return 0;
}
