# 🚀 ESP32-C3 Wake-on-LAN (WoL) - Remote Dashboard

A powerful, secure, and aesthetic Remote Wake-on-LAN system designed for the ESP32-C3. Control your PC from anywhere via **Telegram Bot** and **Web Dashboard (GitHub Pages)** without port forwarding.

## ✨ Key Features

- 🌍 **Global Remote Access:** No Port Forwarding required, works behind NAT/CGNAT.
- 🛡️ **Top-Tier Security:** 
  - Full **SSL/TLS (Port 8883)** encryption for MQTT communication.
  - Authentication via **Secret Key** and private topic derivation.
  - No local web server running (Zero local attack surface).
- 🤖 **Telegram Bot:** Get notifications, check system status, and trigger WoL via interactive buttons.
- 🎨 **Premium Web UI:** Modern Glassmorphism design, ready for GitHub Pages hosting.
- 🔗 **Magic Link:** Instant dashboard login from Telegram with one click.

## 🛠️ How It Works

1. **Frontend:** A static web page hosted on GitHub Pages connects to a public MQTT Broker via Secure WebSockets (WSS).
2. **Bridge:** The MQTT Broker (EMQX) acts as a secure bridge between the Web/Bot and the ESP32.
3. **Hardware:** The ESP32-C3 listens for commands via a secure SSL connection and broadcasts Magic Packets to your local network.

## ⚙️ Setup

### 1. Hardware Requirements
- ESP32-C3 (e.g., Super Mini).
- USB-C cable.

### 2. Software Configuration
1. Rename `include/config.h.example` to `include/config.h`.
2. Update the following parameters:
   - `ssid` / `password`: Your local WiFi credentials.
   - `bot_token` / `chat_id`: Telegram Bot credentials from @BotFather.
   - `secret_key`: Your personal secret token (for Web authentication).
   - `gh_pages_url`: Your GitHub Pages site URL.

### 3. Flashing
- Use **PlatformIO** in VS Code.
- Run `Upload` to flash the firmware.
- (No need to run `Upload Filesystem Image` as we use GitHub Pages for the UI).

### 4. Hosting the Dashboard
1. Create a GitHub Repo and upload the files from the `data/` directory.
2. Enable **GitHub Pages** in the Repo Settings.

## 📱 Usage

1. Open your Telegram Bot.
2. Type `/start` for a list of commands.
3. Type `/web` to receive your **Magic Link**. Click it to open the Dashboard with auto-login.
4. On the Dashboard, click **(+)** to add a new device (Name and MAC Address).
5. Hit **Wake** to wake your PC from anywhere in the world!

## 📄 License
This project is licensed under the MIT License.
