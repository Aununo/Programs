#include <bits/stdc++.h>
using namespace std;

class RandomizedSet {
private:
    vector<int> elements;
    unordered_map<int, int> indices;

public:
    RandomizedSet() {
        
    }
    
    bool insert(int val) {
        if (indices.find(val) != indices.end()) {
            return false;
        }
        elements.push_back(val);
        indices[val] = elements.size() - 1;
        return true;
    }
    
    bool remove(int val) {
        if (indices.find(val) == indices.end()) {
            return false;
        }
        int last_element = elements.back();
        int idx = indices[val];

        elements[idx] = last_element;
        indices[last_element] = idx;

        elements.pop_back();
        indices.erase(val);

        return true;
    }
    
    int getRandom() {
        int random_index = rand() % elements.size();
        return elements[random_index];
    }
};

/**
 * Your RandomizedSet object will be instantiated and called as such:
 * RandomizedSet* obj = new RandomizedSet();
 * bool param_1 = obj->insert(val);
 * bool param_2 = obj->remove(val);
 * int param_3 = obj->getRandom();
 */