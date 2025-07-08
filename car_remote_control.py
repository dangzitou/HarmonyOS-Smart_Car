from flask import Flask, render_template_string, request, jsonify
import socket
import threading
import subprocess
import platform
import time

# 默认小车IP和端口（可被网页端动态修改）
CAR_IP = '192.168.1.102'
CAR_PORT = 50001

app = Flask(__name__)

HTML = '''
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <title>鸿蒙小车遥控器</title>
    <style>
        body { background: #f5f5f7; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'PingFang SC', 'Hiragino Sans GB', 'Microsoft YaHei', sans-serif; display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh; margin: 0; }
        .title { font-size: 2em; margin-bottom: 16px; color: #222; letter-spacing: 2px; }
        .panel { background: #fff; border-radius: 20px; box-shadow: 0 2px 12px #0002; padding: 24px 24px; margin-bottom: 16px; width: 400px; max-width: 96vw; }
        .row { display: flex; align-items: center; margin-bottom: 14px; }
        .row label { min-width: 70px; font-size: 1.1em; color: #555; margin-right: 10px; }
        .mode-select, .speed-slider, .ip-input { flex: 1; font-size: 1em; border-radius: 8px; border: 1px solid #ccc; padding: 6px 8px; }
        .speed-value { width: 48px; text-align: right; margin-left: 10px; color: #007aff; font-weight: bold; font-size: 1.1em; }
        .ip-list { margin-top: 10px; }
        .ip-item { display: inline-block; background: #e0e0e7; border-radius: 8px; padding: 4px 10px; margin: 3px; cursor: pointer; transition: background 0.2s; font-size: 1em; }
        .ip-item:hover { background: #007aff; color: #fff; }
        .scan-btn, .save-btn { margin-left: 10px; border: none; border-radius: 8px; padding: 6px 16px; font-size: 1em; cursor: pointer; transition: background 0.2s; }
        .scan-btn { background: #007aff; color: #fff; }
        .scan-btn:active { background: #005bb5; }
        .save-btn { background: #34c759; color: #fff; }
        .save-btn:active { background: #228c3c; }
        .current-ip { color: #007aff; font-weight: bold; font-size: 1em; margin-left: 70px; margin-top: -8px; margin-bottom: 8px; }
        .controller { display: grid; grid-template-columns: 80px 80px 80px; grid-template-rows: 80px 80px 80px; gap: 18px; justify-content: center; margin: 0 auto; }
        .btn { background: #fff; border: 2px solid #ccc; border-radius: 50%; font-size: 2em; color: #333; box-shadow: 0 2px 8px #0001; transition: all 0.2s; user-select: none; width: 80px; height: 80px; display: flex; align-items: center; justify-content: center; }
        .btn:active { background: #007aff; color: #fff; border-color: #007aff; }
        @media (max-width: 600px) {
            .panel { width: 98vw; min-width: 0; padding: 10px 2vw; }
            .title { font-size: 1.3em; margin-bottom: 10px; }
            .row label { min-width: 60px; font-size: 1em; }
            .mode-select, .speed-slider, .ip-input { font-size: 0.98em; padding: 5px 6px; }
            .controller { grid-template-columns: 60px 60px 60px; grid-template-rows: 60px 60px 60px; gap: 10px; }
            .btn { width: 60px; height: 60px; font-size: 1.3em; }
            .speed-value { width: 36px; font-size: 1em; }
            .current-ip { font-size: 0.95em; margin-left: 60px; }
        }
        @media (max-width: 400px) {
            .panel { padding: 4px 1vw; }
            .controller { grid-template-columns: 44px 44px 44px; grid-template-rows: 44px 44px 44px; gap: 6px; }
            .btn { width: 44px; height: 44px; font-size: 1em; }
            .current-ip { margin-left: 50px; }
        }
    </style>
</head>
<body>
    <div class="title">鸿蒙小车遥控器</div>
    <div class="panel">
        <div class="row">
            <label for="carip">小车IP</label>
            <input id="carip" class="ip-input" type="text" value="{{ car_ip }}" placeholder="如192.168.1.102">
            <button class="save-btn" onclick="saveIP()">保存</button>
        </div>
        <div class="current-ip">当前: <span id="current-ip">{{ car_ip }}</span></div>
        <div class="row">
            <label for="scan">IP扫描</label>
            <input id="scan-segment" class="ip-input" type="text" value="{{ car_ip[:car_ip.rfind('.')]|default('192.168.1') }}" placeholder="如192.168.1">
            <button class="scan-btn" onclick="scanIP()">扫描</button>
        </div>
        <div class="ip-list" id="ip-list"></div>
        <div class="row">
            <label for="mode">模式</label>
            <select id="mode" class="mode-select" onchange="changeMode()">
                <option value="control">遥控</option>
                <option value="trace">循迹</option>
                <option value="obstacle_avoidance">避障</option>
                <option value="stop">停止</option>
            </select>
        </div>
        <div class="row">
            <label for="speed">速度</label>
            <input id="speed" class="speed-slider" type="range" min="4000" max="8000" step="100" value="6000" oninput="updateSpeedValue(this.value)" onchange="changeSpeed(this.value)">
            <span class="speed-value" id="speed-value">6000</span>
        </div>
    </div>
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
        // 发送遥控指令
        function send(cmd) {
            fetch('/cmd', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({cmd})
            });
        }
        // 模式切换
        function changeMode() {
            const mode = document.getElementById('mode').value;
            fetch('/cmd', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({mode})
            });
        }
        // 速度调节
        function changeSpeed(val) {
            fetch('/cmd', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({cmd: 'speed', value: Number(val)})
            });
        }
        // 实时显示速度值
        function updateSpeedValue(val) {
            document.getElementById('speed-value').innerText = val;
        }
        // 保存小车IP
        function saveIP() {
            const ip = document.getElementById('carip').value.trim();
            fetch('/set_ip', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({ip})
            }).then(r => r.json()).then(data => {
                document.getElementById('current-ip').innerText = data.ip;
            });
        }
        // 扫描IP
        function scanIP() {
            const segment = document.getElementById('scan-segment').value.trim();
            if (!segment) return;
            document.getElementById('ip-list').innerHTML = '扫描中...';
            fetch('/scan', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({segment})
            }).then(r => r.json()).then(data => {
                let html = '';
                if (data.ips && data.ips.length > 0) {
                    data.ips.forEach(item => {
                        html += `<span class='ip-item' onclick='setIPFromScan(\"${item.ip}\")'>${item.ip} (${item.time}ms)</span>`;
                    });
                } else {
                    html = '未发现可用IP';
                }
                document.getElementById('ip-list').innerHTML = html;
            });
        }
        // 扫描结果一键设置IP
        function setIPFromScan(ip) {
            document.getElementById('carip').value = ip;
            saveIP();
        }
    </script>
</body>
</html>
'''

