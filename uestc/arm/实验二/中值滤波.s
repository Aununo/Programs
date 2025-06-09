	AREA    MedianFilter, CODE, READONLY
    ENTRY

Main
    ; --- 初始化 ---
    MOV     R0, #9                  ; 1. 设置窗口长度 L = 9
    LDR     R6, =0x40000000         ; 2. 设置数据存储基地址

    ; 第一批：存入前5个数据
    MOV     R1, #0x13               ; 示例数据: 0x13, 0x23, 0x75, 0x34, 0x62
    MOV     R2, #0x23
    MOV     R3, #0x75
    MOV     R4, #0x34
    MOV     R5, #0x62
    STMIA   R6!, {R1-R5}            ; 存入5个数据, R6地址自动增加 5*4=20 字节

    ; 第二批：存入后4个数据
    MOV     R1, #0x43               ; 示例数据: 0x43, 0x91, 0x79, 0x31
    MOV     R2, #0x91
    MOV     R3, #0x79
    MOV     R4, #0x31
    STMIA   R6!, {R1-R4}            ; 存入4个数据, R6地址再次增加 4*4=16 字节

    ; 4. 恢复基地址指针
    SUB     R6, R6, #36             ; R6 总共增加了 20+16=36 字节, 在此减去36使其指回起始地址

    ; --- 第一阶段：冒泡排序算法 (与您提供的代码结构完全相同) ---
    SUB     R8, R0, #1              ; 外层循环次数L-1
    MOV     R9, #0                  ; 外层循环计数器i
OuterLoop
    CMP     R9, R8                  ; 比较i与L-1
    BGE     EndOuterLoop            ; 完成排序则跳出

    MOV     R10, #0                 ; 内层循环计数器j
    SUB     R11, R8, R9             ; 计算内层循环上限L-1-i
InnerLoop
    CMP     R10, R11                ; 比较j与L-1-i
    BGE     EndInnerLoop

    LDR     R4, [R6, R10, LSL #2]   ; 加载data[j]
    ADD     R12, R10, #1
    LDR     R5, [R6, R12, LSL #2]   ; 加载data[j+1]

    CMP     R4, R5                  ; 比较相邻元素
    BLE     NoSwap                  ; 已有序则不交换

    STR     R5, [R6, R10, LSL #2]   ; 执行交换操作
    STR     R4, [R6, R12, LSL #2]
NoSwap
    ADD     R10, R10, #1            ; j++
    B       InnerLoop
EndInnerLoop
    ADD     R9, R9, #1              ; i++
    B       OuterLoop
EndOuterLoop

    ; --- 第二阶段：获取中值 ---
    SUB     R1, R0, #1              ; R1 = L-1
    MOV     R1, R1, LSR #1          ; 计算中值索引 = (L-1)/2
    LDR     R5, [R6, R1, LSL #2]    ; 加载中值到R5 (对于L=9, 此处会加载索引4的值)

    B       .                       ; 程序终止（结果在R5）
    END