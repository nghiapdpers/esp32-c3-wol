import os
import re
import json
import uuid
import paho.mqtt.client as mqtt
import time
import sys
import argparse

def get_config(config_file=None):
    """
    Đọc cấu hình cho Agent. 
    Thứ tự ưu tiên: 
    1. File được chỉ định qua CLI (--config)
    2. File 'agent_config.json' nằm cùng thư mục với script/exe
    3. File '../../include/config.h' (Dùng khi dev/build trong project)
    """
    config = {
        "mqtt_server": "broker.emqx.io",
        "mqtt_port": 1883,
        "secret_key": "default"
    }

    # 1 & 2. Kiểm tra file JSON
    json_path = config_file if config_file else os.path.join(os.path.dirname(sys.argv[0]), "agent_config.json")
    if os.path.exists(json_path):
        try:
            with open(json_path, "r", encoding="utf-8") as f:
                json_data = json.load(f)
                config.update(json_data)
                print(f"📄 Đã tải cấu hình từ file: {json_path}")
                return config
        except Exception as e:
            print(f"⚠️ Lỗi khi đọc file JSON {json_path}: {e}")

    # 3. Kiểm tra config.h (Fallback cho môi trường dev)
    config_h_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../include/config.h"))
    if os.path.exists(config_h_path):
        try:
            with open(config_h_path, "r", encoding="utf-8") as f:
                content = f.read()
                server_match = re.search(r'const char\*\s+mqtt_server\s*=\s*"([^"]+)"', content)
                port_match = re.search(r'const int\s+mqtt_port\s*=\s*(\d+)', content)
                key_match = re.search(r'const char\*\s+secret_key\s*=\s*"([^"]+)"', content)
                
                if server_match: config["mqtt_server"] = server_match.group(1)
                if port_match: config["mqtt_port"] = int(port_match.group(1))
                if key_match: config["secret_key"] = key_match.group(1)
                print(f"🔗 Đã tự động đồng bộ cấu hình từ: config.h")
        except Exception as e:
            print(f"⚠️ Lỗi khi đọc config.h: {e}")
    else:
        print(f"ℹ️ Không tìm thấy config.h, sử dụng cấu hình mặc định hoặc JSON.")

    return config

def get_all_mac_addresses():
    """Lấy danh sách tất cả địa chỉ MAC của các card mạng thực"""
    import psutil
    macs = []
    for interface, addrs in psutil.net_if_addrs().items():
        for addr in addrs:
            if addr.family == psutil.AF_LINK:
                mac = addr.address.replace("-", ":").upper()
                if mac and mac != "00:00:00:00:00:00":
                    macs.append((interface, mac))
    return macs

def get_mac_address():
    """Lấy địa chỉ MAC mặc định (thường là card mạng chính)"""
    mac_num = uuid.getnode()
    mac = ':'.join(['{:02x}'.format((mac_num >> ele) & 0xff) for ele in range(0,8*6,8)][::-1])
    return mac.upper()

def normalize_mac(mac):
    """Chuẩn hóa MAC: xóa :, -, . và viết hoa"""
    if not mac: return ""
    return mac.replace(":", "").replace("-", "").replace(".", "").upper()

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"✅ Đã kết nối MQTT Broker ({userdata['mqtt_server']})")
        
        # Subscribe lệnh
        topic_cmd = f"esp32_c3_wol/{userdata['secret_key']}/cmd"
        client.subscribe(topic_cmd)
        print(f"📡 Đang lắng nghe lệnh trên: {topic_cmd}")
        
        # Publish trạng thái Online ngay lập tức
        publish_status(client, userdata, "online")
    else:
        print(f"❌ Kết nối thất bại, mã lỗi: {rc}")

