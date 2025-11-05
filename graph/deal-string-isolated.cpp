#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <sstream>
/**
 * @brief 本次需要单独处理节点和边数 没有n m输入
 * 参考数组如下
4 5
1 1 0 0 0
1 1 0 0 0
0 0 1 0 0
0 0 0 1 1
*/


// using namespace std;

// std::vector<std::vector<int>> res;

void deal_data(std::vector<std::vector<int>>& res)
{
    // std::cin >> 
    std::string line;
    while(getline(std::cin, line)){
        if(line.empty())
            break;
        std::stringstream ss(line);
        int num = 0;
        std::vector<int> path;
        while(ss >> num){
            path.push_back(num);
        }
        res.push_back(path);
    }
}

int dir[4][2] = {0, 1, 1, 0, -1, 0, 0, -1}; //右下上左

void dfs(std::vector<std::vector<int>>& grid, std::vector<std::vector<bool>>& visited, int x, int y) 
{
    for(int i = 0; i < 4; i ++){
        int nextx = x + dir[i][0];
        int nexty = y + dir[i][1];
        if(nextx < 0 || nexty < 0 || nextx >= grid.size() || nexty >= grid[0].size())
            continue;   //跳出当前循环
        if(grid[nextx][nexty] == 1 && !visited[nextx][nexty] ){
            visited[nextx][nexty] = true; // =写成 = =了 没绷住
            dfs(grid, visited, nextx, nexty);
        }
    }
}

int main()
{
    std::vector<std::vector<int>> graph;
    deal_data(graph);
    int n = graph.size();
    int m = graph[0].size();
    std::vector<std::vector<bool>> is_visited(n, std::vector<bool>(m, false));
    // std::cout << "n : " << n << std::endl << "m : " << m << std::endl;

    int result = 0;
    for(int i = 0; i < n ; i++){
        for(int j = 0; j < m ; j++){
            if(graph[i][j] == 1 && is_visited[i][j] == false){ //找到孤岛
                is_visited[i][j] = true;
                result ++;
                dfs(graph, is_visited, i, j);
            }
        }
    }

    std::cout << result << std::endl;

    return 0;
}