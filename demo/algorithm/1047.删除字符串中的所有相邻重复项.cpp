/*
 * @lc app=leetcode.cn id=1047 lang=cpp
 *
 * [1047] 删除字符串中的所有相邻重复项
 */

// @lc code=start

#include<bits/stdc++.h>
using namespace std;
class Solution {
public:
    string removeDuplicates(string s) {
        stack<char> st;
        for(char c : s){
            if(st.empty() || c != st.top()){
                st.push(c);
            }
            else{
                st.pop();
            }
        }
        string result = "";
        while(!st.empty()){
            result += st.top();
            st.pop();
        }
        reverse(result.begin(), result.end());
        return result;
    }
};
int main()
{
    Solution solution;
    string s = "abbaca";
    cout << solution.removeDuplicates(s) << endl;
    return 0;
}
// @lc code=end

