"""
这是一个同步监控题目列表API的脚本。
它使用 requests 库定期请求API，当列表中任何一个题目的 'firstSolverName' 字段
从 null 变为有值（即产生一血）时，程序会发送邮件通知管理员。
"""
import requests
import time
import smtplib
from email.mime.text import MIMEText
from email.header import Header

# --- 1. 配置区域 ---
API_URL = ""
CHECK_INTERVAL = 15
HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36',
    'Cookie': 'your_cookie_here'
}
SMTP_SERVER = "smtp.qq.com"
SMTP_PORT = 465
SENDER_EMAIL = "aununo@qq.com"
SENDER_PASSWORD = "ixdgrgldjlsldcee"
RECEIVER_EMAIL = "aununo@qq.com"

# --- 2. 状态和邮件发送函数 ---
problems_fb_state = {}

def send_email_notification(subject, body):
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

# --- 3. 核心监控逻辑 ---
def main():
    is_initialized = False
    print("一血监控程序已启动 (同步模式)...")
    
    while True:
        print(f"\n--- 开始新一轮检查 ({time.strftime('%Y-%m-%d %H:%M:%S')}) ---")
        try:
            response = requests.get(API_URL, headers=HEADERS, timeout=20)
            
            if response.status_code == 200:
                data = response.json()
                problems_list = data.get('data', [])

                for problem in problems_list:
                    problem_id = problem.get('id')
                    problem_title = problem.get('title')
                    first_solver_name = problem.get('firstSolverName')

                    if not problem_id or not problem_title:
                        continue

                    if not is_initialized:
                        problems_fb_state[problem_id] = (first_solver_name is not None)
                    
                    elif problems_fb_state.get(problem_id) is False and first_solver_name is not None:
                        print(f"\n!!! 检测到题目 '{problem_title}' 产生一血 !!!")
                        subject = f"【一血通知】题目'{problem_title}'已被攻破！"
                        body = f"题目名称: {problem_title}\n首个解题者: {first_solver_name}\n解题时间: {problem.get('firstSolveTime', 'N/A')}"
                        send_email_notification(subject, body)
                        problems_fb_state[problem_id] = True
                
                if not is_initialized:
                    print("初始化扫描完成。")
                    is_initialized = True
            else:
                print(f"请求API失败, 状态码: {response.status_code}")

        except Exception as e:
            print(f"检查时发生严重错误: {e}")
        
        print(f"--- 本轮检查完毕，等待 {CHECK_INTERVAL} 秒 ---")
        time.sleep(CHECK_INTERVAL)

# --- 4. 运行程序 ---
if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n程序被用户手动中断。")