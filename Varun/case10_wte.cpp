#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using db = double;
const db INF = 1e18;

struct Edge { int u,v; db w; Edge(){} Edge(int a,int b,db c):u(a),v(b),w(c){} };
struct Graph {
    int n;
    vector<vector<pair<int,db>>> adj;
    vector<Edge> edges;
    Graph(int n_=0){ init(n_); }
    void init(int N){ n=N; adj.assign(n,{}); edges.clear(); }
    void addEdge(int u,int v,db w){ if(u<0||v<0||u>=n||v>=n) return; adj[u].push_back({v,w}); adj[v].push_back({u,w}); edges.emplace_back(u,v,w); }
    int size() const { return n; }
};

vector<db> dijkstra(const Graph &g,int s){
    int n=g.n;
    vector<db> dist(n, INF);
    priority_queue<pair<db,int>, vector<pair<db,int>>, greater<pair<db,int>>> pq;
    dist[s]=0; pq.push({0,s});
    while(!pq.empty()){
        auto cur=pq.top(); pq.pop();
        db cd=cur.first; int u=cur.second;
        if(cd!=dist[u]) continue;
        for(auto &pr: g.adj[u]){
            int v=pr.first; db w=pr.second;
            if(dist[u]+w < dist[v]){ dist[v]=dist[u]+w; pq.push({dist[v], v}); }
        }
    }
    return dist;
}

struct SegmentTree {
    int n;
    vector<int> cap;
    void init(int sz){
        n=1; while(n<sz) n<<=1;
        cap.assign(2*n, 0);
    }
    void build(const vector<int>&a){
        init(a.size());
        for(int i=0;i<(int)a.size();++i){ cap[n+i]=a[i]; }
        for(int i=n-1;i>0;--i) cap[i]=cap[i<<1] + cap[i<<1|1];
    }
    bool allocate_range(int l,int r,int need){
        if(l>r) return false;
        int total = range_sum(l,r);
        if(total < need) return false;
        int i=l+n, j=r+n;
        // greedy allocate left-to-right
        for(int idx=l; idx<=r && need>0; ++idx){
            int pos = idx + n;
            while(pos>0 && cap[pos]==0) pos >>= 1;
            if(pos==0) break;
            int available = cap[n + idx];
            int take = min(available, need);
            cap[n+idx] -= take;
            need -= take;
            for(int p=(n+idx)>>1; p; p>>=1) cap[p]=cap[p<<1]+cap[p<<1|1];
        }
        return need==0;
    }
    int range_sum(int l,int r){
        int L=l+n, R=r+n;
        int s=0;
        while(L<=R){
            if(L&1) s+=cap[L++];
            if(!(R&1)) s+=cap[R--];
            L>>=1; R>>=1;
        }
        return s;
    }
    int point_get(int idx){ return cap[n+idx]; }
    void point_add(int idx,int v){ int p=n+idx; cap[p]+=v; for(p>>=1;p;p>>=1) cap[p]=cap[p<<1]+cap[p<<1|1]; }
};

struct Feedstock {
    int id;
    db weight;
    db energy; // energy density
    db density; 
    int source_node;
    Feedstock(){}
    Feedstock(int i, db w, db e, int s):id(i),weight(w),energy(e),density(e/w),source_node(s){}
};

vector<Feedstock> generate_feedstocks(int m,int seed){
    vector<Feedstock> f;
    mt19937_64 rng(seed);
    uniform_real_distribution<db> wu(50,500);
    uniform_real_distribution<db> eu(10,200);
    uniform_int_distribution<int> ni(0,19);
    for(int i=0;i<m;i++){
        db w = wu(rng);
        db e = eu(rng);
        int s = ni(rng);
        f.emplace_back(i,w,e,s);
    }
    return f;
}

