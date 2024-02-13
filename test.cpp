#include <bits/stdc++.h>
 
using namespace std;
 
typedef long long ll;
typedef unsigned long long ull;
 
#define MASK(i) (1LL << (i))
#define GETBIT(mask, i) (((mask) >> (i)) & 1)
#define ALL(v) (v).begin(), (v).end()
 
ll max(ll a, ll b){return (a > b) ? a : b;}
ll min(ll a, ll b){return (a < b) ? a : b;}
 
ll LASTBIT(ll mask){return (mask) & (-mask);}
int pop_cnt(ll mask){return __builtin_popcountll(mask);}
int ctz(ll mask){return __builtin_ctzll(mask);}
int logOf(ll mask){return 63 - __builtin_clzll(mask);}
 
mt19937_64 rng(chrono::high_resolution_clock::now().time_since_epoch().count());
ll rngesus(ll l, ll r){return l + (ull) rng() % (r - l + 1);}
 
template <class T1, class T2>
    bool maximize(T1 &a, T2 b){
        if (a < b) {a = b; return true;}
        return false;
    }
 
template <class T1, class T2>
    bool minimize(T1 &a, T2 b){
        if (a > b) {a = b; return true;}
        return false;
    }
 
template <class T>
    void printArr(T& container, char separator = ' ', char finish = '\n'){
        for(auto item: container) cout << item << separator;
        cout << finish;
    }
 
template <class T>
    void remove_dup(vector<T> &a){
        sort(ALL(a));
        a.resize(unique(ALL(a)) - a.begin());
    }

struct FenwickTree{
    long n;
    vector<ll> a;

    FenwickTree(long _n){
        n = _n;
        a.resize(n+1);
    }

    void update(long i, long v){
        while(i <= n){
            a[i] += v;
            i += LASTBIT(i);
        }
    }

    ll get(long i){
        ll ans = 0;
        while(i > 0){
            ans += a[i];
            i -= LASTBIT(i);
        }
        return ans;
    }

    long binaryLift(ll v){
        long p = MASK(logOf(n)), idx = 0; ll sum = 0;
        while(p > 0){
            if (idx + p <= n && sum + a[idx + p] < v){
                idx += p;
                sum += a[idx];
            }
            p >>= 1;
        }
        return idx + 1;
    }
};

const long N = 1e5 + 69;
const long BLOCK = 350;

long n, m, q;
pair<long, long> jmp[N];

ll getBlock(ll x){return x / BLOCK;}

int main(void){
    ios::sync_with_stdio(0); cin.tie(0); cout.tie(0);

    cin >> n >> m;
    vector<long> a(n+1);
    for(long i = 1; i<=n; ++i) cin >> a[i];

    FenwickTree bit(n);
    for(long i = 1; i<=n; ++i) bit.update(i, a[i]);

    for(long i = n; i>=1; --i){
        long idx = bit.binaryLift(bit.get(i-1) + m + 1);
        if (idx > n || getBlock(i) != getBlock(idx)){
            jmp[i] = {0, i};
        }
        else{
            jmp[i] = jmp[idx];
            jmp[i].first++;
        }
    }

    long q; cin >> q; 
    while(q--){
        long type; cin >> type;
        if (type == 1){
            long l, r; cin >> l >> r;
            long x = l;
            long ans = 0;
            while(x <= r){
                if (jmp[x].second <= r && jmp[x].first > 0){
                    ans += jmp[x].first;
                    x = jmp[x].second;
                    continue;
                }
                long idx = bit.binaryLift(bit.get(x-1) + m + 1);
                x = idx;
                ans++;
                continue;
            }
            cout << ans << "\n";
        }
        else{
            long u, x; cin >> u >> x;
            long diff = x - a[u];
            a[u] = x;
            bit.update(u, diff);

            long boy_love = getBlock(u);
            long l = boy_love * BLOCK, r = (boy_love + 1) * BLOCK - 1;
            maximize(l, 1);
            minimize(r, n);

            ll cur_sum = 0;
            long j = r;
            for(long i = r; i>=l; --i){
                cur_sum += a[i];
                while(cur_sum > m){
                    cur_sum -= a[j--];
                }
                if (j < r){
                    jmp[i] = jmp[j+1];
                    jmp[i].first++;
                }
                else jmp[i] = {0, i};
            }
        }
    }

    return 0;
}
