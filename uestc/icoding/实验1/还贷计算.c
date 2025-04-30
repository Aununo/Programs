int main()  {

    float loan_amount, annual_interest_rate, monthly_payment;
    float balance_1,balance_2,balance_3;
    
    scanf("%f", &loan_amount);
    scanf("%f", &annual_interest_rate);
    scanf("%f", &monthly_payment);
    
    float monthly_interest_rate = annual_interest_rate / 100 / 12;
    
    float monthly_interest_1 = loan_amount * monthly_interest_rate;
    balance_1 = loan_amount - monthly_payment + monthly_interest_1;
    printf("Balance remaining after first payment:%.2f\n",balance_1);
    
    float monthly_interest_2 = balance_1 * monthly_interest_rate;
    balance_2 = balance_1 - monthly_payment + monthly_interest_2;
    printf("Balance remaining after second payment:%.2f\n",balance_2);
    
    float monthly_interest_3 = balance_2 * monthly_interest_rate;
    balance_3 = balance_2 - monthly_payment + monthly_interest_3;
    printf("Balance remaining after third payment:%.2f\n",balance_3);
    
    return 0;
}
