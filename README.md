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
- **Lệnh Chat:** Hỗ trợ các lệnh `/list`, `/add Name MAC`, `/delete Name`, `/status`, `/mqtt`, `/web`.
- **Thông báo tức thì:** Gửi tin nhắn xác nhận mỗi khi có lệnh bật máy thành công.
- **Magic Link:** Tạo đường dẫn đăng nhập tự động vào Web Dashboard kèm sẵn mã Secret Key.

### 4. Bảo mật & Độ tin cậy
- **Mã hóa SSL/TLS:** Toàn bộ giao tiếp qua cổng 8883 (MQTT) và HTTPS đều được mã hóa.
- **Xác thực Secret Key:** Chỉ những thiết bị có mã khóa trùng khớp mới có thể giao tiếp với ESP32.
- **Hệ thống tự phục hồi:** 
  - **Watchdog Timer (WDT):** Tự động khởi động lại ESP32 nếu bị treo hoặc mất kết nối.
  - **Auto-Reconnect:** Tự tìm lại WiFi và Broker MQTT khi bị ngắt mạng.
- **Zero-Local-Server:** Loại bỏ Web Server nội bộ để đảm bảo không có lỗ hổng truy cập từ LAN.

### 5. Cấu hình linh hoạt (Modular Features)
Dự án được thiết kế để bạn có thể chọn sử dụng 1, 2 hoặc cả 3 phương thức điều khiển tùy nhu cầu:
- **Chỉ dùng Telegram:** Bỏ trống `mqtt_server` trong `config.h`.
- **Chỉ dùng MQTT:** Bỏ trống `bot_token` và `chat_id`.
- **Không dùng Web Dashboard:** Bỏ trống `secret_key`. Khi đó, MQTT Topic sẽ mặc định sử dụng từ khóa `default` (Xem Bước 6).
ESP32 sẽ tự động nhận diện và chỉ khởi chạy các tính năng đã được cấu hình, giúp tiết kiệm tài nguyên và hoạt động ổn định.

---

## 🛠️ Hướng dẫn cài đặt chi tiết (Step-by-step)

### Bước 1: Chuẩn bị Telegram Bot
1. Tìm kiếm `@BotFather` trên Telegram và gõ `/newbot` để tạo bot mới.
2. Lưu lại **API Token** mà BotFather cung cấp.
3. Tìm kiếm `@userinfobot` để lấy **Chat ID** cá nhân của bạn.

### Bước 2: Cấu hình mã nguồn (ESP32)
1. Tải mã nguồn dự án về máy tính.
2. Vào thư mục `include/`, sao chép file `config.h.example` thành `config.h`.
3. Mở file `config.h` và điền các thông tin:
   - `ssid` / `password`: Thông tin WiFi nhà bạn.
   - `mqtt_server`: Địa chỉ Broker (mặc định là `broker.emqx.io`).
   - `mqtt_port`: Cổng SSL (mặc định là `8883`).
   - `bot_token`: Token từ BotFather.
   - `chat_id`: ID từ userinfobot.
   - `secret_key`: Mã bí mật tùy ý của bạn (dùng để bảo mật Topic và đăng nhập web).
   - `gh_pages_url`: Đường dẫn GitHub Pages của bạn (xem Bước 4).

### Bước 3: Nạp Firmware
1. Mở dự án bằng **VS Code** có cài sẵn plugin **PlatformIO**.
2. Kết nối ESP32-C3 vào máy tính qua cổng USB.
3. Nhấn biểu tượng mũi tên (→) **Upload** trên thanh công cụ của PlatformIO để nạp code.

### Bước 4: Triển khai Web Dashboard (GitHub Pages)
> **💡 Lưu ý:** Nếu bạn không muốn tự tạo trang GitHub Pages riêng, bạn có thể sử dụng giao diện mặc định của dự án tại: `https://nghiapdpers.github.io/esp32-c3-wol/`. Chỉ cần nhập Secret Key của bạn vào phần Cài đặt là có thể sử dụng ngay.
> **⚠️ Cảnh báo:** Khi sử dụng Dashboard mặc định, bạn **KHÔNG ĐƯỢC THAY ĐỔI** tiền tố topic `esp32_c3_wol` trong mã nguồn, nếu không trang web sẽ không thể kết nối tới ESP32 của bạn.

Để tự triển khai trang riêng:
1. Tạo một Repository mới trên GitHub cá nhân của bạn.
2. Đẩy toàn bộ mã nguồn lên nhánh `main`.
3. Tạo nhánh `gh-pages` và chỉ đưa 3 file trong thư mục `data` ra ngoài root của nhánh này:
   ```bash
   git checkout --orphan gh-pages
   git rm -rf .
   # Chỉ copy index.html, style.css, script.js vào đây
   git add . && git commit -m "Deploy Web" && git push origin gh-pages
   ```
4. Vào **Settings** -> **Pages** của Repo, chọn nhánh `gh-pages` và nhấn Save. GitHub sẽ cung cấp một URL (đây là `gh_pages_url`).

### Bước 5: Cấu hình máy tính mục tiêu (PC)
1. Bật tính năng **Wake-on-LAN** trong BIOS/UEFI của máy tính cần bật (thường nằm trong phần Power Management).
2. Trong Windows, vào *Device Manager* -> *Network Adapters* -> Card mạng của bạn -> *Properties* -> *Advanced* -> Bật các tính năng như "Magic Packet", "Wake on LAN".
3. Vào tab *Power Management* -> Tích chọn "Allow this device to wake the computer".

### Bước 6: Cấu hình MQTT App (Nếu dùng app bên thứ 3)
Nếu bạn muốn dùng các app như *MQTT Dash* hoặc *MQTT Panel* trên điện thoại:
1. **Broker:** `broker.emqx.io` (hoặc broker bạn dùng).
2. **Port:** `8883` (chọn giao thức SSL/TLS).
3. **Command Topic:** `esp32_c3_wol/YOUR_SECRET_KEY/cmd` (Nếu không có key, dùng: `esp32_c3_wol/default/cmd`)
4. **Response Topic:** `esp32_c3_wol/YOUR_SECRET_KEY/res` (Nếu không có key, dùng: `esp32_c3_wol/default/res`)
5. **Cấu trúc lệnh (JSON):**
   - Đánh thức: `{"cmd":"wol", "mac":"00:1A:...", "name":"PC-Name"}`
   - Đồng bộ danh sách: `{"cmd":"sync"}`

---

## 📱 Cách sử dụng
1. Gõ `/web` trong Telegram Bot để lấy link truy cập Dashboard.
2. Gõ `/mqtt` để xem thông tin Secret Key và Topics nếu muốn cài đặt app MQTT bên thứ ba.
3. Nhấn vào link, Dashboard sẽ tự động nhận diện thiết bị của bạn.
4. Thêm máy tính bằng cách nhập **Tên** và **Địa chỉ MAC**.
5. Tận hưởng việc bật máy tính từ xa chỉ với một chạm!

## 📄 Giấy phép
Dự án được phát hành dưới giấy phép MIT.
