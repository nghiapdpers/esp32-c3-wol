# 🚀 ESP32-C3 Wake-on-LAN (WoL) - Remote Dashboard

Hệ thống điều khiển máy tính từ xa (Wake-on-LAN) mạnh mẽ, bảo mật và chuyên nghiệp dành cho ESP32-C3. Dự án cho phép bạn quản lý và đánh thức nhiều máy tính từ bất cứ đâu thông qua Web, Telegram và MQTT.

## 🔥 Danh sách chức năng chi tiết

### 1. Điều khiển & Quản lý máy tính
- **Wake-on-LAN từ xa:** Kích hoạt gói tin Magic Packet để bật máy tính qua Internet.
- **Quản lý danh sách thiết bị:** Thêm, sửa, xóa máy tính trực tiếp từ giao diện Web.
- **Lưu trữ vĩnh viễn:** Danh sách máy tính được lưu vào bộ nhớ Flash (Preferences) của ESP32, không bị mất khi mất điện.

### 2. Giao diện Web Dashboard (GitHub Pages)
- **Glassmorphism UI:** Giao diện hiện đại, mượt mà và tương thích hoàn toàn với điện thoại.
- **Đồng bộ thời gian thực:** Web tự động lấy danh sách máy từ ESP32 ngay khi mở.
- **Thông số hệ thống:** Hiển thị thời gian hoạt động (Uptime), cường độ tín hiệu WiFi (RSSI) và dung lượng RAM trống của ESP32.

### 3. Tích hợp Telegram Bot
- **Menu nút bấm thông minh:** Hiển thị danh sách máy dưới dạng nút bấm để điều khiển nhanh.
- **Lệnh Chat:** Hỗ trợ các lệnh `/list`, `/status`, `/web`.
- **Thông báo tức thì:** Gửi tin nhắn xác nhận mỗi khi có lệnh bật máy thành công.
- **Magic Link:** Tạo đường dẫn đăng nhập tự động vào Web Dashboard kèm sẵn mã Secret Key.

### 4. Bảo mật & Độ tin cậy
- **Mã hóa SSL/TLS:** Toàn bộ giao tiếp qua cổng 8883 (MQTT) và HTTPS đều được mã hóa.
- **Xác thực Secret Key:** Chỉ những thiết bị có mã khóa trùng khớp mới có thể giao tiếp với ESP32.
- **Hệ thống tự phục hồi:** 
  - **Watchdog Timer (WDT):** Tự động khởi động lại ESP32 nếu bị treo hoặc mất kết nối.
  - **Tự động kết nối lại:** Tự tìm lại WiFi và Broker MQTT khi bị ngắt mạng.
- **Zero-Local-Server:** Loại bỏ Web Server nội bộ để đảm bảo không có lỗ hổng truy cập từ LAN.

### 5. Phản hồi phần cứng
- **LED Status:** Đèn LED trên board phản hồi trạng thái kết nối và nháy báo hiệu khi gửi gói tin thành công/thất bại.

## 🛠️ Cách thức hoạt động
- **Giao thức:** MQTT over SSL (Cổng 8883).
- **Cấu trúc Topic:** `esp32_c3_wol/[secret_key]/cmd` và `esp32_c3_wol/[secret_key]/res`.

## ⚙️ Cài đặt & Triển khai
1. Cấu hình `include/config.h` (từ file mẫu `.example`).
2. Nạp code vào ESP32-C3 bằng PlatformIO.
3. Đẩy nội dung thư mục `data` lên nhánh `gh-pages` trên GitHub của bạn.

## 📄 Giấy phép
Dự án được phát hành dưới giấy phép MIT.
