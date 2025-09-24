"""
这是一个可以异步监控多个题目的脚本程序。
它会请求 题目详情API 当任何一个被监控题目的解题总人数('solverNum')增加时，
程序会发送邮件通知管理员，并持续进行监控。
"""

import asyncio
import aiohttp
import time
import smtplib
from email.mime.text import MIMEText
from email.header import Header

# --- 1. 配置区域 ---

# 题目监控配置: 将需要监控的题目详情API链接放入列表中
API_URLS = [
    # 示例URL，请替换为您实际的API
]
CHECK_INTERVAL = 1800  # 检查间隔（秒）
HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36',
    'Cookie': ''  # <<< 请务必替换成您自己的Cookie
}

# SMTP邮箱配置
SMTP_SERVER = "smtp.qq.com"
SMTP_PORT = 465
SENDER_EMAIL = ""  # <<< 发件人邮箱
SENDER_PASSWORD = ""  # <<< SMTP授权码
RECEIVER_EMAIL = ""  # <<< 收件人邮箱


# --- 2. 邮件发送函数 ---

def _send_email_sync(subject, body):
    """邮件发送的同步实现。"""
    try:
        message = MIMEText(body, 'plain', 'utf-8')
        message['From'] = SENDER_EMAIL
        message['To'] = Header(f"管理员 <{RECEIVER_EMAIL}>", 'utf-8')
        message['Subject'] = Header(subject, 'utf-8')

        print("正在连接到邮件服务器...")
        server = smtplib.SMTP_SSL(SMTP_SERVER, SMTP_PORT)
        print("正在登录邮箱...")
        server.login(SENDER_EMAIL, SENDER_PASSWORD)
        print("正在发送邮件...")
        server.sendmail(SENDER_EMAIL, [RECEIVER_EMAIL], message.as_string())
        server.quit()
        print(f"邮件发送成功！主题: {subject}")
        return True
    except Exception as e:
        print(f"邮件发送失败: {e}")
        return False

async def send_email_async(subject, body):
    """邮件发送的异步封装。"""
    loop = asyncio.get_running_loop()
    await loop.run_in_executor(None, _send_email_sync, subject, body)

# --- 3. 核心异步逻辑 ---

async def check_problem(session, url, state):
    """检查单个题目的状态。"""
    try:
        async with session.get(url, timeout=15) as response:
            if response.status == 200:
                data = await response.json()
                problem_data = data.get('data', {})
                
                if not problem_data:
                    print(f"错误: API {url} 的响应中未找到'data'字段。")
                    return
                
                # **核心改动**: 从 'solverNum' 获取解题人数
                current_solver_num = problem_data.get('solverNum', 0)
                
                # 首次运行时，初始化状态
                if state['last_solver_num'] == -1:
                    task_name = problem_data.get('title', '未知题目')
                    state['task_name'] = task_name
                    state['last_solver_num'] = current_solver_num
                    print(f"初始化监控: '{task_name}', 当前解题人数: {current_solver_num}")
                    return

                # **核心逻辑**: 检查当前人数是否大于上次记录的人数
                if current_solver_num > state['last_solver_num']:
                    task_name = state.get('task_name', '未知题目')
                    old_count = state['last_solver_num']
                    print(f"\n!!! 检测到题目 '{task_name}' 有新提交通过！人数从 {old_count} 变为 {current_solver_num} !!!")
                    
                    # **核心改动**: 更新邮件内容以匹配新的API数据
                    subject = f"【解题动态】题目'{task_name}'有新的解答！"
                    body = f"""
您好！

您监控的题目有新的用户通过，详情如下：

题目名称: {task_name}
解题人数: 从 {old_count}人 增加到 {current_solver_num}人

请及时关注！
"""
                    await send_email_async(subject, body)
                
                # 无论是否发送邮件，都更新最后一次记录的人数
                state['last_solver_num'] = current_solver_num

            else:
                print(f"请求失败 {url}, 状态码: {response.status}")
    except Exception as e:
        print(f"检查题目 {url} 时发生错误: {e}")

async def main():
    """程序主入口。"""
    problems_state = {url: {'last_solver_num': -1, 'task_name': '未知'} for url in API_URLS}
    
    print("解题动态监控程序已启动，正在监控 " + str(len(API_URLS)) + " 个题目...")
    
    async with aiohttp.ClientSession(headers=HEADERS) as session:
        while True:
            tasks_to_run = [
                check_problem(session, url, state) 
                for url, state in problems_state.items()
            ]
            
            print(f"\n--- 开始新一轮检查 ({time.strftime('%Y-%m-%d %H:%M:%S')}) ---")
            await asyncio.gather(*tasks_to_run)
            print(f"--- 本轮检查完毕，等待 {CHECK_INTERVAL} 秒 ---")
            
            await asyncio.sleep(CHECK_INTERVAL)

# --- 4. 运行程序 ---

if __name__ == "__main__":
    # 重要：如果您的系统是Windows，可能需要添加下面这行来避免一个常见的asyncio错误
    # asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n程序被用户手动中断。")