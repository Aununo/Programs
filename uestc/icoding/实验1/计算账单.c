#include <stdio.h>

int main() {
    int amount;
    int num_20, num_10, num_5, num_1;
    
    printf("Enter a dollar amount: ");
    scanf("%d",&amount);
    
    num_20=amount/20;
    printf("$20 bills:%d\n",num_20);
    
    num_10=amount%20/10;
    printf("$10 bills:%d\n",num_10);
    
    num_5=amount%20%10/5;
    printf("$5 bills:%d\n",num_5);
    
    num_1=amount%20%10%5;
    printf("$1 bills:%d\n",num_1);
    
    return 0;
}
