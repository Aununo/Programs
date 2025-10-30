import requests
import time
import smtplib
from email.mime.text import MIMEText
from email.header import Header

API_URL = ""
CHECK_INTERVAL = 1200  
HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36',
    'Cookie': '' # <<< 请务必替换成您管理员账号的Cookie
}
# SMTP邮箱配置
SMTP_SERVER = "smtp.qq.com"
SMTP_PORT = 465
SENDER_EMAIL = ""      # <<< 发件人邮箱
SENDER_PASSWORD = ""  # <<< SMTP授权码
RECEIVER_EMAIL = ""      # <<< 收件人邮箱


problems_commit_state = {}

def send_email_notification(subject, body):
    """邮件发送函数"""
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


def main():
    is_initialized = False
    print("新提交监控程序已启动...")
    
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
                    current_commit_count = problem.get('newCommitCount', 0)

                    if not problem_id or not problem_title:
                        continue

                    last_commit_count = problems_commit_state.get(problem_id, 0)

                    if current_commit_count > last_commit_count:
                        if not is_initialized:
                            pass 
                        else:
                            print(f"\n!!! 检测到题目 '{problem_title}' 有新的提交！!!!")
                            print(f"    数量从 {last_commit_count} 变为 {current_commit_count}")
                            
                            subject = f"【招新提交提醒】题目'{problem_title}'有新的提交！"
                            body = f"""
深深宝贝你好呀！

你管理的题目有新的未处理提交哦：

题目名称: {problem_title}
待批改数量: 从 {last_commit_count} 份增加到 {current_commit_count} 份

请及时登录后台进行批改~

——爱你的诺诺 ovo
"""
                            send_email_notification(subject, body)

                    problems_commit_state[problem_id] = current_commit_count

                if not is_initialized:
                    print("初始化扫描完成。已记录所有题目的当前提交状态。")
                    is_initialized = True
            else:
                print(f"请求API失败, 状态码: {response.status_code}。请检查Cookie是否有效或过期。")

        except Exception as e:
            print(f"检查时发生严重错误: {e}")
        
        print(f"--- 本轮检查完毕，等待 {CHECK_INTERVAL} 秒 ---")
        time.sleep(CHECK_INTERVAL)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n程序被用户手动中断。")


#  nohup python3 -u newcommit.py > newcommit.log 2>&1 &
#  tail -f newcommit.log
#  pkill -f newcommit.py
#  ps aux | grep newcommit.py | grep -v grep