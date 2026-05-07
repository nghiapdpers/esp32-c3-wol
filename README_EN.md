# 🚀 ESP32-C3 Wake-on-LAN (WoL) - Remote Dashboard

A powerful, secure, and professional Remote Wake-on-LAN system for the ESP32-C3. Manage and wake multiple computers from anywhere via Web, Telegram, and MQTT.

## 🔥 Detailed Functionalities

### 1. Computer Control & Management
- **Remote WoL:** Trigger Magic Packets to wake your PC via the Internet.
- **Device List Management:** Add, edit, or delete computers directly from the Web interface.
- **Persistent Storage:** Device lists are stored in the ESP32's Flash memory (Preferences), ensuring data is kept after power cycles.

### 2. Web Dashboard (GitHub Pages)
- **Glassmorphism UI:** Modern, smooth, and fully mobile-responsive interface.
- **Real-time Sync:** The web app automatically fetches the device list from the ESP32 upon opening.
- **System Metrics:** Displays Uptime, WiFi signal strength (RSSI), and free RAM on the ESP32.

### 3. Telegram Bot Integration
- **Smart Button Menu:** Displays the device list as interactive buttons for quick control.
- **Chat Commands:** Supports `/list`, `/status`, and `/web` commands.
- **Instant Notifications:** Sends a confirmation message every time a wake command is successfully executed.
- **Magic Link:** Generates an auto-login link for the Web Dashboard with your Secret Key embedded.

### 4. Security & Reliability
- **SSL/TLS Encryption:** All communications via Port 8883 (MQTT) and HTTPS are fully encrypted.
- **Secret Key Authentication:** Only devices with a matching secret key can communicate with the ESP32.
- **Self-Healing System:** 
  - **Watchdog Timer (WDT):** Automatically reboots the ESP32 if it hangs or loses connection.
  - **Auto-Reconnect:** Automatically recovers WiFi and MQTT connections after network interruptions.
- **Zero-Local-Server:** Internal Web Server is removed to eliminate LAN access vulnerabilities.

### 5. Hardware Feedback
- **LED Status:** Onboard LED provides visual feedback for connection status and flashes when a packet is sent successfully/unsuccessfully.

## 🛠️ How It Works
- **Protocol:** MQTT over SSL (Port 8883).
- **Topic Structure:** `esp32_c3_wol/[secret_key]/cmd` and `esp32_c3_wol/[secret_key]/res`.

## ⚙️ Setup & Deployment
1. Configure `include/config.h` (from the `.example` file).
2. Flash the code to the ESP32-C3 using PlatformIO.
3. Push the `data` folder contents to the `gh-pages` branch on your GitHub.

## 📄 License
MIT License.
