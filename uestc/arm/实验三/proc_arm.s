	PRESERVE8 ; 确保堆栈是8字节对齐的
	MACRO
$MDATA STRCOPY $DEST_REG, $SRC_REG 
$MDATA.LOOP   
    LDRB    R2, [$SRC_REG], #1      ; 从源寄存器地址读取字节并自增
    STRB    R2, [$DEST_REG], #1     ; 写入目标寄存器地址并自增
    CMP     R2, #0                  ; 比较R2中的字节是否为零。字符串通常以0作为结束符。
    BNE     $MDATA.LOOP          ; 如果非零则继续循环
    MEND




    AREA    EXAMPLE, CODE, READONLY
	IMPORT Proc_c
		
val EQU 0x40000038
	
    ENTRY  
	LDR 	SP,=0X40000100
    LDR     R1, =SOURCE     ; 将字符串SOURCE的起始地址加载到寄存器R1
    LDR     R0, =0x40000000    ; 将目标内存地址0x40000000加载到寄存器R0
COPY1  STRCOPY R0, R1          
    LDR     R0, =0x40000100 
COPY2     STRCOPY R0, R1          ; 通过寄存器传递地址参数
	MOV R0,#1
	BL Proc_ARM
	B .





Proc_ARM
	STR LR,[SP,#-4]!     ; 将连接寄存器LR的值压入栈中
	ADD R1, R0,R0
	ADD R2, R0,R1
	ADD R3, R0,R2
	ADD R4, R2,R1
	ADD R5, R2,R2
	STR R5,[SP,#-4]!   ; 将R5（对应i6）压入栈中。根据ATPCS，前四个参数(i1-i4)通过R0-R3传递
	STR R4,[SP,#-4]!	 ; 超过四个的参数(i5, i6)需要通过栈传递，且按从右到左的顺序压栈
	BL Proc_c
	ADD SP, SP,#8
	RSB R0, R0, #0
	LDR R1, =val
	STR R0,[R1]
	LDR PC, [SP],#4


	

SOURCE  DCB     "Hello, World!", 0  ; 源字符串
DEST    SPACE   20                  ; 目标空间

    END
		