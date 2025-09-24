import smtplib
from email.mime.text import MIMEText
from email.header import Header

# --- 1. 邮件配置区域 ---
# 请确保这里的每一项都和您之前的脚本一致且正确

SMTP_SERVER = "smtp.qq.com"
SMTP_PORT = 465
SENDER_EMAIL = ""      # 您的发件QQ邮箱
SENDER_PASSWORD = ""   # 您的16位授权码
RECEIVER_EMAIL = ""  # 您的收件邮箱

# --- 2. 邮件发送函数 (无需修改) ---

def send_email_notification(subject, body):
    """发送邮件通知的函数"""
    try:
        message = MIMEText(body, 'plain', 'utf-8')
        message['From'] = SENDER_EMAIL
        message['To'] = Header(f"收件人 <{RECEIVER_EMAIL}>", 'utf-8')
        message['Subject'] = Header(subject, 'utf-8')

        print("正在连接到SMTP服务器...")
        server = smtplib.SMTP_SSL(SMTP_SERVER, SMTP_PORT)

        print("正在使用授权码登录邮箱...")
        server.login(SENDER_EMAIL, SENDER_PASSWORD)

        print("正在发送邮件...")
        server.sendmail(SENDER_EMAIL, [RECEIVER_EMAIL], message.as_string())

        server.quit()
        print("邮件发送成功！")
        return True
    except Exception as e:
        print(f"邮件发送失败: {e}")
        return False

# --- 3. 程序主入口 ---

if __name__ == "__main__":
    print("--- 开始邮件发送功能测试 ---")

    # 定义测试邮件的主题和内容
    test_subject = "【测试邮件】来自您的Python监控脚本"
    test_body = """
您好！

如果您能收到这封邮件，这表明您的Python脚本中的SMTP配置是完全正确的。

授权码、服务器地址和端口均已验证通过。

现在您可以放心地将此邮件功能用于您的监控程序了。
"""

    # 调用发送函数
    send_email_notification(test_subject, test_body)

    print("--- 测试脚本执行完毕 ---")