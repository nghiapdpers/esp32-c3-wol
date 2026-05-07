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
#include "config.h"

// MQTT SSL & WoL
WiFiClientSecure secured_mqtt_client; // Dùng SSL cho MQTT
PubSubClient mqttClient(secured_mqtt_client);
WiFiUDP udp;
WakeOnLan WOL(udp);

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

void executeWoL(String mac, String source, String pcName = "") {
    String displayName = (pcName != "") ? pcName : mac;
    if (mac.length() >= 17) {
        WOL.sendMagicPacket(mac.c_str());
        String msg = "🚀 *WoL Triggered*\n🖥 Device: `" + displayName + "`\n📡 Source: `" + source + "`\n✅ Status: Magic Packet Sent";
        if (enableTelegram) bot.sendMessage(chat_id, msg, "Markdown");
        blinkSuccess();
    } else {
        blinkError();
    }
}

void savePC(String name, String mac) {
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
    DynamicJsonDocument doc(2048);
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
    if (!enableMQTT) {
        preferences.end();
        return;
    }

    doc["uptime"] = getUptime();
    doc["rssi"] = WiFi.RSSI();
    doc["heap"] = ESP.getFreeHeap() / 1024;

    String response;
    serializeJson(doc, response);
    mqttClient.publish(topicRes.c_str(), response.c_str());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) return;

    String cmd = doc["cmd"].as<String>();
    if (cmd == "sync") publishDeviceList();
    else if (cmd == "wol") executeWoL(doc["mac"].as<String>(), "Web-Remote", doc["name"].as<String>());
    else if (cmd == "add") { savePC(doc["name"].as<String>(), doc["mac"].as<String>()); publishDeviceList(); }
    else if (cmd == "delete") { deletePC(doc["name"].as<String>()); publishDeviceList(); }
}

void sendPCListMenu() {
    preferences.begin("wol", true);
    String index = preferences.getString("index", "");
    preferences.end();
    if (index == "") { bot.sendMessage(chat_id, "⚠️ Danh sách máy trống!", ""); return; }

    String keyboardJson = "[";
    int start = 0; int end = index.indexOf('|'); bool first = true;
    while (end != -1) {
        String pcName = index.substring(start, end);
        if (!first) keyboardJson += ",";
        keyboardJson += "[{\"text\":\"🖥 " + pcName + "\", \"callback_data\":\"" + pcName + "\"}]";
        start = end + 1; end = index.indexOf('|', start); first = false;
    }
    keyboardJson += "]";
    bot.sendMessageWithInlineKeyboard(chat_id, "Chọn máy tính:", "Markdown", keyboardJson);
}

void handleNewMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id_incoming = String(bot.messages[i].chat_id);
        if (chat_id_incoming != chat_id) continue;
        String text = bot.messages[i].text;
        
        if (text == "/start" || text == "/help") {
            String welcome = "🖥 *WOL Manager*\n\n/list : Hiện danh sách nút\n/add Name MAC : Thêm máy\n/delete Name : Xóa máy\n/status : Trạng thái hệ thống";
            if (enableWeb) welcome += "\n/web : Link điều khiển từ xa";
            if (enableMQTT) welcome += "\n/mqtt : Cấu hình MQTT";
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
            String stats = "ℹ️ *System Status*\n\n⏱ Uptime: `" + getUptime() + "`\n📶 WiFi: `" + String(WiFi.RSSI()) + " dBm`";
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

    // Cấu hình SSL
    if (enableMQTT) secured_mqtt_client.setInsecure();
    if (enableTelegram) secured_bot_client.setInsecure();

    if (enableMQTT) {
        mqttClient.setServer(mqtt_server, mqtt_port);
        mqttClient.setCallback(mqttCallback);
        mqttClient.setBufferSize(2048); 
    }
    
    WOL.setRepeat(3, 100);
    WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());

    esp_task_wdt_init(WDT_TIMEOUT, true); 
    esp_task_wdt_add(NULL); 
}

void loop() {
    esp_task_wdt_reset(); 

    if (enableMQTT) {
        if (!mqttClient.connected()) {
            String clientId = "ESP32C3-WOL-" + String(random(0xffff), HEX);
            if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
                mqttClient.subscribe(topicCmd.c_str());
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
}
