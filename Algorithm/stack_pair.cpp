#include <iostream>
#include <string>
#include <stack>

bool isValid(const std::string& s) {
    std::stack<char> stk;

    for (char c : s) {
        if (c == '(' || c == '[' || c == '{') {
            stk.push(c);
        } else {
            if (stk.empty()) {
                return false;
            }

            char top_char = stk.top();
            if ((c == ')' && top_char == '(') ||
                (c == ']' && top_char == '[') ||
                (c == '}' && top_char == '{')) {
                stk.pop();
            } else {
                return false;
            }
        }
    }

    return stk.empty();
}

int main() {
    // --- 测试代码 ---
    std::string test_strings[] = {
        "()",
        "()[]{}",
        "([{}])",
        "(]",
        "([)]",
        "{[{",
        ""
    };

    for (const std::string& str : test_strings) {
        // 使用 std::cout 进行输出，并用三元运算符判断结果
        std::cout << "'" << str << "' 是合法的吗? " 
                  << (isValid(str) ? "是" : "否") << std::endl;
    }

    return 0;
}