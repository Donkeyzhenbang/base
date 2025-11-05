#include <iostream>
#include <vector>

using namespace std;

int dir[4][2] = {0, 1, 1, 0, -1, 0, 0, -1}; //右 下 上 左
void dfs(vector<vector<int>>& grid, vector<vector<bool>>& visited, int x, int y)
{
    for(int i = 0; i < 4; i ++){
        int nextx = x + dir[i][0];
        int nexty = y + dir[i][1];
        if(nextx < 0 || nexty < 0 || nextx >= grid.size() || nexty >= grid[0].size()) continue;
        if(!visited[nextx][nexty] && grid[nextx][nexty] == 1){
            visited[nextx][nexty] = true;
            dfs(grid, visited, nextx, nexty);
        }
    }
}

int main()
{
    //deal input
    int m = 0;
    int n = 0;
    cin >> n >> m;
    vector<vector<int>> graph(n, vector<int>(m, 0));
    for(int i = 0; i < n ; i++){
        for(int j = 0; j < m ; j ++){
            cin >> graph[i][j];
        }
    }

    vector<vector<bool>> is_visited(n, vector<bool>(m, false));
    int result = 0;

    //dfs
    for(int i = 0; i < n; i ++){
        for(int j = 0; j < m; j ++){
            if(!is_visited[i][j] && graph[i][j] == 1){
                is_visited[i][j] = true;
                result ++;
                dfs(graph, is_visited, i, j);
            }
        }
    }


    //output
    cout << result << endl;

}