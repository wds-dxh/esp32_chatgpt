#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "../Bluetooth_Configuration_Wi_Fi/Bluetooth_Configuration_Wi_Fi.hpp"
#include "../interface/message_protocol.hpp"  // 添加消息协议头文件

namespace APP {

/**
 * @brief WiFi配对管理类
 * 负责处理WiFi配网和凭据管理
 */
class WifiPair {
private:
    // 使用组合模式集成蓝牙配置功能
    Bluetooth_Configuration_Wi_Fi bleConfig;
    
    // 使用匿名结构体封装缓存数据
    struct {
        std::vector<std::pair<String, String>> wifiCredentials;  // WiFi凭据列表
        int numWifi = 0;     // WiFi配置数量
        bool isDirty = true; // 缓存状态标志
    } cache;

    void loadWifiFromFlash();  // 从Flash加载WiFi配置
    void handleWifiConfiguration(const std::string& jsonData);  // 处理配网请求

public:
    /**
     * @brief 构造函数
     * 显式实现构造函数而不是使用default,避免编译错误
     */
    WifiPair() : bleConfig("ai-toys") {}  // 明确调用带参构造函数
    
    /**
     * @brief 启动配网服务
     * 初始化蓝牙服务并加载已保存的WiFi配置
     */
    void begin() {
        // 使用Lambda表达式设置回调
        bleConfig.setDataCallback([this](const std::string& data) {
            this->handleWifiConfiguration(data);
        });
        bleConfig.begin();
        loadWifiFromFlash();
    }
};

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
        MessageProtocol::MessageData msg("wifi_config", 
            MessageProtocol::Status::NOT_OK, 
            "Invalid JSON data");
        bleConfig.sendNotification(MessageProtocol::MessageHandler::serialize(msg));
        return;
    }

    const char* ssid = doc["ssid"];
    const char* password = doc["password"];

    if (!ssid || !password) {
        MessageProtocol::MessageData msg("wifi_config", 
            MessageProtocol::Status::NOT_OK, 
            "Missing SSID or password");
        bleConfig.sendNotification(MessageProtocol::MessageHandler::serialize(msg));
        return;
    }

    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        delay(1000);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Preferences preferences;
        preferences.begin("wifi", false);
        
        cache.wifiCredentials.push_back({String(ssid), String(password)});
        cache.numWifi++;
        
        preferences.putString(("ssid" + String(cache.numWifi - 1)).c_str(), ssid);
        preferences.putString(("password" + String(cache.numWifi - 1)).c_str(), password);
        preferences.putInt("numWifi", cache.numWifi);
        preferences.end();

        MessageProtocol::MessageData msg("wifi_config", 
            MessageProtocol::Status::OK, 
            "WiFi connected successfully");
        bleConfig.sendNotification(MessageProtocol::MessageHandler::serialize(msg));
    } else {
        MessageProtocol::MessageData msg("wifi_config", 
            MessageProtocol::Status::NOT_OK, 
            "Failed to connect to WiFi");
        bleConfig.sendNotification(MessageProtocol::MessageHandler::serialize(msg));
    }
}

} // namespace APP
