# 文件名: test_login.py
import requests
import json

# --- 1. 配置 ---
LOGIN_URL = ""
ADMIN_ACCOUNT = ""
ADMIN_PASSWORD = ""

# --- 2. 请求头 (包含 Origin 和 Referer) ---
HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36',
    'Content-Type': 'application/json',
    'Origin': '',
    'Referer': '',
}

# --- 3. 登录负载 (Payload) ---
# !!! 关键: 把这里修改成和你浏览器里看到的一模一样 !!!
payload = {
    "account": ADMIN_ACCOUNT,
    "password": ADMIN_PASSWORD
}

# --- 4. 执行测试 ---
def test_login():
    print("--- 开始登录测试 ---")
    try:
        response = requests.post(LOGIN_URL, json=payload, headers=HEADERS)
        print(f"请求已发送至: {LOGIN_URL}")
        print(f"HTTP 状态码: {response.status_code}")
        print(f"响应头 (部分): {response.headers.get('Content-Type')}, {response.headers.get('Set-Cookie')}")
        print(f"响应内容: {response.text}")
        
        if response.status_code == 200 and response.cookies.get('token'):
            print("\n--- 测试结果: 登录成功！可以把 Headers 和 Payload 应用到主脚本了。---")
        else:
            print("\n--- 测试结果: 登录失败！请根据上面的响应内容检查 Payload 和 Headers。---")
            
    except Exception as e:
        print(f"测试过程中发生错误: {e}")

if __name__ == "__main__":
    test_login()