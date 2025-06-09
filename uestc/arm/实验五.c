#include"s3c2440.h"

//定义寄存器地址
#define GPBCON               (*(volatile unsigned long *) 0x56000010)
#define GPBDAT               (*(volatile unsigned long *) 0x56000014)

void delay(){
	int timer=50000;
	while(timer--){
	}
}

int main(){
	GPBCON =0x0;         //清空GPBCON
	//GPBCON |=0X3C03FC;	//GPB0,1~4,9~10 useless
	//GPBCON |=0X015400;
	GPBCON |=  (0x1<<10) | (0x1<<12) | (0x1<<14) | (0x1<<16);   // 设置输出模式
	while(1){
		if(GPBDAT & 0X2){	//如果GPB1为高电平
		
		GPBDAT |=0X1E0;		//置 GPB5 ~ GPB8 为高电平
		GPBDAT &=~ 0X20;	//0010 0000	GPB5 low
		delay();
		
		GPBDAT |=0X1E0;
		GPBDAT &=~ 0x40;	//0100 0000 GPB6 low
		delay();
		
		GPBDAT|=0X1E0;
		GPBDAT&=~0X80;	//1000 0000 GPB7 low
		delay();
		
		GPBDAT|=0X1E0;
		GPBDAT&=~0X100;	//0001 0000 0000  GPB8 low
		delay();
		
		GPBDAT|=0X1E0; //01 1110 0000 5~8high
		delay();
	}
	else{
		GPBDAT|=0X1E0;
		GPBDAT&=~0X100;
		delay();
	
		GPBDAT|=0X1E0;
		GPBDAT&=~0X80;
		delay();
		
		GPBDAT |=0X1E0;
		GPBDAT &=~ 0x40;
		delay();
		
		GPBDAT |=0X1E0;
		GPBDAT &=~ 0X20;
		delay();
	
	  GPBDAT|=0X1E0;
		delay();
	}
	}
}