#!/bin/bash

# 编译代码
clang++ -std=c++17 -o check ../45-Jump-Game-II.cpp

# 检查是否编译成功
if [ $? -ne 0 ]; then
    echo "Compilation Error!"
    exit 1
fi

# 运行程序，生成 output.txt
./check

# 检查是否运行成功
if [ $? -ne 0 ]; then
    echo "Runtime Error!"
    exit 1
fi

# 比较输出和预期结果
if diff -u expected_output.txt output.txt > /dev/null; then
    echo "Accepted."
else
    echo "Wrong Answer!"
fi
