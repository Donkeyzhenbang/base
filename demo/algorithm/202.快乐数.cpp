/*
 * @lc app=leetcode.cn id=202 lang=cpp
 *
 * [202] 快乐数
 */


#include<bits/stdc++.h>
using namespace std;
// @lc code=start
// class Solution {
// public:
//     int getSum(int n) {
//         int sum = 0;
//         while(n){
//             sum += (n % 10) * (n % 10);
//             n /= 10;
//         }
//         return sum;
//     }
//     bool isHappy(int n) {
//         unordered_set<int> set;
//         while(1){
//             int sum = getSum(n);
//             if(sum == 1) return true;
//             if(set.find(sum) != set.end()){
//                 return false;
//             }
//             else{
//                 set.insert(sum);
//             }
//             n = sum;
//         }
//     }
// };

class Solution {
public:
    int getSum(int n) {
        int sum = 0;
        while(n){
            sum += (n % 10) * (n % 10);
            n /= 10;
        }
        return sum;
    }
    bool isHappy(int n) {
        int fast = n;
        int slow = n;
        do{
            slow = getSum(slow);
            fast = getSum(getSum(fast));
            cout << slow << " " << fast << endl;
        }while(slow != fast);
        return slow == 1;
    }
};
// @lc code=end

int main()
{
    Solution solution;
    int n;
    cin >> n ;
    cout << solution.isHappy(n) << endl;
    return 0;
}
