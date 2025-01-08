#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "../Bluetooth_Configuration_Wi_Fi/Bluetooth_Configuration_Wi_Fi.hpp"
#include "../interface/message_protocol.hpp"

namespace APP {

class WifiPair {
private:
    Bluetooth_Configuration_Wi_Fi bleConfig;
    
    struct {
        std::vector<std::pair<String, String>> wifiCredentials;
        int numWifi = 0;
        bool isDirty = true;
    } cache;

    void loadWifiFromFlash();
    void handleWifiConfiguration(const std::string& jsonData);
    bool isInitialized = false;

public:
    // 构造函数只初始化必要的成员
    explicit WifiPair(const String& deviceName = "ai-toys") 
        : bleConfig(deviceName) {}
    
    // 添加 begin 方法进行实际初始化
    void begin() {
        if (isInitialized) return;
        
        // 设置回调函数
        bleConfig.setDataCallback([this](const std::string& data) {
            this->handleWifiConfiguration(data);
        });
        
        // 初始化蓝牙
        bleConfig.begin();
        
        // 加载WiFi配置
        loadWifiFromFlash();
        
        isInitialized = true;
    }
};

// 内联实现部分
inline void WifiPair::loadWifiFromFlash() {
    if (!cache.isDirty) return;
    
    Preferences preferences;
    preferences.begin("wifi", true);
    cache.numWifi = preferences.getInt("numWifi", 0);
    cache.wifiCredentials.clear();
    
    for(int i = 0; i < cache.numWifi; i++) {
        String ssid = preferences.getString(("ssid" + String(i)).c_str(), "");
        String password = preferences.getString(("password" + String(i)).c_str(), "");
        cache.wifiCredentials.push_back({ssid, password});
    }
    preferences.end();
    cache.isDirty = false;
}

inline void WifiPair::handleWifiConfiguration(const std::string& jsonData) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonData);

    if (error) {
        MessageProtocol::ResponseMessage response("wifi_config", 
            MessageProtocol::Status::NOT_OK, "Invalid JSON data");
        bleConfig.sendNotification(response);
        return;
    }

    const char* ssid = doc["ssid"];
    const char* password = doc["password"];

    if (!ssid || !password) {
        MessageProtocol::ResponseMessage response("wifi_config", 
            MessageProtocol::Status::NOT_OK, "Missing SSID or password");
        bleConfig.sendNotification(response);
        return;
    }

    // 尝试连接WiFi
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        delay(1000);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        // 保存WiFi凭证
        Preferences preferences;
        preferences.begin("wifi", false);
        
        cache.wifiCredentials.push_back({String(ssid), String(password)});
        cache.numWifi++;
        
        preferences.putString(("ssid" + String(cache.numWifi - 1)).c_str(), ssid);
        preferences.putString(("password" + String(cache.numWifi - 1)).c_str(), password);
        preferences.putInt("numWifi", cache.numWifi);
        preferences.end();

        // 发送成功响应
        MessageProtocol::ResponseMessage response("wifi_config", 
            MessageProtocol::Status::OK, "WiFi connected successfully");
        bleConfig.sendNotification(response);
    } else {
        MessageProtocol::ResponseMessage response("wifi_config", 
            MessageProtocol::Status::NOT_OK, "Failed to connect to WiFi");
        bleConfig.sendNotification(response);
    }
}

} // namespace APP
