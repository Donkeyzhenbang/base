/*
 * @lc app=leetcode.cn id=454 lang=cpp
 *
 * [454] 四数相加 II
 */


#include<bits/stdc++.h>
using namespace std;
// @lc code=start
class Solution {
public:
    int fourSumCount(vector<int>& nums1, vector<int>& nums2, vector<int>& nums3, vector<int>& nums4) {
        unordered_map<int, int> umap; //key:1+2值 val:1+2cnt
        for(int a : nums1){
            for(int b : nums2){
                umap[a + b] ++;
            }
        }

        int count = 0;
        for(int c : nums3){
            for(int d : nums4) {
                if(umap.find(0 - (c + d)) != umap.end()) {
                    count += umap[0 - (c + d)];
                }
            }
        }
        return count;

    }
};
// @lc code=end

int main()
{
    // Solution solution;
    // vector<int> nums1 = {1,2};
    // vector<int> nums2 = {-2, -1};
    // vector<int> nums3 = {-1, 2};
    // vector<int> nums4 = {0, 2};
    // int result = solution.fourSumCount(nums1, nums2, nums3, nums4);
    // cout << result << endl;

    Solution solution;
    vector<int> nums1;
    vector<int> nums2;
    vector<int> nums3;
    vector<int> nums4; 
    int size1;
    // 输入nums1的元素个数
    cout << "请输入nums1的元素个数: ";
    cin >> size1;
    cout << "请依次输入nums1的元素: ";
    for (int i = 0; i < size1; ++i) {
        int num;  // 在这里初始化用于接收每次输入的变量
        cin >> num;
        nums1.push_back(num);
    }

    int size2;
    // 输入nums2的元素个数
    cout << "请输入nums2的元素个数: ";
    cin >> size2;
    cout << "请依次输入nums2的元素: ";
    for (int i = 0; i < size2; ++i) {
        int num;
        cin >> num;
        nums2.push_back(num);
    }

    int size3;
    // 输入nums3的元素个数
    cout << "请输入nums3的元素个数: ";
    cin >> size3;
    cout << "请依次输入nums3的元素: ";
    for (int i = 0; i < size3; ++i) {
        int num;
        cin >> num;
        nums3.push_back(num);
    }

    int size4;
    // 输入nums4的元素个数
    cout << "请输入nums4的元素个数: ";
    cin >> size4;
    cout << "请依次输入nums4的元素: ";
    for (int i = 0; i < size4; ++i) {
        int num;
        cin >> num;
        nums4.push_back(num);
    }

    int result = solution.fourSumCount(nums1, nums2, nums3, nums4);
    cout << result << endl;
    return 0;
}

//!这里默认nums1已经有足够的空间来存放a个元素了，但实际上在执行这段代码前，nums1只是一个空的vector（按照你之前整体修改为手动输入的代码逻辑来看），它并没有预先分配好对应数量的空间来存放元素。
//!所以当你直接通过nums1[i]去访问元素位置进行写入操作时，就超出了vector当前合法的范围（因为其初始大小为 0），这属于典型的越界访问，这种非法的内存访问行为很可能导致段错误，
//!操作系统检测到程序访问了不该访问的内存区域，就会终止程序并报错。
//!类似地，对于nums2、nums3和nums4的读取循环也存在同样的越界访问隐患，因为它们初始也都是空的vector，没有提前分配好能对应b、c、d个元素的空间。
        // 读取nums1的元素
    // for (int i = 0; i < a; ++i) {
    //     cin >> nums1[i];
    // }
    // 读取nums2的元素
    // for (int i = 0; i < b; ++i) {
    //     cin >> nums2[i];
    // }
    // 读取nums3的元素
    // for (int i = 0; i < c; ++i) {
    //     cin >> nums3[i];
    // }
    // 读取nums4的元素
    // for (int i = 0; i < d; ++i) {
    //     cin >> nums4[i];
    // }