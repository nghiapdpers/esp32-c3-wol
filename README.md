# 🚀 ESP32-C3 Wake-on-LAN (WoL) - Remote Dashboard

Hệ thống điều khiển máy tính từ xa (Wake-on-LAN) mạnh mẽ, bảo mật và thẩm mỹ dành cho ESP32-C3. Hỗ trợ điều khiển qua **Telegram Bot** và **Web Dashboard (GitHub Pages)** mà không cần mở port router.

## ✨ Tính năng nổi bật

- 🌍 **Điều khiển từ xa toàn cầu:** Không cần Port Forwarding, hoạt động tốt sau NAT.
- 🛡️ **Bảo mật tối đa:** 
  - Giao thức MQTT qua **SSL/TLS (Port 8883)** mã hóa toàn diện.
  - Xác thực qua **Secret Key** và Topic bí mật.
  - Không chạy Web Server local để giảm bề mặt tấn công.
- 🤖 **Telegram Bot:** Nhận thông báo, xem trạng thái và kích hoạt WoL qua nút bấm tiện lợi.
- 🎨 **Premium Web UI:** Giao diện Glassmorphism hiện đại, hỗ trợ cài đặt GitHub Pages.
- 🔗 **Magic Link:** Tự động đăng nhập vào Dashboard từ Telegram chỉ với một cú click.

## 🛠️ Cách thức hoạt động

1. **Frontend:** Trang web tĩnh được host trên GitHub Pages, kết nối tới MQTT Broker qua WebSockets (WSS).
2. **Bridge:** MQTT Broker (EMQX) đóng vai trò trung gian truyền lệnh giữa Web/Bot và ESP32.
3. **Hardware:** ESP32-C3 lắng nghe lệnh từ Broker qua kết nối SSL bảo mật và gửi gói tin Magic Packet trong mạng nội bộ.

## ⚙️ Cài đặt

### 1. Chuẩn bị phần cứng
- ESP32-C3 (Ví dụ: Super Mini).
- Cáp USB-C.

### 2. Cấu hình phần mềm
1. Sao chép file `include/config.h.example` thành `include/config.h`.
2. Cập nhật các thông số sau:
   - `ssid` / `password`: WiFi nhà bạn.
   - `bot_token` / `chat_id`: Thông tin từ BotFather.
   - `secret_key`: Mã bí mật của riêng bạn (Dùng để xác thực trên Web).
   - `gh_pages_url`: Đường dẫn trang GitHub Pages của bạn.

### 3. Nạp Code
- Sử dụng **PlatformIO** trong VS Code.
- Chạy `Upload` để nạp firmware.
- (Không cần chạy `Upload Filesystem Image` vì chúng ta dùng GitHub Pages).

### 4. Thiết lập Web Dashboard
1. Tạo Repo GitHub và upload các file trong thư mục `data/` lên đó.
2. Bật tính năng **GitHub Pages** trong phần Settings của Repo.

## 📱 Hướng dẫn sử dụng

1. Mở Telegram Bot của bạn.
2. Gõ `/start` để xem danh sách lệnh.
3. Gõ `/web` để nhận **Magic Link**. Click vào link để mở Dashboard mà không cần nhập key.
4. Trên Dashboard, ấn nút **(+)** để thêm máy tính mới (Tên và MAC Address).
5. Nhấn **Wake** để đánh thức máy tính từ bất cứ đâu!

## 📄 Giấy phép
Dự án này được phát hành dưới giấy phép MIT.
