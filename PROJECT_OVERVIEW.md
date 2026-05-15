# PROJECT_OVERVIEW.md
> **⚠️ AI Agent: Đọc file này TRƯỚC KHI làm bất kỳ task nào trong project này.**

**Project:** ESP32-C3 Wake-on-LAN & Shutdown-on-LAN
**Last Updated:** 2026-05-14
**Tech Stack:** C++ (ESP32-Arduino), MQTT (EMQX), Telegram Bot API, Python (PC Agent).

---

## 1. Architecture Overview
Hệ thống gồm 3 thành phần chính giao tiếp thời gian thực qua MQTT (SSL):
- **ESP32 Controller:** Nhận lệnh từ Telegram Bot hoặc Web Dashboard, gửi gói tin Magic Packet (WOL) hoặc Publish lệnh Shutdown qua MQTT.
- **PC Agent (Python):** Chạy ngầm trên máy tính mục tiêu, lắng nghe MQTT. Khi nhận lệnh Shutdown khớp với địa chỉ MAC của mình, nó thực hiện tắt máy hệ điều hành.
- **Web Dashboard:** Giao diện điều khiển (GitHub Pages) đồng bộ trạng thái qua MQTT.

## 2. Technology Stack & Decisions
- **MQTT Broker:** Sử dụng `broker.emqx.io` (Port 1883/8883).
- **Communication Pattern:** Sử dụng Secret Key để định danh topic nhằm bảo mật (`esp32_c3_wol/SECRET_KEY/cmd`).
- **PC Agent (Python):** Chạy ngầm trên máy tính mục tiêu, lắng nghe MQTT. Hỗ trợ SSL (Port 8883), ghi đè MAC qua CLI và cấu hình linh hoạt qua JSON.

## 3. Project Structure
- `src/main.cpp`: Toàn bộ logic điều khiển, quản lý danh sách PC và xử lý Telegram/MQTT cho ESP32.
- `include/config.h`: Cấu hình WiFi, MQTT, Telegram Token.
- `pc-agent/`: Chứa mã nguồn Python và công cụ đóng gói cho máy tính mục tiêu.
- `partitions.csv`: Cấu hình bộ nhớ cho ESP32-C3.

## 4. Key Features & Status
- [x] Wake-on-LAN (WOL) - Done.
- [x] Quản lý danh sách thiết bị qua Web/Telegram - Done.
- [x] Shutdown-on-LAN (SOL) via MQTT - Done.
- [x] Theo dõi trạng thái Online/Offline của PC - Done (Agent Heartbeat).

## 5. Established Patterns & Conventions
- **Command Format:** JSON string `{"cmd": "wol/shutdown/sync/add", "mac": "...", "name": "..."}`.
- **Security:** Mọi giao tiếp đều yêu cầu `secret_key` trong đường dẫn Topic. ESP32 không mở Web Server local để tránh lỗ hổng LAN.

## 6. Domain Rules & Business Logic
- Một Secret Key có thể điều khiển nhiều máy tính.
- Lệnh Shutdown yêu cầu PC Agent phải đang chạy và kết nối Internet.

## 7. Known Issues & Gotchas
- **MAC Address:** Card WiFi và Card LAN có MAC khác nhau. Lệnh WOL cần MAC của Card LAN (nối dây), trong khi SOL cần MAC của Card mạng đang kết nối Internet.
- **Firewall:** Windows Firewall có thể chặn kết nối MQTT của PC Agent nếu không được phép.

## 8. Changelog (newest first)
| Date | Change | Author |
|------|--------|--------|
| 2026-05-15 | Sửa lỗi không đồng nhất trạng thái PC: Chuẩn hóa MAC triệt để và thêm Heartbeat cho Agent | Antigravity |
| 2026-05-15 | Đồng bộ thông báo Shutdown trên Telegram khi gửi lệnh từ Web Dashboard | Antigravity |
| 2026-05-15 | Tích hợp hiển thị trạng thái Online/Offline của PC trên Telegram Bot (🟢/⚪) | Antigravity |
| 2026-05-15 | Cập nhật hướng dẫn sử dụng SimpleServiceManager (SSM) thay thế NSSM cho PC Agent Service | Antigravity |
| 2026-05-14 | Đồng bộ tài liệu README (VN & EN) với quy trình GitHub Release mới | Antigravity |
| 2026-05-14 | Tự động tạo GitHub Release sau khi build PC Agent thành công | Antigravity |
| 2026-05-14 | Bảo trì & Bảo mật: Fix lỗi delimiter, validate MAC, hỗ trợ MQTT Port 1883/8883, và Heartbeat cho Agent | Antigravity |
| 2026-05-14 | Nâng cấp PC Agent: Hỗ trợ SSL, CLI Arguments và file cấu hình JSON | Antigravity |
| 2026-05-14 | Chuyển đổi PC Agent sang Python để dễ bảo trì và đóng gói | Antigravity |
| 2026-05-13 | Liên kết PC Agent với config.h và cập nhật tài liệu README | Antigravity |
| 2026-05-13 | Tích hợp tính năng Shutdown-on-LAN và tạo PC Agent C++ | Antigravity |
| 2026-05-06 | Khởi tạo dự án WOL cơ bản | Agent |
