/*
 * @lc app=leetcode.cn id=232 lang=cpp
 *
 * [232] 用栈实现队列
 */


#include<bits/stdc++.h>
using namespace std;
// @lc code=start
class MyQueue {
public:
    stack<int> stack_in;
    stack<int> stack_out;
    MyQueue() {
        
    }
    
    void push(int x) {
        stack_in.push(x);
    }
    
    int pop() {
        if(stack_out.empty()){
            while(!stack_in.empty()){
                stack_out.push(stack_in.top());
                stack_in.pop();
            }
        }
        int result = stack_out.top();
        stack_out.pop();
        return result;

    }
    
    int peek() {
        int res = this->pop();
        stack_out.push(res);
        return res;
    }
    
    bool empty() {
        return stack_in.empty() && stack_out.empty();
    }
};

/**
 * Your MyQueue object will be instantiated and called as such:
 * MyQueue* obj = new MyQueue();
 * obj->push(x);
 * int param_2 = obj->pop();
 * int param_3 = obj->peek();
 * bool param_4 = obj->empty();
 */
// @lc code=end
int main()
{
    //动态分配 记得delete
    MyQueue* queue_demo = new MyQueue();
    //栈式分配 
    // MyQueue queue_demo;
    queue_demo->push(1);
    queue_demo->push(2);
    int param_peek = queue_demo->peek();
    int param_pop = queue_demo->pop();
    bool param_empty = queue_demo->empty();
    cout << "param_peek" << param_peek << endl;
    cout << "param_pop" << param_pop << endl;
    cout << "param_empty" << param_empty << endl;
    return 0;
}

