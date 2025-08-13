/*
 * @lc app=leetcode.cn id=20 lang=cpp
 *
 * [20] 有效的括号
 */

#include<bits/stdc++.h>
using namespace std;
// @lc code=start
class Solution {
public:
    bool isValid(string s) {
        if(s.size() % 2) return false;
        stack<char> stk;
        for(char c : s){
            if(c == '(') stk.push(')');
            else if(c == '{') stk.push('}');
            else if(c == '[') stk.push(']');
            //c是右括号符号 若栈是空的或者没有对应要弹出的右括号
            else if(stk.empty() || c != stk.top()) return false;
            //括号对应上了 弹出即可
            else stk.pop();
        }
        //栈是空的，则匹配成功，否则匹配失败
        return stk.empty();
    }
};
// @lc code=end
int main()
{
    Solution solution;
    string s = "()[]{}";
    bool ret = solution.isValid(s);
    cout << ret << endl;
    return 0;
}
