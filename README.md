# 🚀 ESP32-C3 Wake-on-LAN (WoL) - Remote Dashboard

Hệ thống điều khiển máy tính từ xa (Wake-on-LAN) mạnh mẽ, bảo mật và chuyên nghiệp dành cho ESP32-C3. Dự án cho phép bạn quản lý và đánh thức nhiều máy tính từ bất cứ đâu thông qua Web, Telegram và MQTT.

## 🔥 Danh sách chức năng chi tiết

### 1. Điều khiển & Quản lý máy tính
- **Wake-on-LAN từ xa:** Kích hoạt gói tin Magic Packet để bật máy tính qua Internet.
- **Shutdown-on-LAN (Mới):** Tắt máy tính từ xa thông qua lệnh MQTT (Yêu cầu cài đặt PC Agent).
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
3. Mở file `config.h` và điền các thông tin. Bạn có thể chọn bỏ qua các tính năng không cần thiết:
   - **Cơ bản (Bắt buộc):**
     - `ssid` / `password`: Thông tin WiFi để ESP32 kết nối mạng.
   - **Tính năng Telegram (Tùy chọn):**
     - `bot_token` / `chat_id`: Để điều khiển qua Telegram. Nếu không dùng, hãy bỏ trống (`""`).
   - **Tính năng Web & MQTT (Tùy chọn):**
     - `mqtt_server` / `mqtt_port`: Broker MQTT (mặc định dùng EMQX). Nếu không dùng Web/MQTT, hãy bỏ trống server.
     - `secret_key`: Dùng để bảo mật và định danh thiết bị của bạn trên Web/MQTT.
   - **Thông tin bổ sung:**
     - `gh_pages_url`: URL trang Web Dashboard của bạn (dùng để Bot Telegram tạo link truy cập nhanh).

### Bước 3: Nạp Firmware
1. Mở dự án bằng **VS Code** có cài sẵn plugin **PlatformIO**.
2. Kết nối ESP32-C3 vào máy tính qua cổng USB.
3. Nhấn biểu tượng mũi tên (→) **Upload** trên thanh công cụ của PlatformIO để nạp code.

### Bước 4: Triển khai Web Dashboard (GitHub Pages)
> **💡 Lưu ý:** Nếu bạn không muốn tự tạo trang riêng, có thể sử dụng giao diện mặc định tại: `https://nghiapdpers.github.io/esp32-c3-wol/`. Bạn chỉ cần nhập đúng **Secret Key** của mình trong phần cài đặt trên web.
> **⚠️ Cảnh báo:** Khi sử dụng Dashboard mặc định, bạn **KHÔNG ĐƯỢC THAY ĐỔI** tiền tố topic `esp32_c3_wol` trong mã nguồn, nếu không trang web sẽ không thể kết nối tới ESP32 của bạn.

Để tự triển khai Dashboard trên GitHub cá nhân:
1. Đảm bảo Repository của bạn đã có nhánh `gh-pages` (nhánh này chứa mã nguồn của trang web, tách biệt với code ESP32 ở nhánh `main`).
2. Vào **Settings** -> **Pages** của Repository.
3. Tại phần **Build and deployment** -> **Branch**, chọn nhánh `gh-pages` và thư mục `/ (root)`, sau đó nhấn **Save**.
4. GitHub sẽ cung cấp một URL sau vài phút (ví dụ: `https://your-user.github.io/your-repo/`). Hãy sao chép URL này vào biến `gh_pages_url` trong `config.h`.

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
5. **Cấu trúc lệnh (Payload):**
   - **Cách 1 (Đơn giản nhất):** Chỉ cần gửi chuỗi địa chỉ MAC (VD: `00:1A:2B:3C:4D:5E`). ESP32 sẽ tự động nhận diện và gửi lệnh WOL.
   - **Cách 2 (Đầy đủ - JSON):** Dùng cho các ứng dụng cần quản lý chuyên sâu:
     - Đánh thức: `{"cmd":"wol", "mac":"00:1A:...", "name":"PC-Name"}`
     - Tắt máy: `{"cmd":"shutdown", "mac":"00:1A:...", "name":"PC-Name"}`
     - Đồng bộ danh sách: `{"cmd":"sync"}`

### Bước 7: Cài đặt PC Agent (Để dùng tính năng Shutdown)
Để có thể tắt máy từ xa, máy tính mục tiêu cần chạy một phần mềm nhỏ (Agent) để lắng nghe lệnh từ ESP32.

