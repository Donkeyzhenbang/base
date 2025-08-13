/*
 * @lc app=leetcode.cn id=150 lang=cpp
 *
 * [150] 逆波兰表达式求值
 */

// @lc code=start

#include<bits/stdc++.h>
using namespace std;
class Solution {
public:
    int evalRPN(vector<string>& tokens) {
        stack<long long> st;
        for(int i = 0; i < tokens.size(); i ++){
            if(tokens[i] == "+" || tokens[i] == "-" || tokens[i] == "*" || tokens[i] == "/"){
                long long num1 = st.top();
                st.pop();
                long long num2 = st.top();
                st.pop();
                if(tokens[i] == "+") st.push(num2 + num1);
                if(tokens[i] == "-") st.push(num2 - num1);
                if(tokens[i] == "*") st.push(num2 * num1);
                if(tokens[i] == "/") st.push(num2 / num1);
            }else{
                st.push(stoll(tokens[i]));
            }
        }
        long long res = st.top();
        return res;
    }
};

// @lc code=end
int main()
{
    Solution solution;
    vector<string> token;
    string input;
    cout << "请输入逆波兰表达式（以空格分隔操作数和操作符）：";
    getline(cin, input);

    stringstream ss(input);
    string element;
    while(ss >> element){
        // 每次循环，element会依次被赋值为 "3"、"4"、"+"
        token.push_back(element);
    }
    cout << solution.evalRPN(token) << endl;
    return 0;
}
