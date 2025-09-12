/* 12.20 2024 */

#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    bool canJump(vector<int>& nums) {
        int n = nums.size();
        int farthest = 0;

        for (int i = 0; i < n - 1; i++) {
            farthest = max(farthest, i + nums[i]);
            //碰到0
            if (farthest <= i) {
                return false;
            }
        }

        return farthest >= n - 1;
    }
};