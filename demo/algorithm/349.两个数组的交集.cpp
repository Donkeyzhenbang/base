/*
 * @lc app=leetcode.cn id=349 lang=cpp
 *
 * [349] 两个数组的交集
 */


#include<bits/stdc++.h>
using namespace std;
// @lc code=start
class Solution {
public:
    vector<int> intersection(vector<int>& nums1, vector<int>& nums2) {
        unordered_set<int> res;
        unordered_set<int> nums1_set(nums1.begin(), nums1.end());
        for(int num : nums2){
            if(nums1_set.find(num) != nums1_set.end())
            res.insert(num);
        }
        return vector<int>(res.begin(), res.end());

    }
};
// @lc code=end
int main()
{
    Solution solution;
    vector<int> nums1 = {4, 9, 5};
    vector<int> nums2 = {9, 4, 9, 8, 4};
    vector<int> ret = solution.intersection(nums1, nums2);
    for(int i = 0; i < ret.size(); i ++){
        cout << ret[i] << " " ;
    }
    cout << endl;
    return 0;
}
