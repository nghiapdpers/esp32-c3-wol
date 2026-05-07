# ESP32-C3 Remote Wake-on-LAN Manager

Dự án điều khiển bật máy tính từ xa (Wake-on-LAN) chuyên nghiệp sử dụng mạch **ESP32-C3 Super Mini**. Hỗ trợ quản lý danh sách nhiều máy tính, lưu trữ bền vững và giao diện nút bấm tương tác qua Telegram.

## ✨ Tính năng nổi bật
- **PC List Management:** Thêm, xóa và lưu trữ danh sách nhiều máy tính trực tiếp vào bộ nhớ Flash (NVS), không mất dữ liệu khi mất điện.
- **Interactive UI:** Giao diện điều khiển qua Telegram với **Inline Keyboard (Nút bấm trực tiếp)**.
- **Dual-Mode Control:** Hỗ trợ song song Telegram Bot và App MQTT.
- **High Reliability (24/7):** Watchdog Timer, tự động phục hồi WiFi/MQTT, và tự động reset định kỳ.
- **Instant Notification:** Báo cáo trạng thái thành công/thất bại chi tiết ngay lập tức qua Telegram Bot.

## 🛠 Phần cứng yêu cầu
- Mạch **ESP32-C3 Super Mini**.
- Cáp USB-C và Nguồn 5V ổn định.

## 🚀 Hướng dẫn cài đặt

### 1. Chuẩn bị
- Cài đặt VS Code và extension **PlatformIO**.
- Tạo Telegram Bot qua @BotFather và lấy Chat ID qua @myidbot.

### 2. Cấu hình Code
- Copy file `include/config.h.example` thành `include/config.h`.
- Điền thông số vào file `include/config.h`:

| Biến | Phân loại | Giải thích |
| :--- | :--- | :--- |
| `ssid` / `password` | **Bắt buộc** | WiFi để ESP32 kết nối mạng. |
| `bot_token` | **Bắt buộc** | Token lấy từ @BotFather. |
| `chat_id` | **Bắt buộc** | ID cá nhân lấy từ @myidbot. |
| `mqtt_server` | *Tùy chọn* | Broker MQTT (mặc định: `broker.emqx.io`). |
| `topic_command` | *Tùy chọn* | Topic nhận lệnh MQTT. |
| `bot_mtbs` | *Tùy chọn* | Tốc độ check tin nhắn (mặc định `3000`ms). |
| `LED_PIN` | Phần cứng | Chân LED (Super Mini thường là chân `8`). |

### 3. Nạp Code
- Nhấn nút **Upload** trên PlatformIO.

## 📱 Cách sử dụng

### Qua Telegram Bot (Khuyên dùng)
- `/start` : Xem menu hướng dẫn.
- `/add <Tên> <MAC>` : Thêm máy (Ví dụ: `/add PC1 AA:BB:CC:DD:EE:FF`).
- `/list` : Hiện danh sách nút bấm để bật máy.
- `/delete <Tên>` : Xóa máy khỏi danh sách.
- `/status` : Xem trạng thái hệ thống (Uptime, WiFi, RAM).
- `/mqtt` : Xem lại thông số cấu hình MQTT.

### Qua MQTT App
- Gửi địa chỉ MAC tới `topic_command`. ESP32 sẽ thực thi và báo kết quả về Telegram của bạn.

---
*Developed with ❤️ for ESP32-C3 Super Mini.*
