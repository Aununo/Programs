//求最大公因数（baby)
//递归...

#include <stdio.h>

int gcd(int a, int b) {
    if (b == 0) {
        printf("%d", a);
    } else {
        gcd(b, a % b);
    }

}

int main() {
    int a, b;
    scanf("%d %d", &a, &b);
    printf("%d\n", gcd(a,b));
    return 0;
}