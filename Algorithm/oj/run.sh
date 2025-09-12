#!/bin/bash

# 编译代码
g++ -x c++ user.code -o user_program 2> gcc.log

# 检查编译状态并执行相应操作
if [ $? -eq 0 ]; then
    # 编译成功，运行程序
    echo "编译成功，运行程序"
    ./user_program
else
    # 编译失败，显示错误日志
    echo "编译错误："
    cat gcc.log
    exit 1
fi