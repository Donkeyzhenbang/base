/*
 * @lc app=leetcode.cn id=1 lang=cpp
 *
 * [1] 两数之和
 */
#include<bits/stdc++.h>
using namespace std;
//first: 代表键（key），即在 unordered_map 中的索引。
//second: 代表值（value），即与该键对应的存储值。
//例如，在这段代码中，map.insert(pair<int, int>(nums[i], i)); 这行将 nums[i] 作为键（key），i 作为值（value）存储进哈希表中。
// @lc code=start
/*
map.find(target - nums[i]) 用于查找哈希表中是否存在一个键值对，其键是 target - nums[i]。返回的 iter 是一个迭代器，它指向的是找到的 pair 类型对象：
如果找到了匹配的键值对，则 iter 指向该 pair，并通过 iter->first 获取该键，iter->second 获取对应的值。
*/
class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        std::unordered_map <int, int> map;
        for(int i = 0; i < nums.size(); i ++){
            //遍历当前元素，在map中寻找是否存在匹配
            auto iter = map.find(target - nums[i]);
            if(iter != map.end()){
                return {iter->second, i};
            }
            //没有匹配，把访问的元素下标加入
            map.insert(pair<int, int>(nums[i], i));
        }

        return {};
    }
};
// @lc code=end

int main()
{
    vector<int> nums = {2, 7, 11, 15};
    Solution solution;
    vector<int> res = solution.twoSum(nums, 17);
    for(int i = 0; i < int(res.size()); i ++) {
        cout << res[i] << " " ;
    }
    cout << endl;
    return 0;
}





