AREA    median_filter, CODE, READONLY 
        ENTRY                                   

BASE_ADDRESS        EQU     0x40000000   ; 定义数据存储的 RAM 起始地址
N_VALUE             EQU     9            ; 定义 N 的值 (必须为奇数，这里是 9)

start

        ; --- 数据拷贝阶段 ---
        ; 将初始数据从代码段的数据区域拷贝到 RAM (0x40000000)

        LDR     R0, =N_VALUE         ; R0 = N (要拷贝的字数)
        LDR     R1, =DataSource      ; R1 为数据源的起始地址 (代码段中的 DataSource 标签)
        LDR     R2, =BASE_ADDRESS    ; R2 为数据目的地的起始地址 (RAM 0x40000000)

copy_loop
        SUBS    R0, R0, #1           ; 拷贝计数减一，并更新标志位
        BLT     copy_done            ; 如果计数 < 0，跳出循环

        LDR     R3, [R1], #4         ; 从源地址 R1 加载一个字到 R3，R1 自动加 4
        STR     R3, [R2], #4         ; 将 R3 中的字存储到目的地址 R2，R2 自动加 4
        B       copy_loop            ; 继续循环

copy_done
        ; --- 数据拷贝结束 ---


        ; --- 中值滤波核心逻辑 (在 RAM 区域 0x40000000 上执行) ---

        ; R0 和 R2 的值在拷贝完成后被修改，需要重新加载它们用于滤波逻辑
        LDR     R0, =N_VALUE         ; R0 = N, 排序数的个数
        LDR     R2, =BASE_ADDRESS    ; R2 为原始数据的起始地址 (现在数据已拷贝到此)

        ; --- 初始化循环边界/指针 ---
        ; 根据 N 和基地址计算内、外循环的结束地址
        ; R3 为内循环指针的结束地址 (Base + (N-1)*4)
        ADD     R3, R2, R0, LSL #2   ; R3 = Base + N*4 (数组末尾地址的下一个地址)
        SUB     R3, R3, #4           ; R3 = Base + (N-1)*4 (数组最后一个元素的地址)

        ; R4 为外循环指针的结束地址 (Base + (N-2)*4)
        ; 外循环指针 R2 遍历到倒数第二个元素的位置即可完成排序
        SUB     R4, R3, #4           ; R4 = (Base + (N-1)*4) - 4 = Base + (N-2)*4

        ; R2 将在外循环中作为当前位置的指针，从 Base 开始递增

        LDR     R2, =BASE_ADDRESS    ; 重新将 R2 初始化为排序循环的起始地址

; --- 排序部分 (Selection Sort 变体) ---
; 外循环 (LOOP1) 控制 R2 从 Base 遍历到 R4 (Base + (N-2)*4)
; 内循环 (LOOP2) 控制 R5 从 R2+4 遍历到 R3 (Base + (N-1)*4)
; 比较 [R2] 和 [R5]，若 [R2] > [R5] 则交换

LOOP1
        CMP     R2, R4               ; 比较外循环指针是否超过结束地址 (Base + (N-2)*4)
        BGT     sort_done            ; 如果 R2 > R4，排序完成

        ADD     R5, R2, #4           ; R5 作为内循环指针，初始化为 R2 + 4 (从外循环当前位置的下一个元素开始)

LOOP2
        CMP     R5, R3               ; 比较内循环指针是否超过结束地址 (Base + (N-1)*4)
        BGT     inner_loop_done      ; 如果 R5 > R3，内循环完成

        ; 比较 [R2] 指向的值和 [R5] 指向的值 (外循环当前元素与内循环遍历到的元素比较)
        LDR     R6, [R2]             ; 加载外循环当前位置 (R2) 的元素值到 R6
        LDR     R7, [R5]             ; 加载内循环当前位置 (R5) 的元素值到 R7

        CMP     R6, R7               ; 比较 R6 和 R7
        BLS     no_swap              ; 如果 R6 <= R7（无符号比较），则不进行交换

        ; 交换 [R2] 和 [R5] 指向的内存中的值
        STR     R7, [R2]             ; 将 R7 (原 [R5] 的值) 存到 R2 指向的内存位置
        STR     R6, [R5]             ; 将 R6 (原 [R2] 的值) 存到 R5 指向的内存位置

no_swap
        ADD     R5, R5, #4           ; 内循环指针后移一个字 (4字节)
        B       LOOP2                ; 继续内循环

inner_loop_done
        ADD     R2, R2, #4           ; 外循环指针后移一个字 (4字节)，移动到下一个位置进行外循环比较
        B       LOOP1                ; 继续外循环

sort_done
        ; --- 排序完成 ---
        ; 查找中值元素
        ; N 的值在 R0 中
        ; 中值元素的索引 = N / 2 (整数除法)
        ; 对应汇编 MOV R2, R0, LSR #1

        ; N_VALUE 已在 R0 (如果拷贝后 R0 未被修改)
        ; 如果 R0 在拷贝循环后被修改，这里需要重新加载 N_VALUE 到 R0
        ; LDR     R0, =N_VALUE ; 确保 R0 存有 N 的值

        MOV     R0, R0, LSR #1       ; R0 现在存储中值索引 (N/2) (例如 9/2 = 4)

        ; 计算中值元素的地址 = 基地址 + 中值索引 * 4
        LDR     R2, =BASE_ADDRESS    ; 重新加载基地址到 R2 (排序循环结束后 R2 已不再是基地址)
        ADD     R3, R2, R0, LSL #2   ; R3 = 基地址 + 中值索引 * 4 (R0 * 4)

        ; 将中值元素的值加载到 R1
        LDR     R1, [R3]             ; 将 R3 指向的中值元素加载到 R1

        B       stop                 ; 程序结束

stop    B       stop                 ; 无限循环，用于模拟程序停止

        AREA    DataSourceArea, DATA, READONLY ; 定义一个数据段，用于存放原始数据
DataSource
        DCD     10, 5, 20, 15, 25, 8, 12, 30, 3 ; 初始数据值 (与 N_VALUE 数量匹配)

        END