1. **Cấu hình:** Agent có 3 cách để nhận thông tin (Server, Port, Secret Key):
   - **Tự động:** Khi build trong project, Agent sẽ tự đọc file `include/config.h`.
   - **File JSON:** Sao chép file `agent_config.json.example` thành `agent_config.json` nằm cùng thư mục với file chạy và sửa nội dung:
     ```json
     {"mqtt_server": "broker.emqx.io", "mqtt_port": 8883, "secret_key": "your_key"}
     ```
   - **Tham số dòng lệnh:** Chạy agent với `--mac AA:BB:CC...` để ghi đè địa chỉ MAC nếu máy có nhiều card mạng.

2. **Cách lấy file PC Agent:**
   - **Tải bản build sẵn (Nhanh nhất):** Truy cập mục [**Releases**](https://github.com/nghiapdpers/esp32-c3-wol/releases) của Repository này để tải về bản mới nhất cho Windows (`.exe`) hoặc Linux.
   - **Tự build thủ công:**
     - Yêu cầu: Đã cài đặt Python 3.10+ và PyInstaller (`pip install pyinstaller`).
     - Vào thư mục `pc-agent/`.
     - Windows: Chạy file `build_exe.bat`.
     - Linux: Chạy lệnh `bash build_linux.sh`.
     - Sau khi chạy, file kết quả sẽ nằm trong thư mục `dist/`.

3. **Triển khai:** 
   - Copy file `pc_agent_windows.exe` (hoặc `pc_agent_linux`) sang máy tính mục tiêu và chạy.
   - *Lưu ý:* Agent hỗ trợ **SSL/TLS** tự động nếu bạn sử dụng port `8883`.
   - *Mẹo nâng cao:* Để có thể tắt máy ngay cả khi **chưa đăng nhập (Lock Screen)**, bạn nên cài đặt Agent dưới dạng **Windows Service** bằng công cụ [SimpleServiceManager (SSM)](https://github.com/koleys/SimpleServiceManager).
     - **Cấu hình:** Sửa file `appsettings.json` đi kèm SSM:
       ```json
       {
         "Configs": {
           "AppPath": "C:\\path\\to\\pc_agent_windows.exe",
           "AppParams": "--mac AA:BB:CC:DD:EE:FF",
           "RestartAppAutomatically": true,
           "RestartDelay": 5000
         }
       }
       ```
     - **Cài đặt:** Mở CMD với quyền Admin và chạy: `sc create PCAgent start= auto binPath= "C:\path\to\SimpleServiceManager.exe"`
     - **Lưu ý:** Nếu máy tính có nhiều card mạng (Wifi, LAN, Virtual), bạn **bắt buộc** phải điền tham số `--mac` vào `AppParams` để Agent định danh đúng thiết bị cần tắt.

---

## 📱 Cách sử dụng

Dự án hỗ trợ 3 phương thức điều khiển song song:

### 1. Web Dashboard (Tiện lợi nhất)
- Mở URL GitHub Pages của bạn.
- Lần đầu sử dụng, vào phần **Settings** trên web và nhập **Secret Key** đã cấu hình trong ESP32.
- Dashboard sẽ tự động kết nối qua MQTT (SSL) để lấy danh sách máy và thực hiện lệnh bật máy.
- **Mẹo:** Dùng lệnh `/web` trên Telegram để lấy "Magic Link" - tự động đăng nhập không cần nhập key.

### 2. Telegram Bot (Nhanh chóng)
- Gõ `/list` để hiển thị danh sách máy tính. Mỗi máy sẽ có 2 lựa chọn:
  - 🚀 **Tên máy:** Bật máy (WOL).
  - 🛑 **Off:** Tắt máy (Shutdown - Yêu cầu Agent).
- Dùng `/status` để kiểm tra tình trạng kết nối của ESP32.
- Thêm máy mới trực tiếp bằng lệnh: `/add Tên_Máy MAC_Address`.

### 3. App MQTT bên thứ ba
- Nếu bạn thích dùng các app như *MQTT Dash*, hãy cấu hình:
  - **Topic lệnh:** `esp32_c3_wol/YOUR_SECRET_KEY/cmd`
  - **Payload:** Gửi địa chỉ MAC của máy (ví dụ: `AA:BB:CC:DD:EE:FF`).

## 📄 Giấy phép
Dự án được phát hành dưới giấy phép MIT.
