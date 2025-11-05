#include <iostream>
#include <vector>
#include <list>

using namespace std;

vector<vector<int>> res;
vector<int> path;

void dfs(vector<list<int>>& graph, int x, int n){ //邻接表
    if(x == n) {
        res.push_back(path);
        return;
    }
    for(auto& y : graph[x]){
        path.push_back(y);
        dfs(graph, y, n);
        path.pop_back();
    }
}

int main()
{
    //deal input
    int n = 0;
    int m = 0;
    int s = 0;
    int t = 0;
    cin >> n >> m;

    vector<list<int>> graph(n + 1); //邻接表存储图
    while(m --){
        cin >> s >> t; //邻接表  s -> t
        graph[s].push_back(t);
    }
    path.push_back(1);
    dfs(graph, 1, n);
    //output
    if(res.empty()) cout << -1 << endl;

    int res_row = res.size();
    // cout << res_row << endl;

    for(int i = 0; i < res_row; i ++){
        for(int j = 0; j < res[i].size() - 1; j ++){
            cout << res[i][j] << " ";
        }
        cout << res[i][res[i].size() - 1] << endl; // last element
    }
    return 0;

}