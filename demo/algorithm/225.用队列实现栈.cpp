/*
 * @lc app=leetcode.cn id=225 lang=cpp
 *
 * [225] 用队列实现栈
 */


#include<bits/stdc++.h>
using namespace std;
// @lc code=start
// class MyStack {
// public:
//     queue<int> q1;
//     queue<int> q2;
//     MyStack() {
        
//     }
    
//     void push(int x) {
//         q1.push(x);
//     }
    
//     int pop() {
//         int len = q1.size();
//         len --;
//         while(len --){
//             q2.push(q1.front());
//             q1.pop();
//         }
//         int res = q1.front();
//         q1.pop();
//         q1 = q2;
//         while(!q2.empty()){
//             q2.pop();
//         }
//         return res;
//     }
    
//     int top() {
//         int res = this->pop();
//         this->push(res);
//         return res;
//     }
    
//     bool empty() {
//         return q1.empty();
//     }
// };

class MyStack {
public:
    queue<int> que;
    MyStack() {
        
    }
    
    void push(int x) {
        que.push(x);
    }
    
    int pop() {
        int len = que.size();
        len --;
        while(len --){
            que.push(que.front());
            que.pop();
        }
        int res = que.front();
        que.pop();
        return res;
    }
    
    int top() {
        int res = this->pop();
        que.push(res);
        return res;
    }
    
    bool empty() {
        return que.empty();
    }
};

/**
 * Your MyStack object will be instantiated and called as such:
 * MyStack* obj = new MyStack();
 * obj->push(x);
 * int param_2 = obj->pop();
 * int param_3 = obj->top();
 * bool param_4 = obj->empty();
 */
// @lc code=end

int main()
{
    MyStack* stk = new MyStack();
    stk->push(1);
    stk->push(2);
    stk->push(3);
    int para_1 = stk->pop();
    int para_2 = stk->top();
    int para_3 = stk->empty();
    cout << "para_1" << para_1 << endl << "para_2" << para_2 << endl << "para_3" << para_3 << endl;
    return 0;
}