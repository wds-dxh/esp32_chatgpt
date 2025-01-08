#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <functional>
#include "../interface/message_protocol.hpp"
#include <Preferences.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <vector>

// 前向声明
namespace APP {
    class WifiPair;
}

class Bluetooth_Configuration_Wi_Fi {
    friend class APP::WifiPair;  // 现在可以正确识别友元类

public:
    using DataCallback = std::function<void(const std::string&)>;

private:
    BLEServer *pServer;                // BLE服务器指针
    BLEService *pService;              // BLE服务指针
    BLECharacteristic *pCharacteristic;// BLE特征值指针

    // 设备名称
    String deviceName;

    // BLE服务和特征值的UUID
    const char *SERVICE_UUID = "12345678-1234-1234-1234-1234567890ab";
    const char *CHAR_UUID = "12345678-1234-1234-1234-1234567890cd";

    DataCallback dataCallback;

    // 处理BLE特征值写入的回调类
    class CharacteristicCallbacks : public BLECharacteristicCallbacks {
    private:
        Bluetooth_Configuration_Wi_Fi *parent;

    public:
        explicit CharacteristicCallbacks(Bluetooth_Configuration_Wi_Fi *p);
        void onWrite(BLECharacteristic *pCharacteristic) override;
    };

    // 处理接收到的数据
    void handleReceivedData(const std::string &data);

    // 添加缓存
    struct {
        std::vector<std::pair<String, String>> wifiCredentials; // WiFi凭证
        int numWifi = 0;
        bool isDirty = true;
    } cache;


    // 从flash加载WiFi信息到缓存，减少flash读取次数
    void loadWifiFromFlash() {
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

protected:
    void begin();
    
public:
    explicit Bluetooth_Configuration_Wi_Fi(const String& deviceName);
    void setDataCallback(DataCallback callback);
    void sendNotification(const MessageProtocol::ResponseMessage& message);
};

