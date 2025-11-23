#include <iostream>
#include <vector>
using namespace std;

int main()
{
    int m = 0;
    int n = 0;
    cin >> m >> n;// m 物品 n 空间
    vector<vector<int>> dp(m, vector<int>(n + 1, 0));
    vector<int> weight(m, 0);
    vector<int> value(m, 0);
    for(int i = 0; i < m ; i++){
        cin >> weight[i];
    }
    for(int i = 0; i < m; i ++){
        cin >> value[i];
    }
    //第一行先初始化
    for(int i = weight[0]; i <= n; i ++){
        dp[0][i] = value[0];
    }

    //从第二行开始遍历
    for(int i = 1; i < m ; i ++){
        for(int j = 0; j <= n; j ++){
            if(j >= weight[i])
                dp[i][j] = max(dp[i - 1][j], dp[i - 1][j - weight[i]] + value[i]);
            else
                dp[i][j] = dp[i - 1][j];
            cout << dp[i][j] << " ";
        }
        cout << endl;
    }
    cout << dp[m - 1][n] << endl;

}