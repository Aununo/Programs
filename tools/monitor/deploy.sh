#!/bin/bash

# ==============================================================================
# 全自动部署监控脚本 (deploy.sh)
# 功能:
# 1. 停止当前正在运行的 newcommit.py 脚本。
# 2. 清理旧的脚本文件和日志文件。
# 3. 自动创建新的 newcommit.py 文件，并写入下面定义的完整 Python 代码。
# 4. 在后台启动新的 newcommit.py 脚本，并将日志输出到 newcommit.log。
# ==============================================================================

echo "====== [部署脚本开始] ======"

# --- 第1步: 停止可能正在运行的旧脚本 ---
echo "[1/5] 正在停止旧的监控脚本 ()..."
pkill -f newcommit.py
# 等待1秒确保进程已完全终止
sleep 1
echo "旧脚本已停止。"

# --- 第2步: 清理旧文件 ---
echo "[2/5] 正在清理旧的脚本和日志文件 (rm -f)..."
rm -f newcommit.py newcommit.log
echo "清理完成。"

# --- 第3步: 创建新的 Python 脚本文件 ---
# 使用 cat <<'EOF' 语法 (Here Document) 将代码块写入文件。
# 'EOF' 加上单引号可以防止 bash 试图解析代码中的 '$' 等特殊字符。
echo "[3/5] 正在创建新的 newcommit.py 脚本..."
cat << 'EOF' > newcommit.py
########################################################################### << 替换为你的脚本
EOF
# --- Here Document 结束 ---

echo "脚本 newcommit.py 创建成功。"

# --- 第4步: 启动新脚本 ---
# 使用 nohup ... & 在后台运行，并将所有输出重定向到 newcommit.log
# python3 -u 参数确保日志是实时写入的，方便 tail -f 查看
echo "[4/5] 正在后台启动新的监控脚本..."
nohup python3 -u newcommit.py > newcommit.log 2>&1 &
echo "脚本已启动。"

# --- 第5步: 提示用户如何查看状态和日志 ---
echo "[5/5] 部署完成！"
echo "你可以使用以下命令检查脚本是否正在运行:"
echo "ps aux | grep newcommit.py | grep -v grep"
echo "你可以使用以下命令实时监控脚本日志:"
echo "tail -f newcommit.log"
echo "====== [部署脚本结束] ======"