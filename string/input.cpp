#include <iostream>
#include <string>
#include <sstream>
#include <vector>
/**
 * @brief 处理未知大小字符串输入
 * 
 *
 * @return int 
 */

 /* 输入示例
 4 5
1 1 0 0 0
1 1 0 0 0
0 0 1 0 0
0 0 0 1 1
 */
// int main()
// {
//     std::vector<std::vector<int>> grid;
//     std::string line;
//     while(getline(std::cin, line)){
//         if(line.empty()){
//             break;
//         }
//         std::vector<int> row;
//         std::stringstream ss(line);
//         int num = 0;
//         while(ss >> num){
//             row.push_back(num);
//         }
//         grid.push_back(row);

//     }
//     std::cout << "row : " << grid.size() << std::endl;
//     std::cout << "column : " << grid[0].size() << std::endl;
// }

void deal_file(){
    std::vector<std::vector<int>> grid;
    std::string line;
    while(getline(std::cin, line)){
        if(line.empty())
            break;
        std::stringstream ss(line);
        int num = 0;
        std::vector<int> row;
        while(ss >> num){
            row.push_back(num);
        }
        grid.push_back(row);
    }
    int grid_row = grid.size();
    int grid_column = grid[0].size();
    std::cout << "grid_row : " << grid_row << "\ngrid_column ： " << grid_column << std::endl;
    for(int i = 0; i < grid_row; i ++){
        for(int j = 0; j < grid_column; j ++){
            std::cout << grid[i][j] <<  " " ;
        }
        std::cout << std::endl;
    }
    
}

int main()
{
    deal_file();
    return 0;
}
