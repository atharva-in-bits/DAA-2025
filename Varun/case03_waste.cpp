#include <bits/stdc++.h>
using namespace std;
using ll = long long;
struct Node{int id; double x,y; double waste; int urgency; int region;};
double euclid(const Node&a,const Node&b){double dx=a.x-b.x,dy=a.y-b.y;return sqrt(dx*dx+dy*dy);}
vector<string> split_csv(const string&s){
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
    for(auto &t:out){
        size_t a=0,b=t.size();
        while(a<b && isspace((unsigned char)t[a])) ++a;
        while(b>a && isspace((unsigned char)t[b-1])) --b;
        t = t.substr(a,b-a);
    }
    return out;
}
int parse_int(const string&s){ if(s.empty()) return 0; try{return stoi(s);}catch(...){return 0;} }
double parse_double(const string&s){ if(s.empty()) return 0.0; try{return stod(s);}catch(...){return 0.0;} }
vector<int> kmeans_assign(const vector<Node>&nodes,int k,int iterations=50){
    int n=nodes.size();
    vector<int> assign(n,0);
    if(k<=0) k=1;
    vector<pair<double,double>> center(k);
    random_device rd; mt19937_64 rng(rd());
    uniform_int_distribution<int> dist(0,n-1);
    for(int i=0;i<k;++i){ int idx=dist(rng); center[i]={nodes[idx].x,nodes[idx].y}; }
    for(int it=0; it<iterations; ++it){
        for(int i=0;i<n;++i){
            double best=1e100; int bi=0;
            for(int j=0;j<k;++j){
                double dx=nodes[i].x-center[j].first,dy=nodes[i].y-center[j].second;
                double d=dx*dx+dy*dy;
                if(d<best){ best=d; bi=j; }
            }
            assign[i]=bi;
        }
        vector<double> sx(k,0), sy(k,0); vector<int> cnt(k,0);
        for(int i=0;i<n;++i){ int c=assign[i]; sx[c]+=nodes[i].x; sy[c]+=nodes[i].y; cnt[c]++; }
        bool moved=false;
        for(int j=0;j<k;++j){
            if(cnt[j]>0){
                double nx=sx[j]/cnt[j], ny=sy[j]/cnt[j];
                if(fabs(nx-center[j].first)>1e-9 || fabs(ny-center[j].second)>1e-9) moved=true;
                center[j].first=nx; center[j].second=ny;
            }
        }
        if(!moved) break;
    }
    return assign;
}
vector<int> nn_tour(const vector<int>&ids,const vector<Node>&nodes){
    if(ids.empty()) return {};
    int m=ids.size();
    vector<int> tour; tour.reserve(m);
    vector<char> used(m,0);
    int cur=0;
    tour.push_back(ids[cur]); used[cur]=1;
    for(int step=1; step<m; ++step){
        double best=1e200; int bi=-1;
        for(int j=0;j<m;++j) if(!used[j]){
            double d=euclid(nodes[ids[cur]], nodes[ids[j]]);
            if(d<best){ best=d; bi=j; }
        }
        if(bi==-1) break;
        cur=bi; used[cur]=1; tour.push_back(ids[cur]);
    }
    return tour;
}
double tour_length(const vector<int>&tour,const vector<Node>&nodes){
    if(tour.empty()) return 0;
    double s=0;
    for(size_t i=0;i+1<tour.size();++i) s+=euclid(nodes[tour[i]], nodes[tour[i+1]]);
    s+=euclid(nodes[tour.back()], nodes[tour.front()]);
    return s;
}
void two_opt(vector<int>&tour,const vector<Node>&nodes){
    int n=tour.size();
    if(n<4) return;
    bool improved=true;
    while(improved){
        improved=false;
        for(int i=0;i<n-1 && !improved;++i){
            for(int k=i+2;k<n && !improved;++k){
                int a=tour[i], b=tour[(i+1)%n], c=tour[k% n], d=tour[(k+1)%n];
                double before = euclid(nodes[a],nodes[b]) + euclid(nodes[c],nodes[d]);
                double after = euclid(nodes[a],nodes[c]) + euclid(nodes[b],nodes[d]);
                if(after + 1e-9 < before){
                    reverse(tour.begin()+i+1, tour.begin()+k+1);
                    improved=true;
                }
            }
        }
    }
}
vector<vector<int>> split_by_capacity(const vector<int>&tour,const vector<Node>&nodes,double capacity){
    vector<vector<int>> parts;
    double cur_load=0;
    vector<int> cur_part;
    for(int id : tour){
        double w = nodes[id].waste;
        if(cur_load + w > capacity && !cur_part.empty()){
            parts.push_back(cur_part);
            cur_part.clear();
            cur_load=0;
        }
        cur_part.push_back(id);
        cur_load += w;
    }
    if(!cur_part.empty()) parts.push_back(cur_part);
    return parts;
}
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    vector<string> lines;
    string row;
    while(getline(cin,row)) lines.push_back(row);
    if(lines.empty()) return 0;
    int first=0; while(first<(int)lines.size() && trim(lines[first]).empty()) first++;
    if(first>=(int)lines.size()) return 0;
    auto header = split_csv(lines[first]);
    unordered_map<string,int> hmap;
    for(int i=0;i<(int)header.size();++i){ string t=header[i]; for(auto &c:t) c=tolower((unsigned char)c); hmap[t]=i; }
    vector<Node> nodes;
    for(int i=first+1;i<(int)lines.size();++i){
        string s = lines[i];
        if(trim(s).empty()) continue;
        auto f = split_csv(s);
        string cmd = "";
        if(hmap.count("command")) cmd = f[hmap["command"]];
        if(cmd.empty()){
            if(hmap.count("node_id")) cmd = "NODE";
        }
        for(auto &c:cmd) c=toupper((unsigned char)c);
        if(cmd=="NODE"){
            int nid = 0; double x=0,y=0; double waste=0; int urg=1;
            if(hmap.count("node_id")) nid = parse_int(f[hmap["node_id"]]);
            if(hmap.count("x")) x = parse_double(f[hmap["x"]]);
            if(hmap.count("y")) y = parse_double(f[hmap["y"]]);
            if(hmap.count("waste_volume")) waste = parse_double(f[hmap["waste_volume"]]);
            if(hmap.count("urgency")) urg = parse_int(f[hmap["urgency"]]);
            Node n; n.id = nid; n.x = x; n.y = y; n.waste = waste; n.urgency = urg; n.region = -1;
            nodes.push_back(n);
        }
    }
    int n = nodes.size();
    if(n==0) return 0;
    int k = max(1, (int)round(sqrt((double)n)));
    auto assign = kmeans_assign(nodes, k, 50);
    for(int i=0;i<n;++i) nodes[i].region = assign[i];
    vector<vector<int>> clusters(k);
    for(int i=0;i<n;++i) clusters[nodes[i].region].push_back(i);
    double truck_capacity = 300.0;
    for(int ci=0;ci<k;++ci){
        auto &ids = clusters[ci];
        if(ids.empty()) continue;
        vector<int> tour = nn_tour(ids, nodes);
        two_opt(tour, nodes);
        auto parts = split_by_capacity(tour, nodes, truck_capacity);
        cout<<"Cluster "<<ci<<" size "<<ids.size()<<" routes "<<parts.size()<<"\n";
        for(size_t pi=0; pi<parts.size(); ++pi){
            cout<<"Route "<<ci<<"."<<pi<<":";
            for(int v: parts[pi]) cout<<" "<<nodes[v].id;
            cout<<"\n";
        }
    }
    return 0;
}
