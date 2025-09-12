#include <stdio.h>

#define ll long long

double qpow(double a, ll b) {
    double res = 1.0;
    while (b) {
        if (b & 1) res *= a;
        a *= a;
        b >>= 1;
    }
    return res;
}