# 用于线程安全地保存当前小车IP
car_ip_lock = threading.Lock()
car_ip = CAR_IP

def get_car_ip():
    with car_ip_lock:
        return car_ip

def set_car_ip(ip):
    global car_ip
    with car_ip_lock:
        car_ip = ip

# 发送UDP指令到小车，支持cmd/mode/speed
def send_udp_command(data):
    try:
        msg = None
        if isinstance(data, dict):
            msg = data
        else:
            msg = {"cmd": str(data)}
        import json
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.sendto(json.dumps(msg).encode('utf-8'), (get_car_ip(), CAR_PORT))
        sock.close()
    except Exception as e:
        print('UDP发送失败:', e)

@app.route('/')
def index():
    return render_template_string(HTML, car_ip=get_car_ip())

@app.route('/cmd', methods=['POST'])
def cmd():
    data = request.get_json()
    if 'mode' in data:
        threading.Thread(target=send_udp_command, args=({'mode': data['mode']},)).start()
    elif data.get('cmd') == 'speed' and 'value' in data:
        threading.Thread(target=send_udp_command, args=({'cmd': 'speed', 'value': data['value']},)).start()
    elif 'cmd' in data:
        threading.Thread(target=send_udp_command, args=({'cmd': data['cmd']},)).start()
    return 'ok'

@app.route('/set_ip', methods=['POST'])
def set_ip():
    data = request.get_json()
    ip = data.get('ip', '').strip()
    if ip:
        set_car_ip(ip)
    return jsonify({'ip': get_car_ip()})

@app.route('/scan', methods=['POST'])
def scan():
    data = request.get_json()
    segment = data.get('segment', '').strip()
    if not segment:
        return jsonify({'ips': []})
    # 并发ping 1~254，返回有回应的IP和响应时间
    results = []
    threads = []
    lock = threading.Lock()
    def ping_ip(ip):
        t1 = time.time()
        # 兼容Windows和Linux
        param = '-n' if platform.system().lower() == 'windows' else '-c'
        try:
            output = subprocess.check_output(['ping', param, '1', '-w', '300', ip], stderr=subprocess.DEVNULL, universal_newlines=True)
            t2 = time.time()
            if 'TTL=' in output or 'ttl=' in output:
                with lock:
                    results.append({'ip': ip, 'time': int((t2-t1)*1000)})
        except Exception:
            pass
    # 最多同时32线程
    max_threads = 32
    for i in range(1, 255):
        ip = f'{segment}.{i}'
        while threading.active_count() > max_threads:
            time.sleep(0.01)
        t = threading.Thread(target=ping_ip, args=(ip,))
        t.start()
        threads.append(t)
    for t in threads:
        t.join()
    # 按响应时间排序
    results.sort(key=lambda x: x['time'])
    return jsonify({'ips': results})

if __name__ == '__main__':
    print(f'请用浏览器访问：http://127.0.0.1:5000')
    app.run(host='0.0.0.0', port=5000) 