def publish_status(client, userdata, status):
    """Gửi trạng thái Online/Offline với MAC đã chuẩn hóa"""
    my_mac = userdata.get("override_mac") or get_mac_address()
    n_mac = normalize_mac(my_mac)
    topic_status = f"esp32_c3_wol/{userdata['secret_key']}/status/{n_mac}"
    client.publish(topic_status, status, retain=True)
    if status == "online":
        print(f"🟢 [Heartbeat] Đã gửi trạng thái Online: {n_mac}")

def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode("utf-8")
        data = json.loads(payload)
        
        my_mac = userdata.get("override_mac") or get_mac_address()
        cmd = data.get("cmd")
        mac_target = data.get("mac")
        
        if cmd == "shutdown" and normalize_mac(mac_target) == normalize_mac(my_mac):
            print(f"🛑 Nhận lệnh SHUTDOWN cho máy này ({my_mac})")
            if os.name == 'nt': # Windows
                os.system("shutdown /s /f /t 0")
            else: # Linux
                print("🐧 Thực hiện lệnh shutdown trên Linux...")
                res = os.system("sudo shutdown now")
                if res != 0:
                    os.system("shutdown now") # Thử không có sudo
    except Exception as e:
        print(f"⚠️ Lỗi khi xử lý message: {e}")

def main():
    parser = argparse.ArgumentParser(description="ESP32 WoL - PC Agent")
    parser.version = "1.1.1"
    parser.add_argument("--config", help="Đường dẫn tới file cấu hình JSON")
    parser.add_argument("--mac", help="Ghi đè địa chỉ MAC (VD: AA:BB:CC:DD:EE:FF)")
    args = parser.parse_args()

    config = get_config(args.config)
    my_mac = args.mac if args.mac else get_mac_address()
    config["override_mac"] = my_mac
    
    print("🖥 --- ESP32 WoL: PC AGENT STARTING ---")
    
    print("\n🔍 Danh sách Card mạng tìm thấy:")
    try:
        all_macs = get_all_mac_addresses()
        for interface, mac in all_macs:
            print(f"  - {interface}: {mac} {' (Đang dùng)' if normalize_mac(mac) == normalize_mac(my_mac) else ''}")
    except Exception as e:
        print(f"  ⚠️ Không thể liệt kê đầy đủ card mạng: {e}")
    
    print(f"\n📍 MAC Address đang sử dụng: {my_mac} (ID: {normalize_mac(my_mac)})")
    print(f"🔑 Secret Key: {config['secret_key'][:4]}****")

    client = mqtt.Client(userdata=config)
    client.on_connect = on_connect
    client.on_message = on_message

    # Thiết lập Last Will (Trạng thái Offline khi mất kết nối đột ngột)
    n_mac = normalize_mac(my_mac)
    topic_status = f"esp32_c3_wol/{config['secret_key']}/status/{n_mac}"
    client.will_set(topic_status, "offline", retain=True)

    # Tự động kích hoạt SSL nếu dùng port 8883
    if config["mqtt_port"] == 8883:
        print("🔒 Kích hoạt kết nối bảo mật (SSL/TLS)")
        client.tls_set()

    client.reconnect_delay_set(min_delay=1, max_delay=120)
    
    # Kết nối ban đầu
    try:
        client.connect(config["mqtt_server"], config["mqtt_port"], 60)
    except Exception as e:
        print(f"❌ Không thể kết nối MQTT ban đầu: {e}")

    # Chạy loop MQTT trong thread riêng để có thể gửi heartbeat ở thread chính
    client.loop_start()

    try:
        while True:
            if client.is_connected():
                publish_status(client, config, "online")
            else:
                print("⏳ Đang đợi kết nối MQTT...")
            time.sleep(30) # Heartbeat mỗi 30 giây
    except KeyboardInterrupt:
        print("\n🛑 Đang dừng Agent...")
        publish_status(client, config, "offline")
        client.disconnect()
        client.loop_stop()
    except Exception as e:
        print(f"⚠️ Lỗi thực thi: {e}")
        client.loop_stop()

if __name__ == "__main__":
    main()
