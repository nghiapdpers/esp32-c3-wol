#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <WakeOnLan.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include <Preferences.h>
#include <map>
#include "config.h"

// MQTT SSL & WoL
WiFiClient wifi_client;
WiFiClientSecure secured_mqtt_client;
PubSubClient mqttClient; 
WiFiUDP udp;
WakeOnLan WOL(udp);

// Trạng thái Agent (Lưu thời điểm cuối cùng nhận tin nhắn từ Agent)
std::map<String, unsigned long> lastSeen;
String topicStatusPrefix = "";

bool isDeviceOnline(String nMac) {
    if (lastSeen.count(nMac) == 0) return false;
    // Timeout sau 90 giây (gấp 3 lần chu kỳ heartbeat 30s của Agent)
    return (millis() - lastSeen[nMac]) < 90000;
}

// Telegram
WiFiClientSecure secured_bot_client; // Dùng SSL cho Bot
UniversalTelegramBot bot(bot_token, secured_bot_client);
unsigned long lastTimeBotRan;

// Feature Toggles
bool enableMQTT = false;
bool enableTelegram = false;
bool enableWeb = false;

// Storage
Preferences preferences;

// Topics động dựa trên Secret Key
String topicCmd = "";
String topicRes = "";

#define RESTART_INTERVAL 604800000 
#define WDT_TIMEOUT 30 

void ledOn() { digitalWrite(LED_PIN, LOW); }
void ledOff() { digitalWrite(LED_PIN, HIGH); }
void blinkSuccess() { ledOff(); delay(200); ledOn(); }
void blinkError() { for (int i = 0; i < 10; i++) { ledOff(); delay(50); ledOn(); delay(50); } }

String getUptime() {
    unsigned long sec = millis() / 1000;
    int days = sec / 86400;
    int hours = (sec % 86400) / 3600;
    int mins = (sec % 3600) / 60;
    return String(days) + "d " + String(hours) + "h " + String(mins) + "m";
}

bool isValidMAC(String mac) {
    if (mac.length() != 17) return false;
    for (int i = 0; i < 17; i++) {
        if (i % 3 == 2) {
            if (mac[i] != ':' && mac[i] != '-' && mac[i] != '.') return false;
        } else {
            if (!isxdigit(mac[i])) return false;
        }
    }
    return true;
}

String normalizeMac(String mac) {
    mac.replace(":", "");
    mac.replace("-", "");
    mac.replace(".", "");
    mac.toUpperCase();
    mac.trim();
    return mac;
}

void executeWoL(String mac, String source, String pcName = "") {
    String displayName = (pcName != "") ? pcName : mac;
    if (isValidMAC(mac)) {
        WOL.sendMagicPacket(mac.c_str());
        String msg = "🚀 *WoL Triggered*\n🖥 Device: `" + displayName + "`\n📡 Source: `" + source + "`\n✅ Status: Magic Packet Sent";
        if (enableTelegram) bot.sendMessage(chat_id, msg, "Markdown");
        blinkSuccess();
    } else {
        if (enableTelegram) bot.sendMessage(chat_id, "❌ Lỗi: Địa chỉ MAC không hợp lệ: `" + mac + "`", "Markdown");
        blinkError();
    }
}

void executeShutdown(String mac, String source, String pcName = "", bool publishMQTT = true) {
    String displayName = (pcName != "") ? pcName : mac;
    if (isValidMAC(mac)) {
        if (enableMQTT) {
            if (publishMQTT) {
                DynamicJsonDocument doc(256);
                doc["cmd"] = "shutdown";
                doc["mac"] = mac;
                doc["name"] = pcName;
                doc["from"] = "esp32"; // Đánh dấu nguồn gửi từ ESP32
                String payload;
                serializeJson(doc, payload);
                mqttClient.publish(topicCmd.c_str(), payload.c_str());
            }
            
            String status = publishMQTT ? "Sent via MQTT" : "Received via MQTT";
            String msg = "🛑 *Shutdown Command*\n🖥 Device: `" + displayName + "`\n📡 Source: `" + source + "`\n✅ Status: " + status;
            if (enableTelegram) bot.sendMessage(chat_id, msg, "Markdown");
            blinkSuccess();
        } else {
            if (enableTelegram) bot.sendMessage(chat_id, "⚠️ MQTT chưa được cấu hình để gửi lệnh Shutdown!", "");
            blinkError();
        }
    } else {
        if (enableTelegram) bot.sendMessage(chat_id, "❌ Lỗi: Địa chỉ MAC không hợp lệ: `" + mac + "`", "Markdown");
        blinkError();
    }
}