vector<Feedstock> greedy_knapsack(vector<Feedstock> items, db capacity){
    sort(items.begin(), items.end(), [](const Feedstock &a,const Feedstock &b){
        if(a.density==b.density) return a.energy > b.energy;
        return a.density > b.density;
    });
    vector<Feedstock> chosen;
    db used=0;
    for(auto &it: items){
        if(used + it.weight <= capacity){
            chosen.push_back(it);
            used += it.weight;
        }
    }
    return chosen;
}

vector<vector<Feedstock>> bin_pack_batches(vector<Feedstock> items, db cap_per_batch){
    sort(items.begin(), items.end(), [](const Feedstock &a,const Feedstock &b){ return a.weight > b.weight; });
    vector<vector<Feedstock>> batches;
    multiset<pair<db,int>> free_space; // remaining space, batch idx
    for(auto &it: items){
        bool placed=false;
        for(int i=0;i<batches.size(); ++i){
            db rem = cap_per_batch;
            for(auto &x: batches[i]) rem -= x.weight;
            if(rem + 1e-9 >= it.weight){
                batches[i].push_back(it);
                placed=true;
                break;
            }
        }
        if(!placed){
            batches.emplace_back(vector<Feedstock>{it});
        }
    }
    return batches;
}

struct Plant {
    int node;
    db capacity;
    int time_slots;
    vector<int> slot_cap;
    SegmentTree st;
    Plant(){}
    Plant(int n, db c, int t):node(n),capacity(c),time_slots(t){
        slot_cap.assign(time_slots, (int)capacity);
        st.build(slot_cap);
    }
    void init(int t, db cap){
        time_slots = t;
        capacity = cap;
        slot_cap.assign(time_slots, (int)capacity);
        st.build(slot_cap);
    }
    bool schedule_batch(int start_slot, int duration, int needed){
        int end = min(time_slots-1, start_slot + duration - 1);
        return st.allocate_range(start_slot, end, needed);
    }
};

struct TransportPlan {
    int truck_id;
    vector<int> path_nodes;
    db cost;
    int batch_id;
};

db route_cost_between_nodes(const Graph &g, int a,int b){
    auto d = dijkstra(g, a);
    if(b<0 || b>=d.size()) return INF;
    return d[b];
}

vector<int> route_nodes_sequence(const vector<int>&sources, const vector<int>&order){
    vector<int> seq;
    for(int id: order) seq.push_back(sources[id]);
    return seq;
}

vector<int> nearest_neighbour_order(const vector<int>&nodes,const vector<vector<db>>&D,int start_idx){
    int k=nodes.size();
    vector<char> used(k,0);
    vector<int> order;
    used[start_idx]=1;
    order.push_back(start_idx);
    for(int t=1;t<k;t++){
        int last = order.back();
        int best=-1; db bd=INF;
        for(int j=0;j<k;j++){
            if(used[j]) continue;
            db w = D[nodes[last]][nodes[j]];
            if(w < bd){ bd=w; best=j; }
        }
        used[best]=1;
        order.push_back(best);
    }
    return order;
}

db two_opt_improve(vector<int>&perm,const vector<int>&nodes,const vector<vector<db>>&D){
    int n=perm.size();
    if(n<4) return 0;
    bool improved=true;
    while(improved){
        improved=false;
        for(int i=1;i<n-2;i++){
            for(int j=i+1;j<n-1;j++){
                int A = nodes[perm[i-1]];
                int B = nodes[perm[i]];
                int C = nodes[perm[j]];
                int Dn = nodes[perm[j+1]];
                db d1 = D[A][B] + D[C][Dn];
                db d2 = D[A][C] + D[B][Dn];
                if(d2 + 1e-9 < d1){
                    reverse(perm.begin()+i, perm.begin()+j+1);
                    improved=true;
                }
            }
        }
    }
    db s=0;
    for(int i=0;i+1<n;i++) s+=D[nodes[perm[i]]][nodes[perm[i+1]]];
    return s;
}

