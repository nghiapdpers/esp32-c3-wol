# 🚀 ESP32-C3 Wake-on-LAN (WoL) - Remote Dashboard

A powerful, secure, and professional Remote Wake-on-LAN system for the ESP32-C3. Manage and wake multiple computers from anywhere via Web, Telegram, and MQTT.

## 🔥 Detailed Functionalities

### 1. Computer Control & Management
- **Remote WoL:** Trigger Magic Packets to wake your PC via the Internet.
- **Remote Shutdown (New):** Safely power off your PC via MQTT commands (Requires PC Agent).
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

### 5. Flexible Configuration (Modular Features)
The project is designed so you can choose to use 1, 2, or all 3 control methods based on your needs:
- **Telegram Only:** Leave `mqtt_server` empty in `config.h`.
- **MQTT Only:** Leave `bot_token` and `chat_id` empty.
- **No Web Dashboard:** Leave `secret_key` empty. In this case, MQTT topics will use the `default` keyword by default (See Step 6).
The ESP32 will automatically detect and only run the features that have been configured, saving resources and ensuring stability.

---

## 🛠️ Detailed Installation Guide (Step-by-step)

### Step 1: Prepare Telegram Bot
1. Search for `@BotFather` on Telegram and type `/newbot` to create a new bot.
2. Save the **API Token** provided by BotFather.
3. Search for `@userinfobot` to get your personal **Chat ID**.

### Step 2: Configure Source Code (ESP32)
1. Download the project source code to your computer.
2. Navigate to the `include/` directory, copy `config.h.example` to `config.h`.
3. Open `config.h` and fill in the details. You can choose to skip features you don't need:
   - **Core (Required):**
     - `ssid` / `password`: Your local WiFi credentials.
   - **Telegram Features (Optional):**
     - `bot_token` / `chat_id`: For control via Telegram. Leave empty (`""`) if not using.
   - **Web & MQTT Features (Optional):**
     - `mqtt_server` / `mqtt_port`: MQTT Broker info. Leave server empty if not using Web/MQTT.
     - `secret_key`: Used for topic security and web identification.
   - **Additional Info:**
     - `gh_pages_url`: Your GitHub Pages URL (used by the Telegram bot for quick links).

### Step 3: Flash Firmware
1. Open the project in **VS Code** with the **PlatformIO** plugin installed.
2. Connect your ESP32-C3 to your computer via USB.
3. Click the arrow icon (→) **Upload** on the PlatformIO toolbar to flash the code.

### Step 4: Deploy Web Dashboard to GitHub Pages
> **💡 Note:** If you don't want to create your own site, use the official one at: `https://nghiapdpers.github.io/esp32-c3-wol/`. Just enter your **Secret Key** in the settings modal.

To host your own Dashboard on personal GitHub:
1. Ensure your repository has the `gh-pages` branch (this branch contains the web source code).
2. Go to **Settings** -> **Pages** in the Repository.
3. In **Build and deployment** -> **Branch**, select `gh-pages` and `/ (root)`, then click **Save**.
4. GitHub will provide a URL after a few minutes (e.g., `https://your-user.github.io/your-repo/`). Copy this to `gh_pages_url` in `config.h`.

### Step 5: Configure Target PC
1. Enable **Wake-on-LAN** in your PC's BIOS/UEFI settings (usually under Power Management).
2. In Windows, go to *Device Manager* -> *Network Adapters* -> Your network card -> *Properties* -> *Advanced* -> Enable "Magic Packet" and "Wake on Magic Packet".
3. Go to the *Power Management* tab -> Check "Allow this device to wake the computer".

