#include <stdlib.h>
#include <string.h>
#include <S3C2440.H>  
#include "2440addr.h"

#define U32 unsigned int

static void __irq Key_ISR(void); 

void delay()  
{  
	int i,j;  
	for(i=0;i<10000;i++)  
	{  
		for(j=0;j<50;j++);  
	}  
}  

void key_init(void)
{
	//设置EINT[8]-EINT[13]为中断，设置为输出模式 GPG0-5，设置为中断模式
	//初始化中断端口
	//把对应操作位置（GPB5678)为设置为0
	GPGCON &= 0xFFFFF000;
	GPGCON |= 0xAAA;
	//因为是下降沿触发，所以GPG端口应该都是高点平。
	GPGDAT |=0x3F ;	
	
//设置EINT[8]-EINT[13]为边沿触发
  rEXTINT1 &=~(7<<0|7<<4|7<<8|7<<12|7<<16|7<<20);
	rEXTINT1 |= (3<<0 | 3<<4 | 3<<8 | 3<<12 | 3<<16 | 3<<20);
//清EINTPEND对应的标志位(EINT8-13)为0
	rEINTPEND&=~((1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12)|(1<<13));
//设置EINT[8]-EINT[13]的屏蔽标志位，使能对应的外中断
	rEINTMASK&=~((1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12)|(1<<13));
//把编写的按键中断服务程序的入口地址写入到 IRQ中断向量表中对应的外中断（EINT8_23）的存储单元中。
	//将函数Key_ISR存入pISR_EINT8_23（中断向量）指向的地址中
	pISR_EINT8_23=(U32)Key_ISR;
//使能按键中断对应的一级屏蔽标志位（EINT8_23）。
	EnableIrq(BIT_EINT8_23);
}

//这里有两组io端口GPB5-8,
void LED_init(void)
{
	//正确配置LED对应的端口
	
	//初始化LED端口
	//把对应操作位置（GPB5678)为设置为0 ，1111 1111 1111 1100 0000 0011 1111 1111
	GPBCON &= 0xFFFC03FF;
	//设置（GPB5678)为输出模式
	GPBCON |= 0x15400;
	
	//GPBDAT都设置为高电平，没有灯亮，0001 1110 0000=0x1E0
	GPBDAT|= 0x1E0;

}

void button1press()			//点亮LED1
{

	//点亮LED1-->GPB5:1111 1111 1111 1111 1111 1111 1101 1111=0xFFFFFFDF
	GPBDAT &= 0xFFFFFFDF;
}

void button2press()			//点亮LED2
{
  //点亮LED2-->GPB6:... 1011 1111=0xFFFFFFBF
	GPBDAT &= 0xFFFFFFBF;
}

void button3press()			//点亮LED3
{
  //点亮LED3-->GPB7:... 0111 1111=0xFFFFFF7F
	GPBDAT &=0xFFFFFF7F;
}

void button4press()			//点亮LED4
{
	 //点亮LED4-->GPB8:... 1110 1111 1111=0xFFFFFEFF
	GPBDAT &= 0xFFFFFEFF;
}

void button5press()			//点亮所有LED
{
  //二进制为：0001 1110 0000=0x1E0
	GPBDAT &= ~0x1E0;
}

void button6press()			//关闭所有LED
{
	//二进制为：0001 1110 0000=0x1E0
	GPBDAT |=0x1E0;	
}


static void __irq Key_ISR(void)      //定义中断服务程序
{
//首先判断，中断是否是由对应的EINT8_23所引起的？如果是，才做进一步处理
	//通过INTPND来判断中断源
	if(rINTPND==BIT_EINT8_23){
		//这里通过查询中断挂起寄存器，判断是否是EINT8_23所引起的
		//是的话就依次清除SRCPND（源挂起寄存器）和INTPND（中断挂起寄存器）
		ClearPending(BIT_EINT8_23);
	}
//如果中断中是由对应的EINT8_23所引起的，然后再进一步判断，中断是由EINT8_23中的哪一个外中断所引起的，并根据判断的结果，进一步调用对应的按键处理程序，进行按键响应。
//注：在调用按键响应前，应当先相关标志位清0。具体步骤是：把EINTPEND中对应的标志位清0，在把SRCPND、INTPND对应标志为清0.
//实验证明，如果不先清零EINTPEND，而是先清零SRCPND、INTPND对应标志为。EINTPEND对应的标志位会再次引起SRCPND、INTPND对应标志位置1，从而导致第二次中断，第二次中断后，由于第一次中断中清理了EINTPEND的标志位，所以不会引起第3次中断。
		if(rEINTPEND&(1<<8)) {
			//判断是否是EINT8发生的中断
			rEINTPEND |= 1<<8; //清除EINT8的中断
			//点亮LED1
			button6press();
			button1press();
		}
		else if(rEINTPEND&(1<<9)){
			//这里是EINT9中断
			rEINTPEND |= 1<<9; //清除EINT9的中断
			//点亮LED2
			button6press();
			button2press();
		}
		else if(rEINTPEND&(1<<10)){
			//这里是EINT10中断
			rEINTPEND |= 1<<10; //清除EINT10的中断
			//点亮LED3
			button6press();
			button3press();
		}
		else if(rEINTPEND&(1<<11)){
			//这里是EINT11中断
			rEINTPEND |= 1<<11; //清除EINT11的中断
			//点亮LED4
			button6press();
			button4press();
		}
		else if(rEINTPEND&(1<<12)){
			//这里是EINT12中断
			rEINTPEND |= 1<<12; //清除EINT12的中断
			//点亮全部
			button5press();
		}
		else if(rEINTPEND&(1<<13)){
			//这里是EINT13中断
			rEINTPEND |= 1<<13; //清除EINT13的中断
			//熄灭全部
			button6press();
	}
}

int main()  
{
	LED_init();
	key_init();
	while(1)
	{
		delay();  
		delay();  
	}
}