vector<vector<db>> build_distance_matrix(const Graph &g){
    int n=g.n;
    vector<vector<db>> D(n, vector<db>(n, INF));
    for(int i=0;i<n;i++){
        auto d = dijkstra(g, i);
        for(int j=0;j<n;j++) D[i][j]=d[j];
    }
    return D;
}

struct BatchSchedule {
    int batch_id;
    int plant_node;
    int start_slot;
    int duration;
    vector<int> feed_ids;
    db transport_cost;
};

vector<BatchSchedule> create_schedules(
    const vector<vector<Feedstock>>&batches,
    const vector<Plant>&plants,
    const Graph &g,
    const vector<vector<db>>&D,
    int max_duration
){
    vector<BatchSchedule> schedules;
    int nb = batches.size();
    int np = plants.size();
    for(int i=0;i<nb;i++){
        db best_cost = INF;
        int best_plant = -1;
        for(int p=0;p<np;p++){
            db cost = 0;
            for(auto &fd: batches[i]){
                cost += route_cost_between_nodes(g, fd.source_node, plants[p].node);
            }
            if(cost < best_cost){ best_cost = cost; best_plant = p; }
        }
        if(best_plant==-1) continue;
        int needed = 0;
        for(auto &fd: batches[i]) needed += (int)round(fd.weight);
        int assigned_slot = -1;
        for(int s=0; s<=plants[best_plant].time_slots - 1; ++s){
            if(plants[best_plant].st.range_sum(s, min(plants[best_plant].time_slots-1, s + max_duration - 1)) >= needed){
                bool ok = plants[best_plant].st.allocate_range(s, min(plants[best_plant].time_slots-1, s + max_duration - 1), needed);
                if(ok){ assigned_slot = s; break; }
            }
        }
        if(assigned_slot==-1) assigned_slot = 0;
        BatchSchedule bs;
        bs.batch_id = i;
        bs.plant_node = plants[best_plant].node;
        bs.start_slot = assigned_slot;
        bs.duration = max_duration;
        for(auto &fd: batches[i]) bs.feed_ids.push_back(fd.id);
        bs.transport_cost = best_cost;
        schedules.push_back(bs);
    }
    return schedules;
}

vector<TransportPlan> build_transport_plans(const vector<BatchSchedule>&schedules,const vector<Feedstock>&items,const Graph&g){
    vector<TransportPlan> out;
    int tid=0;
    for(auto &bs: schedules){
        vector<int> sources;
        for(int id: bs.feed_ids) sources.push_back(items[id].source_node);
        vector<int> order(sources.size()); for(int i=0;i<sources.size();i++) order[i]=i;
        auto D = build_distance_matrix(g);
        auto perm = nearest_neighbour_order(sources, D, 0);
        two_opt_improve(perm, sources, D);
        vector<int> path;
        for(int id: perm) path.push_back(sources[id]);
        TransportPlan tp;
        tp.truck_id = tid++;
        tp.path_nodes = path;
        tp.cost = bs.transport_cost;
        tp.batch_id = bs.batch_id;
        out.push_back(tp);
    }
    return out;
}

vector<Feedstock> flatten_batches(const vector<vector<Feedstock>>&b){
    vector<Feedstock> out;
    for(auto &v: b) for(auto &x: v) out.push_back(x);
    return out;
}

vector<vector<Feedstock>> balance_batches(const vector<vector<Feedstock>>&batches, int target_batches){
    vector<Feedstock> all = flatten_batches(batches);
    sort(all.begin(), all.end(), [](const Feedstock&a,const Feedstock&b){ return a.weight > b.weight; });
    vector<vector<Feedstock>> out(target_batches);
    vector<db> cap(target_batches, 0);
    for(auto &it: all){
        int best = min_element(cap.begin(), cap.end()) - cap.begin();
        out[best].push_back(it);
        cap[best] += it.weight;
    }
    return out;
}