### Step 6: MQTT App Configuration (Optional)
If you want to use third-party apps like *MQTT Dash* or *MQTT Panel*:
1. **Broker:** `broker.emqx.io` (or your chosen broker).
2. **Port:** `8883` (select SSL/TLS protocol).
3. **Command Topic:** `esp32_c3_wol/YOUR_SECRET_KEY/cmd` (If no key, use: `esp32_c3_wol/default/cmd`)
4. **Response Topic:** `esp32_c3_wol/YOUR_SECRET_KEY/res` (If no key, use: `esp32_c3_wol/default/res`)
5. **Command Structure (Payload):**
   - **Method 1 (Simplest):** Just publish the plain MAC address string (e.g., `00:1A:2B:3C:4D:5E`). The ESP32 will automatically detect it and trigger WOL.
   - **Method 2 (Advanced - JSON):** Used for full management features:
      - Wake Up: `{"cmd":"wol", "mac":"00:1A:...", "name":"PC-Name"}`
      - Shutdown: `{"cmd":"shutdown", "mac":"00:1A:...", "name":"PC-Name"}`
      - Sync List: `{"cmd":"sync"}`

### Step 7: Install PC Agent (For Shutdown Feature)
To shut down your computer remotely, the target PC must run a lightweight background script (Agent) to listen for commands from the ESP32.

1. **Configuration:** The Agent has 3 ways to get settings (Server, Port, Secret Key):
   - **Automatic:** When built within the project, it automatically reads `include/config.h`.
   - **JSON Config:** Copy `agent_config.json.example` to `agent_config.json` in the same directory as the executable:
     ```json
     {"mqtt_server": "broker.emqx.io", "mqtt_port": 8883, "secret_key": "your_key"}
     ```
   - **CLI Arguments:** Run the agent with `--mac AA:BB:CC...` to override the MAC address if you have multiple network adapters.

2. **How to get the PC Agent:**
   - **Download Pre-built Binaries (Fastest):** Go to the [**Releases**](https://github.com/nghiapdpers/esp32-c3-wol/releases) section of this Repository to download the latest version for Windows (`.exe`) or Linux.
   - **Manual Build:**
     - Requirement: Python 3.10+ and install libraries: `pip install -r pc-agent/requirements.txt pyinstaller`.
     - Windows: Run `pc-agent/build_exe.bat`.
     - Linux: Run `bash pc-agent/build_linux.sh`.
     - After building, the resulting files will be located in the `dist/` directory.

3. **Deployment:** 
   - Copy `pc_agent_windows.exe` (or `pc_agent_linux`) to the target PC and run it.
   - *Note:* The Agent automatically enables **SSL/TLS** if you use port `8883`.
   - *Advanced Tip:* To allow shutdown even from the **Lock Screen**, install the agent as a **Windows Service** using [SimpleServiceManager (SSM)](https://github.com/koleys/SimpleServiceManager).
     - **Configuration:** Edit the `appsettings.json` file bundled with SSM:
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
     - **Installation:** Open CMD as Administrator and run: `sc create PCAgent start= auto binPath= "C:\path\to\SimpleServiceManager.exe"`
     - **Note:** If the computer has multiple network adapters (WiFi, LAN, Virtual), you **must** specify the `--mac` parameter in `AppParams` so the Agent can correctly identify the device to shut down.

---

## 📱 Usage

The project supports 3 simultaneous control methods:

### 1. Web Dashboard (Recommended)
- Open your GitHub Pages URL.
- On first use, go to **Settings** and enter your **Secret Key**.
- The dashboard will automatically sync and allow you to "Wake" devices with one click.
- **Tip:** Use the `/web` command on Telegram to get a "Magic Link" for auto-login.

### 2. Telegram Bot (Quick Access)
- Type `/list` to see your computer list as interactive buttons. Each device has two options:
  - 🚀 **PC Name:** Trigger Wake-on-LAN.
  - 🛑 **Off:** Trigger Remote Shutdown.
- Use `/status` to check the ESP32 connection state.
- Add new PCs remotely using: `/add PC_Name MAC_Address`.

### 3. Third-party MQTT App
- Configure apps like *MQTT Dash*:
  - **Command Topic:** `esp32_c3_wol/YOUR_SECRET_KEY/cmd`
  - **Payload:** Send the MAC address (e.g., `AA:BB:CC:DD:EE:FF`).

## 📄 License
MIT License.