void savePC(String name, String mac) {
    name.replace("|", ""); // Xóa ký tự phân tách để tránh lỗi parse
    mac.toUpperCase();
    mac.trim();
    if (!isValidMAC(mac)) return;

    preferences.begin("wol", false);
    preferences.putString(name.c_str(), mac);
    String index = preferences.getString("index", "");
    if (index.indexOf(name + "|") == -1) {
        index += name + "|";
        preferences.putString("index", index);
    }
    preferences.end();
}

void deletePC(String name) {
    preferences.begin("wol", false);
    preferences.remove(name.c_str());
    String index = preferences.getString("index", "");
    index.replace(name + "|", "");
    preferences.putString("index", index);
    preferences.end();
}

void publishDeviceList() {
    preferences.begin("wol", true);
    String index = preferences.getString("index", "");
    DynamicJsonDocument doc(4096);
    doc["type"] = "list";
    JsonArray array = doc.createNestedArray("devices");
    
    int start = 0;
    int end = index.indexOf('|');
    while (end != -1) {
        String name = index.substring(start, end);
        String mac = preferences.getString(name.c_str(), "");
        JsonObject obj = array.createNestedObject();
        obj["name"] = name;
        obj["mac"] = mac;
        start = end + 1;
        end = index.indexOf('|', start);
    }
    preferences.end();

    if (!enableMQTT) return;

    doc["uptime"] = getUptime();
    doc["rssi"] = WiFi.RSSI();
    doc["heap"] = ESP.getFreeHeap() / 1024;

    String response;
    serializeJson(doc, response);
    mqttClient.publish(topicRes.c_str(), response.c_str());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String topicStr = String(topic);
    String message = "";
    for (int i = 0; i < length; i++) message += (char)payload[i];
    message.trim();

    // Xử lý status từ Agent
    if (topicStr.startsWith(topicStatusPrefix)) {
        String mac = topicStr.substring(topicStatusPrefix.length());
        String nMac = normalizeMac(mac); 
        if (message == "online") {
            lastSeen[nMac] = millis();
        } else {
            lastSeen[nMac] = 0; // Đánh dấu Offline ngay lập tức
        }
        return;
    }

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload, length);

    if (!error) {
        String cmd = doc["cmd"].as<String>();
        String from = doc["from"].as<String>();

        // Bỏ qua nếu lệnh này do chính ESP32 gửi đi (tránh vòng lặp phản hồi)
        if (from == "esp32") return;
        
        if (cmd == "sync") publishDeviceList();
        else if (cmd == "wol") executeWoL(doc["mac"].as<String>(), "MQTT-App", doc["name"].as<String>());
        else if (cmd == "shutdown") { 
            executeShutdown(doc["mac"].as<String>(), "Web-Dashboard", doc["name"].as<String>(), false);
        }
        else if (cmd == "add") { savePC(doc["name"].as<String>(), doc["mac"].as<String>()); publishDeviceList(); }
        else if (cmd == "delete") { deletePC(doc["name"].as<String>()); publishDeviceList(); }
    } else {
        if (message.length() >= 17) {
            executeWoL(message, "MQTT-Plain-MAC", "Unknown PC");
        }
    }
}

void sendPCListMenu() {
    preferences.begin("wol", true);
    String index = preferences.getString("index", "");
    if (index == "") { preferences.end(); bot.sendMessage(chat_id, "⚠️ Danh sách máy trống!", ""); return; }

    String keyboardJson = "[";
    int start = 0; int end = index.indexOf('|'); bool first = true;
    while (end != -1) {
        String pcName = index.substring(start, end);
        String mac = preferences.getString(pcName.c_str(), "");
        
        // Kiểm tra trạng thái online bằng timestamp
        String nMac = normalizeMac(mac);
        bool isOnline = isDeviceOnline(nMac);
        String statusEmoji = isOnline ? "🟢" : "⚪";
        
        if (!first) keyboardJson += ",";
        keyboardJson += "[{\"text\":\"" + statusEmoji + " " + pcName + "\", \"callback_data\":\"wol_" + pcName + "\"}, {\"text\":\"🛑 Off\", \"callback_data\":\"off_" + pcName + "\"}]";
        start = end + 1; end = index.indexOf('|', start); first = false;
    }
    preferences.end();
    keyboardJson += "]";
    bot.sendMessageWithInlineKeyboard(chat_id, "Chọn lệnh cho máy tính (🟢 Online, ⚪ Offline):", "Markdown", keyboardJson);
}

void handleNewMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id_incoming = String(bot.messages[i].chat_id);
        if (chat_id_incoming != chat_id) continue;
        String text = bot.messages[i].text;
        String type = bot.messages[i].type;

        if (type == "callback_query") {
            String callbackData = text; // callback_data của library đưa vào text
            String pcName = "";
            if (callbackData.startsWith("wol_")) {
                pcName = callbackData.substring(4);
                bot.answerCallbackQuery(bot.messages[i].query_id, "🚀 Đang gửi lệnh WOL...");
                preferences.begin("wol", true);
                String mac = preferences.getString(pcName.c_str(), "");
                preferences.end();
                if (mac != "") executeWoL(mac, "Telegram-Bot", pcName);
            } else if (callbackData.startsWith("off_")) {
                pcName = callbackData.substring(4);
                bot.answerCallbackQuery(bot.messages[i].query_id, "🛑 Đang gửi lệnh Shutdown...");
                preferences.begin("wol", true);
                String mac = preferences.getString(pcName.c_str(), "");
                preferences.end();
                if (mac != "") executeShutdown(mac, "Telegram-Bot", pcName);
            }
            continue;
        }
        if (text == "/start" || text == "/help") {
            String welcome = "🖥 *ESP32-C3 WoL & Shutdown Manager*\n\n";
            welcome += "🚀 `/list` : Hiện danh sách máy & điều khiển\n";
            welcome += "➕ `/add Name MAC` : Thêm máy mới\n";
            welcome += "🗑 `/delete Name` : Xóa máy khỏi danh sách\n";
            welcome += "ℹ️ `/status` : Kiểm tra thông số hệ thống\n";
            
            if (enableWeb) welcome += "🌐 `/web` : Mở Dashboard điều khiển từ xa\n";
            if (enableMQTT) welcome += "📡 `/mqtt` : Xem cấu hình MQTT\n";
            
            welcome += "\n💡 *Tips:* Bạn có thể bật máy từ xa (WOL) hoặc tắt máy (Shutdown) trực tiếp từ menu `/list`.";
            bot.sendMessage(chat_id, welcome, "Markdown");
        } 
        else if (text.startsWith("/add")) {
            // Định dạng: /add PC-Name 00:1A:2B:3C:4D:5E
            int firstSpace = text.indexOf(' ');
            int lastSpace = text.lastIndexOf(' ');
            if (firstSpace != -1 && lastSpace != -1 && firstSpace != lastSpace) {
                String name = text.substring(firstSpace + 1, lastSpace);
                String mac = text.substring(lastSpace + 1);
                savePC(name, mac);
                bot.sendMessage(chat_id, "✅ Đã thêm: `" + name + "` (" + mac + ")", "Markdown");
                publishDeviceList(); // Sync qua MQTT
            } else {
                bot.sendMessage(chat_id, "❌ Sai cú pháp! Dùng: `/add Name MAC`", "Markdown");
            }
        }
        else if (text.startsWith("/delete")) {
            // Định dạng: /delete PC-Name
            int space = text.indexOf(' ');
            if (space != -1) {
                String name = text.substring(space + 1);
                deletePC(name);
                bot.sendMessage(chat_id, "🗑 Đã xóa: `" + name + "`", "Markdown");
                publishDeviceList(); // Sync qua MQTT
            } else {
                bot.sendMessage(chat_id, "❌ Sai cú pháp! Dùng: `/delete Name`", "Markdown");
            }
        }
        else if (text == "/web") {
            if (enableWeb) {
                String webLink = String(gh_pages_url) + "?key=" + String(secret_key);
                bot.sendMessage(chat_id, "🌐 *Remote Dashboard*\n\nNhấp vào link để mở (tự động nhập Key):\n\n" + webLink, "Markdown");
            } else {
                bot.sendMessage(chat_id, "⚠️ Web Dashboard chưa được cấu hình!", "");
            }
        }
        else if (text == "/status") {
            String stats = "ℹ️ *System Status*\n\n⏱ Uptime: `" + getUptime() + "`\n📶 WiFi: `" + String(WiFi.RSSI()) + " dBm`\n🧠 Free RAM: `" + String(ESP.getFreeHeap() / 1024) + " KB`";
            bot.sendMessage(chat_id, stats, "Markdown");
        }
        else if (text == "/mqtt") {
            if (enableMQTT) {
                String mqttInfo = "📡 *MQTT Configuration*\n\n";
                mqttInfo += "🔑 *Secret Key:* `" + String(secret_key) + "`\n";
                mqttInfo += "📥 *Command Topic:* `" + topicCmd + "`\n";
                mqttInfo += "📤 *Response Topic:* `" + topicRes + "`";
                bot.sendMessage(chat_id, mqttInfo, "Markdown");
            } else {
                bot.sendMessage(chat_id, "⚠️ MQTT chưa được cấu hình!", "");
            }
        }
        else if (text == "/list") sendPCListMenu();
    }
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    ledOn();

    // Kiểm tra cấu hình
    if (strlen(mqtt_server) > 0) enableMQTT = true;
    if (strlen(bot_token) > 0 && strlen(chat_id) > 0 && String(bot_token) != "YOUR_BOT_TOKEN") enableTelegram = true;
    if (strlen(secret_key) > 0) enableWeb = true;

    // Tạo topic bảo mật (Mặc định dùng "default" nếu không có secret_key)
    String key = enableWeb ? String(secret_key) : "default";
    topicCmd = "esp32_c3_wol/" + key + "/cmd";
    topicRes = "esp32_c3_wol/" + key + "/res";
    topicStatusPrefix = "esp32_c3_wol/" + key + "/status/";

    // Cấu hình MQTT Client dựa trên Port
    if (enableMQTT) {
        if (mqtt_port == 8883) {
            secured_mqtt_client.setInsecure();
            mqttClient.setClient(secured_mqtt_client);
            Serial.println("🔒 MQTT: Using Secure connection (Port 8883)");
        } else {
            mqttClient.setClient(wifi_client);
            Serial.println("🔓 MQTT: Using Non-secure connection (Port 1883)");
        }
    }
    
    if (enableTelegram) secured_bot_client.setInsecure();

    if (enableMQTT) {
        mqttClient.setServer(mqtt_server, mqtt_port);
        mqttClient.setCallback(mqttCallback);
        mqttClient.setBufferSize(4096); 
    }
    
    WOL.setRepeat(3, 100);
    WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());

    esp_task_wdt_init(WDT_TIMEOUT, true); 
    esp_task_wdt_add(NULL); 

    if (enableTelegram) {
        String startMsg = "✅ *System Online*\nESP32-C3 WoL đã khởi động thành công.\n🌐 IP: `" + WiFi.localIP().toString() + "`";
        bot.sendMessage(chat_id, startMsg, "Markdown");
    }
}

void loop() {
    esp_task_wdt_reset(); 

    if (enableMQTT) {
        if (!mqttClient.connected()) {
            String clientId = "ESP32C3-WOL-" + String(random(0xffff), HEX);
            if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
                mqttClient.subscribe(topicCmd.c_str());
                mqttClient.subscribe((topicStatusPrefix + "#").c_str());
                publishDeviceList();
            }
        }
        mqttClient.loop();
    }

    if (enableTelegram && millis() > lastTimeBotRan + bot_mtbs) {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while (numNewMessages) {
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }

    // Tự động khởi động lại định kỳ (7 ngày) để giải phóng bộ nhớ và duy trì độ ổn định
    if (millis() > RESTART_INTERVAL) {
        if (enableTelegram) bot.sendMessage(chat_id, "🔄 *Maintenance:* Hệ thống đang tự động khởi động lại định kỳ để tối ưu hiệu suất...", "Markdown");
        delay(1000);
        ESP.restart();
    }
}