pair<vector<vector<Feedstock>>, db> plan_waste_to_energy_batching(
    vector<Feedstock> items,
    db digester_capacity,
    int max_batches,
    const Graph &g
){
    vector<vector<Feedstock>> batches = bin_pack_batches(items, digester_capacity);
    if(batches.size() > max_batches) batches = balance_batches(batches, max_batches);
    db total_energy = 0;
    for(auto &batch: batches){
        for(auto &it: batch) total_energy += it.energy;
    }
    return {batches, total_energy};
}

vector<Feedstock> sample_items_for_demo(int m){
    return generate_feedstocks(m, 2025);
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int mode;
    if(!(cin>>mode)) return 0;
    if(mode==0){
        int m=120;
        auto items = sample_items_for_demo(m);
        db digcap = 2000.0;
        int maxb = 12;
        Graph g(30);
        for(int i=0;i<30;i++) for(int j=i+1;j<min(30,i+6);j++) g.addEdge(i,j, (db)(1 + abs(i-j)));
        auto plan = plan_waste_to_energy_batching(items, digcap, maxb, g);
        auto &batches = plan.first;
        cout.setf(std::ios::fixed); cout<<setprecision(6);
        cout<<"BATCHES "<<batches.size()<<" TOTAL_ENERGY "<<plan.second<<"\n";
        for(int i=0;i<batches.size();i++){
            db w=0,e=0;
            for(auto &f: batches[i]){ w+=f.weight; e+=f.energy; }
            cout<<"B "<<i<<" W "<<w<<" E "<<e<<" CNT "<<batches[i].size()<<"\n";
        }
        return 0;
    }
    if(mode==1){
        int n,m;
        cin>>n>>m;
        vector<Feedstock> items;
        for(int i=0;i<m;i++){
            int id; db wt,en; int src;
            cin>>id>>wt>>en>>src;
            items.emplace_back(id,wt,en,src);
        }
        db digcap; int maxb;
        cin>>digcap>>maxb;
        Graph g(n);
        int edges; cin>>edges;
        for(int i=0;i<edges;i++){
            int u,v; db w; cin>>u>>v>>w; g.addEdge(u,v,w);
        }
        auto plan = plan_waste_to_energy_batching(items, digcap, maxb, g);
        auto batches = plan.first;
        auto allitems = flatten_batches(batches);
        auto D = build_distance_matrix(g);
        vector<Plant> plants;
        plants.emplace_back(); plants[0].init(24, 1000.0);
        auto schedules = create_schedules(batches, plants, g, D, 4);
        auto transports = build_transport_plans(schedules, allitems, g);
        cout<<batches.size()<<"\n";
        for(auto &bs: schedules) cout<<bs.batch_id<<" "<<bs.plant_node<<" "<<bs.start_slot<<" "<<bs.duration<<" "<<bs.transport_cost<<"\n";
        cout<<transports.size()<<"\n";
        for(auto &t: transports) cout<<t.truck_id<<" "<<t.cost<<" "<<t.path_nodes.size()<<"\n";
        return 0;
    }
    if(mode==2){
        int nodes, items_count;
        cin>>nodes>>items_count;
        vector<Feedstock> items;
        for(int i=0;i<items_count;i++){
            int id, src; db wt,en; cin>>id>>wt>>en>>src;
            items.emplace_back(id,wt,en,src);
        }
        db cap; cin>>cap;
        int maxb; cin>>maxb;
        int e; cin>>e;
        Graph g(nodes);
        for(int i=0;i<e;i++){ int u,v; db w; cin>>u>>v>>w; g.addEdge(u,v,w); }
        auto plan = plan_waste_to_energy_batching(items, cap, maxb, g);
        cout<<plan.first.size()<<"\n";
        for(auto &b: plan.first){
            db W=0,E=0;
            for(auto &it: b) W+=it.weight, E+=it.energy;
            cout<<b.size()<<" "<<W<<" "<<E<<"\n";
        }
        return 0;
    }
    return 0;
}
