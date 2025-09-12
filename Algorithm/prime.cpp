//判断一个数是否是质数(baby)


#include <stdio.h>
#include <stdbool.h>
#include <math.h>

bool is_prime(int n) {
    if (n <= 1) {
        return false;
    }

    if (n <= 2) {
        return true;
    }

    if (n % 2 == 0) {
        return false;
    }
// point
    int limit = (int)sqrt(n);

    for (int i = 3; i <= limit; i+=2) {
        if (n % i == 0) {
            return false;
        }
    }

    return true;
}

int main() {
    // --- 测试代码 ---
    int test_numbers[] = {29, 91, 1, 2, 8, 997}; // 997是一个较大的质数
    int num_tests = sizeof(test_numbers) / sizeof(test_numbers[0]);

    for (int i = 0; i < num_tests; i++) {
        int num = test_numbers[i];

        printf("%d 是质数吗? %s\n", num, is_prime(num) ? "是" : "否");
    }

    return 0;
}