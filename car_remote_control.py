from flask import Flask, render_template_string, request
import socket
import threading

# 你的车的IP和端口
CAR_IP = '192.168.4.1'  # 修改为你的车的实际IP
CAR_PORT = 50001

app = Flask(__name__)

HTML = '''
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <title>小车遥控器</title>
    <style>
        body { background: #f5f5f7; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'PingFang SC', 'Hiragino Sans GB', 'Microsoft YaHei', sans-serif; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh;}
        .controller { display: grid; grid-template-columns: 80px 80px 80px; grid-template-rows: 80px 80px 80px; gap: 20px;}
        .btn { background: #fff; border: 2px solid #ccc; border-radius: 20px; font-size: 1.5em; color: #333; box-shadow: 0 2px 8px #0001; transition: all 0.2s; user-select: none;}
        .btn:active { background: #007aff; color: #fff; border-color: #007aff;}
        .title { font-size: 2em; margin-bottom: 40px; color: #222;}
    </style>
</head>
<body>
    <div class="title">小车遥控器</div>
    <div class="controller">
        <div></div>
        <button class="btn" ontouchstart="send('forward')" ontouchend="send('stop')" onmousedown="send('forward')" onmouseup="send('stop')" onmouseleave="send('stop')">↑</button>
        <div></div>
        <button class="btn" ontouchstart="send('left')" ontouchend="send('stop')" onmousedown="send('left')" onmouseup="send('stop')" onmouseleave="send('stop')">←</button>
        <div></div>
        <button class="btn" ontouchstart="send('right')" ontouchend="send('stop')" onmousedown="send('right')" onmouseup="send('stop')" onmouseleave="send('stop')">→</button>
        <div></div>
        <button class="btn" ontouchstart="send('backward')" ontouchend="send('stop')" onmousedown="send('backward')" onmouseup="send('stop')" onmouseleave="send('stop')">↓</button>
        <div></div>
    </div>
    <script>
        function send(cmd) {
            fetch('/cmd', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({cmd})
            });
        }
    </script>
</body>
</html>
'''

def send_udp_command(cmd):
    msg = ('{"cmd":"%s"}' % cmd).encode('utf-8')
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.sendto(msg, (CAR_IP, CAR_PORT))
        sock.close()
    except Exception as e:
        print('UDP发送失败:', e)

@app.route('/')
def index():
    return render_template_string(HTML)

@app.route('/cmd', methods=['POST'])
def cmd():
    data = request.get_json()
    cmd = data.get('cmd')
    threading.Thread(target=send_udp_command, args=(cmd,)).start()
    return 'ok'

if __name__ == '__main__':
    print(f'请用浏览器访问：http://127.0.0.1:5000')
    app.run(host='0.0.0.0', port=5000) 