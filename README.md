# ESP32-C3 Remote Wake-on-LAN (MQTT & Telegram)

Dự án điều khiển bật máy tính từ xa (Wake-on-LAN) sử dụng mạch **ESP32-C3 Super Mini**. Hỗ trợ đồng thời hai giao thức: MQTT (tích hợp nhà thông minh) và Telegram Bot (điều khiển qua tin nhắn).

## ✨ Tính năng nổi bật
- **Dual-Mode Control:** Điều khiển qua App MQTT hoặc Telegram Bot.
- **Multi-PC Support:** Có thể bật nhiều máy tính trong cùng mạng LAN bằng cách gửi địa chỉ MAC.
- **High Reliability (24/7):**
  - Tích hợp **Watchdog Timer** tự động khởi động lại nếu treo mạch.
  - Tự động kết nối lại WiFi/MQTT khi mất mạng.
  - Tự động reset định kỳ mỗi 7 ngày để giải phóng bộ nhớ.
- **LED Feedback:** Báo hiệu trạng thái qua đèn LED tích hợp (Sáng: Online, Nháy: Đã gửi lệnh, Nháy nhanh: Lỗi).

## 🛠 Phần cứng yêu cầu
- Mạch **ESP32-C3 Super Mini**.
- Cáp USB-C.
- Nguồn 5V ổn định (Sạc điện thoại).

## 🚀 Hướng dẫn cài đặt

### 1. Chuẩn bị
- Cài đặt VS Code và extension **PlatformIO**.
- Tạo Telegram Bot qua [@BotFather](https://t.me/botfather) để lấy `Token`.
- Lấy Chat ID cá nhân qua [@IDBot](https://t.me/myidbot).

### 2. Cấu hình Code
- Copy file `include/config.h.example` thành `include/config.h`.
- Mở `include/config.h` và điền các thông số sau:

| Biến | Phân loại | Giải thích |
| :--- | :--- | :--- |
| `ssid` / `password` | **Bắt buộc** | WiFi để ESP32 kết nối mạng. |
| `bot_token` | **Bắt buộc** | Lấy từ @BotFather. |
| `chat_id` | **Bắt buộc** | Lấy từ @myidbot (để chỉ bạn mới có quyền bật máy). |
| `mqtt_server` | *Tùy chọn* | Mặc định dùng `broker.emqx.io` (có thể đổi sang server riêng). |
| `mqtt_port` | *Tùy chọn* | Cổng MQTT (mặc định: `1883`). |
| `topic_command` | *Tùy chọn* | Có thể đổi tên tùy ý (nhưng phải trùng với App điện thoại). |
| `topic_status` | *Tùy chọn* | Dùng để ESP32 báo cáo trạng thái. |
| `bot_mtbs` | *Tùy chọn* | Tốc độ check tin nhắn (mặc định `3000`ms). |
| `LED_PIN` | Phần cứng | Chân LED (Super Mini thường là chân `8`). |

### 3. Nạp Code
- Kết nối ESP32-C3 vào máy tính.
- Nhấn nút **Upload** trên PlatformIO.

## 📱 Cách sử dụng

### Qua Telegram
- Nhắn `/start` để kiểm tra trạng thái hoạt động.
- Nhắn `/mqtt` để xem lại thông số cấu hình MQTT (Server, Port, Topic).
- Nhắn `/wake AA:BB:CC:DD:EE:FF` để bật máy tính.

### Qua MQTT
- Sử dụng App (MQTT Dash, MQTTTool) gửi địa chỉ MAC tới topic `esp32/wol/command`.

## ⚠️ Lưu ý quan trọng
- Máy tính mục tiêu phải được bật tính năng **Wake-on-LAN** trong BIOS và cài đặt Card mạng (Ethernet).
- WoL thường chỉ hoạt động qua kết nối dây LAN (không hỗ trợ qua WiFi của máy tính).

---
*Developed with ❤️ for ESP32-C3 Super Mini.*
