/* 12.15 2024 */

#include <vector>
#include <algorithm>
#include <unordered_map>
using namespace std;

class Solution {
public:
    int majorityElement(vector<int>& nums) {
        int count = 0, candidate = 0;

        // Boyer-Moore Voting Algorithm
        for (int num : nums) {
            if (count == 0) {
                candidate = num;
            } else if (num == candidate) {
                count++;
            } else {
                count--;
            }
        }
        return candidate;
    }
}; // O(n) time, O(1) space


class Solution2 {
public:
    int majorityElement(vector<int>& nums) {
        sort(nums.begin(), nums.end());
        return nums[nums.size() / 2];
    }
}; // O(n log n) time, O(1) space


class Solution3 {
public:
    int majorityElement(vector<int>& nums) {
        unordered_map<int, int> count;
        for (int num : nums) {
            count[num]++;
            if (count[num] > nums.size() / 2) {
                return num;
            }
        }
        return 0;
    }
}; // O(n) time, O(n) space