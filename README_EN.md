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
- **Chat Commands:** Supports `/list`, `/add Name MAC`, `/delete Name`, `/status`, `/mqtt`, and `/web` commands.
- **Instant Notifications:** Sends a confirmation message every time a wake command is successfully executed.
- **Magic Link:** Generates an auto-login link for the Web Dashboard with your Secret Key embedded.

### 4. Security & Reliability
- **SSL/TLS Encryption:** All communications via Port 8883 (MQTT) and HTTPS are fully encrypted.
- **Secret Key Authentication:** Only devices with a matching secret key can communicate with the ESP32.
- **Self-Healing System:** 
  - **Watchdog Timer (WDT):** Automatically reboots the ESP32 if it hangs or loses connection.
  - **Auto-Reconnect:** Automatically recovers WiFi and MQTT connections after network interruptions.
- **Zero-Local-Server:** Internal Web Server is removed to eliminate LAN access vulnerabilities.

---

## 🛠️ Detailed Installation Guide (Step-by-step)

### Step 1: Prepare Telegram Bot
1. Search for `@BotFather` on Telegram and type `/newbot` to create a new bot.
2. Save the **API Token** provided by BotFather.
3. Search for `@userinfobot` to get your personal **Chat ID**.

### Step 2: Configure Source Code (ESP32)
1. Download the project source code to your computer.
2. Navigate to the `include/` directory, copy `config.h.example` to `config.h`.
3. Open `config.h` and fill in the details:
   - `ssid` / `password`: Your local WiFi credentials.
   - `bot_token`: The API token from BotFather.
   - `chat_id`: Your ID from userinfobot.
   - `secret_key`: Any secret string of your choice (for web login).
   - `gh_pages_url`: Your GitHub Pages URL (see Step 4).

### Step 3: Flash Firmware
1. Open the project in **VS Code** with the **PlatformIO** plugin installed.
2. Connect your ESP32-C3 to your computer via USB.
3. Click the arrow icon (→) **Upload** on the PlatformIO toolbar to flash the code.

### Step 4: Deploy Web Dashboard to GitHub Pages
> **💡 Note:** If you don't want to create your own GitHub Pages site, you can use the project's official dashboard at: `https://nghiapdpers.github.io/esp32-c3-wol/`. Just enter your Secret Key in the Settings modal to start using it.
> **⚠️ Warning:** When using the official dashboard, you **MUST NOT CHANGE** the `esp32_c3_wol` topic prefix in the source code; otherwise, the website will not be able to connect to your ESP32.

To deploy your own custom site:
1. Create a new Repository on your personal GitHub account.
2. Push all source code to the `main` branch.
3. Create a `gh-pages` branch and move only the 3 files from the `data` folder to the root of this branch:
   ```bash
   git checkout --orphan gh-pages
   git rm -rf .
   # Copy index.html, style.css, script.js here
   git add . && git commit -m "Deploy Web" && git push origin gh-pages
   ```
4. Go to **Settings** -> **Pages** in the Repo, select the `gh-pages` branch, and click Save. GitHub will provide a URL (this is your `gh_pages_url`).

### Step 5: Configure Target PC
1. Enable **Wake-on-LAN** in your PC's BIOS/UEFI settings.
2. In Windows, go to *Device Manager* -> *Network Adapters* -> Your network card -> *Properties* -> *Power Management* -> Check "Allow this device to wake the computer".

---

## 📱 Usage
1. Type `/web` in your Telegram Bot to get the Dashboard link.
2. Type `/mqtt` to view your Secret Key and MQTT topics for third-party app integration.
3. Click the link; the Dashboard will automatically recognize your device.
4. Add your computers by entering a **Name** and **MAC Address**.
5. Enjoy waking your PC remotely with just one tap!

## 📄 License
MIT License.
