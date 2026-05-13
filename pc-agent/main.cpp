#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <windows.h>
#include <iphlpapi.h>
#include <mosquitto.h>
#include "../include/config.h"

#pragma comment(lib, "iphlpapi.lib")

// Các biến config sẽ được lấy trực tiếp từ ../include/config.h:
// mqtt_server, mqtt_port, secret_key

std::string get_mac_address() {
    IP_ADAPTER_INFO adapter_info[16];
    DWORD buflen = sizeof(adapter_info);
    if (GetAdaptersInfo(adapter_info, &buflen) != ERROR_SUCCESS) return "";

    char mac_str[18];
    sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X",
            adapter_info[0].Address[0], adapter_info[0].Address[1],
            adapter_info[0].Address[2], adapter_info[0].Address[3],
            adapter_info[0].Address[4], adapter_info[0].Address[5]);
    return std::string(mac_str);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    if (!msg->payload) return;
    
    std::string payload((char*)msg->payload, msg->payloadlen);
    std::string my_mac = get_mac_address();
    
    // Tìm kiếm cơ bản (thay vì dùng thư viện JSON nặng nề để tối ưu RAM)
    if (payload.find("\"cmd\":\"shutdown\"") != std::string::npos && 
        payload.find(my_mac) != std::string::npos) {
        
        std::cout << "[!] Shutdown command received for MAC: " << my_mac << std::endl;
        
        // Thực hiện lệnh tắt máy của Windows
        // /s: shutdown, /f: force, /t 0: delay 0s
        system("shutdown /s /f /t 0");
    }
}

int main() {
    // Ẩn cửa sổ Console để chạy ngầm hoàn toàn
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    std::string my_mac = get_mac_address();
    if (my_mac.empty()) return 1;

    mosquitto_lib_init();
    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    
    if (!mosq) return 1;

    mosquitto_message_callback_set(mosq, on_message);

    if (mosquitto_connect(mosq, mqtt_server, mqtt_port, 60) != MOSQ_ERR_SUCCESS) {
        return 1;
    }

    // Subscribe vào topic lệnh của bạn
    std::string topic = "esp32_c3_wol/" + std::string(secret_key) + "/cmd";
    mosquitto_subscribe(mosq, NULL, topic.c_str(), 0);

    std::cout << "[+] PC Agent Online. MAC: " << my_mac << std::endl;
    std::cout << "[+] Listening on: " << topic << std::endl;

    // Vòng lặp lắng nghe mạng (Non-blocking / Event-driven)
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    
    return 0;
}
