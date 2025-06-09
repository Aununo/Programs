	AREA MedianFilter, CODE, READONLY
	ENTRY

Main
    MOV     R0, #6                  ; 设置采样点数N=6
    LDR     R7, =0x40000000         ; 数据存储基地址 (改用R7)
    
    ; 数据初始化（使用连续寄存器 R1-R6）
    MOV     R1, #0x0A               ; 示例数据
    MOV     R2, #0x08
    MOV     R3, #0x02
    MOV     R4, #0x01
    MOV     R5, #0x06
    MOV     R6, #0x04               ; 将第6个数据存入R6
	
    STMIA   R7!, {R1-R6}            ; 一次性存入R1到R6中的6个数据
    SUB     R7, R7, #24             ; 恢复基地址指针 (6个数据 * 4字节 = 24)

    ; 第一阶段：冒泡排序算法
    SUB     R8, R0, #1              ; 外层循环次数N-1
    MOV     R9, #0                  ; 外层循环计数器i
OuterLoop
    CMP     R9, R8                  ; 比较i与N-1
    BGE     EndOuterLoop            ; 完成排序则跳出
    
    MOV     R10, #0                 ; 内层循环计数器j
    SUB     R11, R8, R9             ; 计算内层循环上限N-1-i
InnerLoop
    CMP     R10, R11                ; 比较j与N-1-i
    BGE     EndInnerLoop
    
    LDR     R4, [R7, R10, LSL #2]   ; 加载data[j] (使用R7作为基地址)
    ADD     R12, R10, #1
    LDR     R5, [R7, R12, LSL #2]   ; 加载data[j+1] (使用R7作为基地址)
    
    CMP     R4, R5                  ; 比较相邻元素
    BLE     NoSwap                  ; 已有序则不交换
    
    STR     R5, [R7, R10, LSL #2]   ; 执行交换操作 (使用R7作为基地址)
    STR     R4, [R7, R12, LSL #2]
NoSwap
    ADD     R10, R10, #1            ; j++
    B       InnerLoop
EndInnerLoop
    ADD     R9, R9, #1              ; i++
    B       OuterLoop
EndOuterLoop

    ; 第二阶段：计算中间值之和
    MOV     R1, #1                  ; 起始索引（跳过最小值）
    SUB     R2, R0, #2              ; 有效数据个数N-2
    MOV     R3, #0                  ; 初始化累加器
SumLoop
    LDR     R4, [R7, R1, LSL #2]    ; 加载当前数据 (使用R7作为基地址)
    ADD     R3, R3, R4              ; 累加到总和
    ADD     R1, R1, #1              ; 指针递增
    SUBS    R2, R2, #1              ; 计数器递减
    BNE     SumLoop                 ; 继续循环直到处理完所有数据
    
    ; 第三阶段：除法实现
    SUB     R0, R0, #2              ; 计算除数N-2=4
    MOV     R4, #0                  ; 初始化移位计数器
FindShift
    MOVS    R5, R0, LSR #1          ; R5 = R0 >> 1
    BEQ     ShiftDone               ; 当商为0时结束
    ADD     R4, R4, #1              ; 移位计数器+1
    MOV     R0, R5                  ; 更新除数值
    B       FindShift
ShiftDone 
    ; 此时R4包含需要的右移位数（log2(N-2)）
    ADD     R3, R3, R0, LSR #1      ; 四舍五入预处理
    MOV     R5, R3, LSR R4          ; 执行右移代替除法

    B       .                       ; 程序终止
    END