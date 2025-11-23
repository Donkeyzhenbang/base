#include <iostream>
#include <vector>
#include <queue>

using namespace std;

int dir[4][2] = {0, 1, 1, 0 ,-1, 0, 0, -1};

void bfs(vector<vector<int>>& graph, vector<vector<bool>>& visited, int x, int y)
{
    queue<pair<int, int>> que;
    que.push({x, y}); //起始节点加[入队列
    visited[x][y] = true;
    while(!que.empty()){
        pair<int, int> cur = que.front();
        que.pop();
        int curx = cur.first;
        int cury = cur.second;
        for(int i = 0; i < 4; i ++){
            int nextx = curx + dir[i][0];
            int nexty = cury + dir[i][1];
            if(nextx < 0 || nexty < 0 || nextx >= graph.size() || nexty >= graph[0].size())
                continue;
            if(visited[nextx][nexty] == false && graph[nextx][nexty] == 1){
                que.push({nextx, nexty});
                visited[nextx][nexty] = true;
                // bfs(graph, visited, nextx, nexty);
            }

        }
    

    }
}

// void dfs(vector<vector<int>>& graph, vector<vector<bool>>& visited, int x, int y)
// {
//     for(int i = 0; i < 4; i ++){
//         int nextx = x + dir[i][0];
//         int nexty = y + dir[i][1];
        // if(nextx < 0 || nexty < 0 || nextx >= graph.size() || nexty >= graph[0].size())
        //     continue;
//         if(visited[nextx][nexty] == false && graph[nextx][nexty] == 1){
//             visited[nextx][nexty] = true;
//             dfs(graph, visited, nextx, nexty);
//         }
//     }
// }


int main()
{
    int n = 0;
    int m = 0;
    int res = 0;
    cin >> n >> m;
    vector<vector<int>> graph(n, vector<int>(m, 0));
    for(int i = 0; i < n ; i++){
        for(int j = 0; j < m ; j++){
            cin >> graph[i][j];
        }
    }
    vector<vector<bool>> visited(n, vector<bool>(m, 0));

    for(int i = 0; i < n; i ++){
        for(int j = 0; j < m; j ++){
            if(graph[i][j] == 1 && visited[i][j] == false){
                visited[i][j] = true;
                res ++;
                // dfs(graph, visited, i, j);
                bfs(graph, visited, i, j);
            }
        }
    }
    cout << res << endl;
}