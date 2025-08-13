/*
 * @lc app=leetcode.cn id=242 lang=cpp
 *
 * [242] 有效的字母异位词
 */


#include<bits/stdc++.h>
using namespace std;
// @lc code=start
class Solution {
public:
    bool isAnagram(string s, string t) {
        int letter[26] = {0};
        for(auto i :s){
            letter[i - 'a'] ++;
        }         
        for(auto j : t){
            letter[j - 'a'] --;
        }
        for(int i = 0; i < 26; i ++){
            if(letter[i] != 0)
                return false;
        }
        return true;
    }
};
// @lc code=end
int main()
{
    Solution solution;
    bool para1 = solution.isAnagram("anagram", "nagaram");
    bool para2 = solution.isAnagram("rat", "car");
    cout << para1 << " " << para2 << endl;
    return 0;
}
