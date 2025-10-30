#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


uint32_t rot(uint32_t value, unsigned int shift) {
    return (value >> shift) | (value << (32 - shift));
}

bool can_be_encoded(uint32_t K) {
    for (int rotated_imm = 0; rotated_imm < 16; rotated_imm++) {
        uint32_t rotated = rot(K, rotated_imm * 2);
        if ((rotated & ~0xFF) ==  0) {
            return true;
        }
    }
    return false;
}

void find_nearest_num(uint32_t K, uint32_t *m, uint32_t *n) {
    *m = 0;
    *n = 0;

    for (uint32_t i = K - 1; i > 0; i--) {
        if (can_be_encoded(i) && can_be_encoded(K - i)) {
            *m = i;
            break;
        }
    }

    for (uint32_t i = K + 1; i < UINT32_MAX; i++) {
        if (can_be_encoded(i) && can_be_encoded(K - i)) {
            *n = i;
            break;
        }
    }
}


int main() {
    uint32_t K;
    scanf("%u", &K);

    if (can_be_encoded(K)) {
        printf("-1\n");
        return 0;
    } 

    uint32_t m, n;
    find_nearest_num(K, &m, &n);

    if (m > 0 && n > 0) {
        printf("%u %u\n", m, n);
    } else {
        printf("-2\n");
    }

    return 0;
}