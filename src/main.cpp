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
        bot.sendMessage(chat_id, msg, "Markdown");
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
    preferences.end();
    
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
            bot.sendMessage(chat_id, "🖥 *WOL Manager*\n\n/list : Hiện danh sách nút\n/web : Link điều khiển từ xa\n/status : Trạng thái ESP32", "Markdown");
        } 
        else if (text == "/web") {
            String webLink = String(gh_pages_url) + "?key=" + String(secret_key);
            bot.sendMessage(chat_id, "🌐 *Remote Dashboard*\n\nNhấp vào link để mở (tự động nhập Key):\n\n" + webLink, "Markdown");
        }
        else if (text == "/status") {
            String stats = "ℹ️ *System Status*\n\n⏱ Uptime: `" + getUptime() + "`\n📶 WiFi: `" + String(WiFi.RSSI()) + " dBm`\n🔑 Key: `" + String(secret_key) + "`";
            bot.sendMessage(chat_id, stats, "Markdown");
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

    // Tạo topic bảo mật
    topicCmd = "nghiapd_wol/" + String(secret_key) + "/cmd";
    topicRes = "nghiapd_wol/" + String(secret_key) + "/res";

    // Cấu hình SSL
    secured_mqtt_client.setInsecure();
    secured_bot_client.setInsecure();

    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(2048); // Tăng buffer cho JSON & SSL
    
    WOL.setRepeat(3, 100);
    WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());

    esp_task_wdt_init(WDT_TIMEOUT, true); 
    esp_task_wdt_add(NULL); 
}

void loop() {
    esp_task_wdt_reset(); 

    if (!mqttClient.connected()) {
        String clientId = "ESP32C3-WOL-" + String(random(0xffff), HEX);
        if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
            mqttClient.subscribe(topicCmd.c_str());
            publishDeviceList();
        }
    }
    mqttClient.loop();

    if (millis() > lastTimeBotRan + bot_mtbs) {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while (numNewMessages) {
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
}
