#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <WakeOnLan.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include "config.h"

// MQTT
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Telegram
WiFiClientSecure secured_client;
UniversalTelegramBot bot(bot_token, secured_client);
unsigned long lastTimeBotRan;

// WoL
WiFiUDP udp;
WakeOnLan WOL(udp);

#define RESTART_INTERVAL 604800000 
#define WDT_TIMEOUT 30 

// --- Các hàm điều khiển LED ---
void ledOn() { digitalWrite(LED_PIN, LOW); }  // LED Super Mini tích cực mức thấp
void ledOff() { digitalWrite(LED_PIN, HIGH); }

void blinkSuccess() {
    ledOff(); delay(200); ledOn();
}

void blinkError() {
    for (int i = 0; i < 10; i++) {
        ledOff(); delay(50); ledOn(); delay(50);
    }
}

void executeWoL(String mac, String source) {
    if (mac.length() >= 17) {
        WOL.sendMagicPacket(mac.c_str());
        mqttClient.publish(topic_status, ("WoL Sent: " + mac).c_str());
        if (source == "Telegram") {
            bot.sendMessage(chat_id, "✅ Done: " + mac, "");
        }
        blinkSuccess();
    } else {
        if (source == "Telegram") {
            bot.sendMessage(chat_id, "❌ Error: Invalid MAC", "");
        }
        blinkError();
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) message += (char)payload[i];
    if (String(topic) == topic_command) executeWoL(message, "MQTT");
}

void handleNewMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id_incoming = String(bot.messages[i].chat_id);
        if (chat_id_incoming != chat_id) continue;

        String text = bot.messages[i].text;
        if (text == "/start") {
            bot.sendMessage(chat_id, "WOL System Online.", "");
        } 
        else if (text == "/mqtt") {
            String info = "🌐 *MQTT Configuration*\n\n";
            info += "📍 *Server:* `" + String(mqtt_server) + "`\n";
            info += "🔌 *Port:* " + String(mqtt_port) + "\n";
            info += "📥 *Command Topic:* `" + String(topic_command) + "`\n";
            info += "📤 *Status Topic:* `" + String(topic_status) + "`\n\n";
            info += "💡 _Dùng các thông số này để cấu hình App MQTT trên điện thoại._";
            bot.sendMessage(chat_id, info, "Markdown");
        }
        else if (text.startsWith("/wake ")) {
            executeWoL(text.substring(6), "Telegram");
        }
    }
}

void checkWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        ledOff();
        WiFi.disconnect();
        WiFi.begin(ssid, password);
    } else {
        ledOn();
    }
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    ledOff();
    
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    
    // Nháy chậm trong khi chờ WiFi
    while (WiFi.status() != WL_CONNECTED) {
        ledOn(); delay(100); ledOff(); delay(400);
    }
    
    ledOn(); // Sáng đèn báo hiệu đã online

    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(mqttCallback);
    secured_client.setInsecure();
    
    WOL.setRepeat(3, 100);
    WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());

    esp_task_wdt_init(WDT_TIMEOUT, true); 
    esp_task_wdt_add(NULL); 
}

void loop() {
    esp_task_wdt_reset(); 

    if (millis() % 60000 == 0) checkWiFi();

    if (millis() > RESTART_INTERVAL) ESP.restart();

    if (!mqttClient.connected()) {
        ledOff(); // Tắt đèn nếu mất kết nối MQTT
        String clientId = "ESP32C3-WOL-" + String(random(0xffff), HEX);
        if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
            mqttClient.subscribe(topic_command);
            ledOn();
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
