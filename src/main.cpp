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

// MQTT & WoL
WiFiClient espClient;
PubSubClient mqttClient(espClient);
WiFiUDP udp;
WakeOnLan WOL(udp);

// Telegram
WiFiClientSecure secured_client;
UniversalTelegramBot bot(bot_token, secured_client);
unsigned long lastTimeBotRan;

// Storage
Preferences preferences;

#define RESTART_INTERVAL 604800000 
#define WDT_TIMEOUT 30 

void ledOn() { digitalWrite(LED_PIN, LOW); }
void ledOff() { digitalWrite(LED_PIN, HIGH); }
void blinkSuccess() { ledOff(); delay(200); ledOn(); }
void blinkError() { for (int i = 0; i < 10; i++) { ledOff(); delay(50); ledOn(); delay(50); } }

// --- Hàm thực thi WoL ---
void executeWoL(String mac, String source, String pcName = "") {
    String displayName = (pcName != "") ? pcName : mac;
    
    if (mac.length() >= 17) {
        WOL.sendMagicPacket(mac.c_str());
        
        // Luôn báo về Telegram bất kể nguồn từ đâu
        String msg = "🚀 *WoL Triggered*\n";
        msg += "🖥 Device: `" + displayName + "`\n";
        msg += "📡 Source: `" + source + "`\n";
        msg += "✅ Status: Magic Packet Sent";
        bot.sendMessage(chat_id, msg, "Markdown");
        
        blinkSuccess();
    } else {
        String msg = "❌ *WoL Failed*\n";
        msg += "🖥 Device: `" + displayName + "`\n";
        msg += "📡 Source: `" + source + "`\n";
        msg += "⚠️ Reason: Invalid MAC Address";
        bot.sendMessage(chat_id, msg, "Markdown");
        
        blinkError();
    }
}

// --- Quản lý PC ---
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

// --- Menu Nút Bấm ---
void sendPCListMenu() {
    preferences.begin("wol", true);
    String index = preferences.getString("index", "");
    preferences.end();

    if (index == "") {
        bot.sendMessage(chat_id, "⚠️ Danh sách máy trống!", "");
        return;
    }

    String keyboardJson = "[";
    int start = 0;
    int end = index.indexOf('|');
    bool first = true;

    while (end != -1) {
        String pcName = index.substring(start, end);
        if (!first) keyboardJson += ",";
        keyboardJson += "[{\"text\":\"🖥 " + pcName + "\", \"callback_data\":\"" + pcName + "\"}]";
        start = end + 1;
        end = index.indexOf('|', start);
        first = false;
    }
    keyboardJson += "]";

    bot.sendMessageWithInlineKeyboard(chat_id, "Chọn máy tính:", "Markdown", keyboardJson);
}

// --- Callback MQTT ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String mac = "";
    for (int i = 0; i < length; i++) mac += (char)payload[i];
    if (String(topic) == topic_command) executeWoL(mac, "MQTT");
}

// --- Xử lý Telegram ---
void handleNewMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id_incoming = String(bot.messages[i].chat_id);
        if (chat_id_incoming != chat_id) continue;

        String text = bot.messages[i].text;
        
        if (bot.messages[i].type == "callback_query") {
            String pcName = bot.messages[i].text;
            preferences.begin("wol", true);
            String mac = preferences.getString(pcName.c_str(), "");
            preferences.end();

            if (mac != "") {
                bot.answerCallbackQuery(bot.messages[i].query_id, "🚀 Sending...", false);
                executeWoL(mac, "Bot Button", pcName);
            }
            continue;
        }

        if (text == "/start" || text == "/help") {
            String welcome = "🖥 *WOL Manager*\n\n/list : Danh sách máy\n/add Name MAC : Thêm\n/delete Name : Xóa";
            bot.sendMessage(chat_id, welcome, "Markdown");
        } 
        else if (text.startsWith("/add ")) {
            int firstSpace = text.indexOf(' ', 5);
            if (firstSpace > 5) {
                String name = text.substring(5, firstSpace);
                String mac = text.substring(firstSpace + 1);
                mac.trim();
                if (mac.length() >= 17) {
                    savePC(name, mac);
                    bot.sendMessage(chat_id, "💾 Saved: *" + name + "*", "Markdown");
                }
            }
        }
        else if (text.startsWith("/delete ")) {
            String name = text.substring(8);
            name.trim();
            deletePC(name);
            bot.sendMessage(chat_id, "🗑 Deleted: " + name, "");
        }
        else if (text == "/list") {
            sendPCListMenu();
        }
    }
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    ledOn();

    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(mqttCallback); // Đảm bảo đã set callback
    secured_client.setInsecure();
    
    WOL.setRepeat(3, 100);
    WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());

    esp_task_wdt_init(WDT_TIMEOUT, true); 
    esp_task_wdt_add(NULL); 
}

void loop() {
    esp_task_wdt_reset(); 
    if (millis() % 60000 == 0 && WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect();
        WiFi.begin(ssid, password);
    }
    if (millis() > RESTART_INTERVAL) ESP.restart();

    if (!mqttClient.connected()) {
        String clientId = "ESP32C3-WOL-" + String(random(0xffff), HEX);
        if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
            mqttClient.subscribe(topic_command);
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
