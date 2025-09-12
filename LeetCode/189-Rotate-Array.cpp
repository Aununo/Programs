/* 12.16 2024 */

#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    void rotate(vector<int>& nums, int k) {
        int n = nums.size();
        k = k % n;
        // Step 1: Reverse the entire array
        reverse(nums.begin(), nums.end());  
        // Step 2: Reverse the first k elements
        reverse(nums.begin(), nums.begin() + k);
        // Step 3: Reverse the remaining elements
        reverse(nums.begin() + k, nums.end());
    }
};


