#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

class Solution {

};

int main() {
    ifstream inputFile("input.txt");
    ofstream outputFile("output.txt");

    if (!inputFile.is_open() || !outputFile.is_open()) {
        cerr << "Warning: cannot open files." << endl;
        return 1;
    }

    Solution solution;

    int t;
    inputFile >> t;

    while (t--) {
        int n;
        inputFile >> n;
        vector<int> nums(n);

        for (int i = 0; i < n; i++) {
            inputFile >> nums[i];
        }

        //int result = solution.jump(nums);
        //outputFile << result << endl; 
    }

    inputFile.close();
    outputFile.close();

    return 0;
}