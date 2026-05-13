# PROJECT_OVERVIEW.md
> **⚠️ AI Agent: Đọc file này TRƯỚC KHI làm bất kỳ task nào trong project này.**

**Project:** ESP32-C3 Wake-on-LAN & Shutdown-on-LAN
**Last Updated:** 2026-05-13
**Tech Stack:** C++ (ESP32-Arduino), MQTT (EMQX), Telegram Bot API, C++ (PC Agent).

---

## 1. Architecture Overview
Hệ thống gồm 3 thành phần chính giao tiếp thời gian thực qua MQTT (SSL):
- **ESP32 Controller:** Nhận lệnh từ Telegram Bot hoặc Web Dashboard, gửi gói tin Magic Packet (WOL) hoặc Publish lệnh Shutdown qua MQTT.
- **PC Agent (C++):** Chạy ngầm trên máy tính mục tiêu, lắng nghe MQTT. Khi nhận lệnh Shutdown khớp với địa chỉ MAC của mình, nó thực hiện tắt máy hệ điều hành.
- **Web Dashboard:** Giao diện điều khiển (GitHub Pages) đồng bộ trạng thái qua MQTT.

## 2. Technology Stack & Decisions
- **MQTT Broker:** Sử dụng `broker.emqx.io` (Port 1883/8883).
- **Communication Pattern:** Sử dụng Secret Key để định danh topic nhằm bảo mật (`esp32_c3_wol/SECRET_KEY/cmd`).
- **PC Agent:** Viết bằng C++ nguyên bản (Native) để tối ưu RAM (< 2MB) và kích thước binary. Sử dụng thư viện `libmosquitto`.

## 3. Project Structure
- `src/main.cpp`: Toàn bộ logic điều khiển, quản lý danh sách PC và xử lý Telegram/MQTT cho ESP32.
- `include/config.h`: Cấu hình WiFi, MQTT, Telegram Token (User tự điền).
- `pc-agent/`: Chứa mã nguồn C++ dành cho máy tính mục tiêu.
- `partitions.csv`: Cấu hình bộ nhớ cho ESP32-C3.

## 4. Key Features & Status
- [x] Wake-on-LAN (WOL) - Done.
- [x] Quản lý danh sách thiết bị qua Web/Telegram - Done.
- [x] Shutdown-on-LAN (SOL) via MQTT - Done (New).
- [ ] Theo dõi trạng thái Online/Offline của PC - Planned.

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
| 2026-05-13 | Liên kết PC Agent với config.h và cập nhật tài liệu README | Antigravity |
| 2026-05-13 | Tích hợp tính năng Shutdown-on-LAN và tạo PC Agent C++ | Antigravity |
| 2026-05-06 | Khởi tạo dự án WOL cơ bản | Agent |
