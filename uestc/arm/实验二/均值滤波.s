;==============================================================================
; 文件名: mean_filter_styled.s
; 功能: 实现对内存0x40000000开始的N个无符号数的改进均值滤波
;       模仿中值滤波示例的结构和风格。
;       - 将代码段数据复制到0x40000000
;       - 移除最大值和最小值
;       - 计算剩余N-2个数的平均值
;       - 结果存入R1
;       - N (偶数) 在N_VALUE中设置，且N-2必须是2的幂次方
;==============================================================================

        AREA    mean_filter, CODE, READONLY    ; 定义代码段，可读
        ENTRY                                  ; 声明程序入口点

BASE_ADDRESS    EQU     0x40000000         ; 定义数据存储的 RAM 起始地址 (S3C2440 片内 RAM)
N_VALUE         EQU     10                 ; 定义 N 的值 (必须为偶数，且 N-2 是 2 的幂次方)
                                           ; 例如: 4(N-2=2), 6(N-2=4), 10(N-2=8), 18(N-2=16)

start

        ; --- 数据拷贝阶段 ---
        ; 将初始数据从代码段的数据区域拷贝到 RAM (BASE_ADDRESS)
        ; R0: 拷贝计数 (N)
        ; R1: 数据源地址
        ; R2: 数据目的地址
        ; R3: 临时存放加载的数据

        LDR     R0, =N_VALUE               ; R0 = N (要拷贝的字数)
        LDR     R1, =DataSource            ; R1 为数据源的起始地址 (代码段中的 DataSource 标签)
        LDR     R2, =BASE_ADDRESS          ; R2 为数据目的地的起始地址 (RAM 0x40000000)

copy_loop
        SUBS    R0, R0, #1                 ; 拷贝计数减一，并更新标志位
        BLT     copy_done                  ; 如果计数 < 0，跳出循环

        LDR     R3, [R1], #4               ; 从源地址 R1 加载一个字到 R3，R1 自动加 4
        STR     R3, [R2], #4               ; 将 R3 中的字存储到目的地址 R2，R2 自动加 4
        B       copy_loop                  ; 继续循环

copy_done
        ; --- 数据拷贝结束 ---


        ; --- 改进均值滤波核心逻辑 (在 RAM 区域 BASE_ADDRESS 上执行) ---
        ; 计算 N 个数 (去掉最大最小值) 的平均值，结果存入 R1
        ; R0: N (重新加载)
        ; R2: 数据指针
        ; R3: 滤波循环计数 (N-1)
        ; R4: 最小值 (Min)
        ; R5: 最大值 (Max)
        ; R6: 总和 / 过滤后的总和
        ; R7: 临时存放当前加载的数据
        ; R8: N-2
        ; R9: 右移位数 k (log2(N-2))
        ; R10: 查找 2^k 的辅助变量

        LDR     R0, =N_VALUE               ; R0 = N (重新加载 N 的值)
        LDR     R2, =BASE_ADDRESS          ; R2 = 数据起始地址 (RAM)

        ; 初始化 Min, Max, Sum with the first element
        ; 假设 N >= 1，因此第一个元素有效
        LDR     R4, [R2]                   ; R4 = Min = 第一个元素
        MOV     R5, R4                     ; R5 = Max = 第一个元素
        MOV     R6, R4                     ; R6 = Sum = 第一个元素 (累计所有 N 个数的和)

        ; 循环处理剩余的 N-1 个元素
        SUBS    R3, R0, #1                 ; R3 = 滤波循环计数 (N-1)
        ADD     R2, R2, #4                 ; R2 指针前移 4 字节，指向第二个元素

filter_loop
        LDR     R7, [R2]                   ; R7 = 加载当前元素

        ; 累加当前元素到 Sum
        ADD     R6, R6, R7                 ; R6 = Sum + 当前元素

        ; 更新最小值 (Min)
        CMP     R7, R4                     ; 比较当前元素 (R7) 与 Min (R4)
        IT      LT                         ; 如果小于...
        MOVLT   R4, R7                     ; ...更新 Min

        ; 更新最大值 (Max)
        CMP     R7, R5                     ; 比较当前元素 (R7) 与 Max (R5)
        IT      GT                         ; 如果大于...
        MOVGT   R5, R7                     ; ...更新 Max

        ADD     R2, R2, #4                 ; 数据指针前移 4 字节
        SUBS    R3, R3, #1                 ; 滤波循环计数减 1
        BNE     filter_loop                ; 如果计数不为零，继续循环

        ; --- 计算过滤后的平均值 ---
        ; Sum (所有 N 个数的和) 在 R6 中
        ; Min (最小值) 在 R4 中
        ; Max (最大值) 在 R5 中
        ; N 在 R0 中

        ; 从总和中减去最小值和最大值
        SUBS    R6, R6, R4                 ; R6 = Sum - Min
        SUBS    R6, R6, R5                 ; R6 = (Sum - Min) - Max  (现在 R6 是 N-2 个数的和)

        ; 计算 N-2 (过滤后的元素个数)
        SUBS    R8, R0, #2                 ; R8 = N-2
                                           ; IMPORTANT: R8 必须是 2 的幂次方 (2, 4, 8, 16, ...)

        ; 计算除以 N-2 所需的右移位数 k (即 log2(N-2))
        MOV     R9, #0                     ; R9 = 初始化右移位数为 0
        MOV     R10, #1                    ; R10 = 初始化查找值为 2^0 = 1

shift_count_loop
        CMP     R10, R8                    ; 比较当前的 2^k 值 (R10) 与 N-2 (R8)
        BEQ     shift_count_done           ; 如果相等，找到了正确的移位数，跳出循环

        LSL     R10, R10, #1               ; R10 左移 1 位 (乘以 2)，计算下一个 2 的幂次方 (2^(k+1))
        ADD     R9, R9, #1                 ; R9 (移位数) 加 1
        B       shift_count_loop           ; 继续循环查找

shift_count_done
        ; 执行右移运算 (除法)
        ; Filtered_Sum (在 R6) / (N-2) (等价于右移 R9 位)
        LSR     R1, R6, R9                 ; R1 = R6 >> R9 (将计算结果存入 R1)

        ; --- 程序结束 ---
stop
        B       stop                       ; 无限循环，用于模拟程序停止


; --- 初始数据定义 ---
; 定义一个数据段，用于存放原始数据
; 数据个数必须至少等于你在 N_VALUE 中设置的值
        AREA    DataSourceArea, DATA, READONLY ; 定义一个数据段，可读
DataSource
        DCD     10, 25, 5, 15, 30, 12, 8, 20, 18, 22 ; 10 个数据 (用于 N=10)
        DCD     1, 2, 3, 4, 5, 6, 7, 9, 11, 13      ; 添加更多数据，如果 N_VALUE 可能更大

        END                                        ; 文件结束