#include <stdio.h>

int main() {
    float amount;
    printf("Enter an amount: ");
    scanf("%f", &amount);
    float amount_taxed = amount * 1.05;
    printf("with tax added: $%.2f", amount_taxed);
    return 0;
}
