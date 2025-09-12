"""
这是一个招新期间的题目一血脚本监控程序。
当监控的题目产生一血记录时，程序会发送邮件通知管理员。
请确保在使用前正确配置API URL、Cookie和邮箱信息。
"""


import requests
import time
import json
import smtplib
from email.mime.text import MIMEText
from email.header import Header

# --- 1. 配置区域 ---

# 题目监控配置
API_URL = "your_api_url" # API URL
CHECK_INTERVAL = 6  # 检查间隔（秒）
HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36',
    'Cookie': 'your_cookie' # <<< 请务必替换成您自己的Cookie
}

SMTP_SERVER = "smtp.qq.com"
SMTP_PORT = 465
SENDER_EMAIL = "xxx@qq.com"
SENDER_PASSWORD = "your_smtp_authorization_code"  # SMTP授权码
RECEIVER_EMAIL = "xxx@qq.com"

# --- 2. 邮件发送函数 ---

def send_email_notification(subject, body):
    try:
        # 创建一个带附件的实例
        message = MIMEText(body, 'plain', 'utf-8')
        message['From'] = SENDER_EMAIL
        message['To'] = Header(f"管理员 <{RECEIVER_EMAIL}>", 'utf-8')
        message['Subject'] = Header(subject, 'utf-8')

        # 连接到SMTP服务器
        print("正在连接到邮件服务器...")
        server = smtplib.SMTP_SSL(SMTP_SERVER, SMTP_PORT)
        
        # 登录
        print("正在登录邮箱...")
        server.login(SENDER_EMAIL, SENDER_PASSWORD)
        
        # 发送邮件
        print("正在发送邮件...")
        server.sendmail(SENDER_EMAIL, [RECEIVER_EMAIL], message.as_string())
        
        # 关闭连接
        server.quit()
        print("邮件发送成功！")
        return True
    
    except Exception as e:
        print(f"邮件发送失败: {e}")
        return False

# --- 3. 程序主体 ---

last_solver_num = -1 
first_blood_notified = False  # 一血邮件发送状态标志

print("一血监控程序已启动...")

while True:
    # 如果已经发送过一血通知，就没必要再循环了
    if first_blood_notified:
        print("一血已被捕获并已发送通知，程序退出。")
        break
        
    try:
        response = requests.get(API_URL, headers=HEADERS, timeout=15)
        
        if response.status_code == 200:
            data = response.json()
            problem_data = data.get('data', {})
            
            if not problem_data:
                print("错误：JSON响应中未找到'data'字段。")
                time.sleep(CHECK_INTERVAL)
                continue

            current_solver_num = problem_data.get('solverNum')
            task_name = problem_data.get('title')

            if last_solver_num == -1:
                last_solver_num = current_solver_num
                print(f"初始化成功！题目: '{task_name}', 当前解题人数: {last_solver_num}")

            # 核心逻辑：检查是否产生一血
            if last_solver_num == 0 and current_solver_num > 0 and not first_blood_notified:
                print(f"\n!!! 检测到一血产生 !!!")
                
                passing_time = problem_data.get('firstSolveTime')
                
                # 构造邮件内容
                subject = f"【一血通知】题目'{task_name}'已被攻破！"
                body = f"""
您好！

您监控的题目产生了一血记录，详情如下：

题目名称: {task_name}

解题时间: {passing_time}

请及时关注！
"""
                # 发送邮件并更新状态
                if send_email_notification(subject, body):
                    first_blood_notified = True

            # 更新上一次的人数记录
            last_solver_num = current_solver_num
        else:
            print(f"请求失败，状态码: {response.status_code}")

    except Exception as e:
        print(f"发生错误: {e}")

    time.sleep(CHECK_INTERVAL)