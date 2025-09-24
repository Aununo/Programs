"""
这是一个监控题目管理列表API的脚本。
它会定期请求API，当列表中任何一个题目的 'newCommitCount' (新提交数量) 增加时，
程序会发送邮件通知管理员前去批改作业。
脚本会正确处理管理员批改后 newCommitCount 归零的情况，不会重复或错误通知。
"""
import requests
import time
import smtplib
from email.mime.text import MIMEText
from email.header import Header

# --- 1. 配置区域 ---
API_URL = ""
CHECK_INTERVAL = 1200  # 检查间隔（秒），可以根据需要调整
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

# --- 2. 状态与邮件函数 ---

# 用于存储所有题目的新提交数量状态, 格式: { "题目ID": 上一次的数量 }
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

# --- 3. 核心监控逻辑 ---
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

                # 遍历API返回的每一个题目
                for problem in problems_list:
                    problem_id = problem.get('id')
                    problem_title = problem.get('title')
                    current_commit_count = problem.get('newCommitCount', 0)

                    if not problem_id or not problem_title:
                        continue

                    # 获取上次记录的提交数，如果题目是第一次见到，则默认为0
                    last_commit_count = problems_commit_state.get(problem_id, 0)
                    
                    # 核心逻辑：只在当前提交数 > 上次记录的提交数时，才触发通知
                    if current_commit_count > last_commit_count:
                        # 对于首次运行，我们只记录状态，不发送邮件，防止启动时对现有提交发送垃圾邮件
                        if not is_initialized:
                            pass # 首次运行时跳过通知
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

                    # 无论是否发送邮件，都必须更新状态为当前最新的数量
                    # 这样当您批改后数量减少到0，状态也会更新为0，为下一次增加做准备
                    problems_commit_state[problem_id] = current_commit_count
                
                # 首次运行结束后，打印初始化信息并更改标志
                if not is_initialized:
                    print("初始化扫描完成。已记录所有题目的当前提交状态。")
                    is_initialized = True
            else:
                print(f"请求API失败, 状态码: {response.status_code}。请检查Cookie是否有效或过期。")

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


#  nohup python3 -u newcommit.py > newcommit.log 2>&1 &
#  tail -f newcommit.log
#  pkill -f newcommit.py
#  ps aux | grep newcommit.py | grep -v grep