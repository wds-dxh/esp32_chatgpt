#ifndef BLUETOOTH_CONFIGURATION_WIFI_HPP
#define BLUETOOTH_CONFIGURATION_WIFI_HPP

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <vector>

class Bluetooth_Configuration_Wi_Fi {
private:
    BLEServer *pServer;                // BLE服务器指针
    BLEService *pService;              // BLE服务指针
    BLECharacteristic *pCharacteristic;// BLE特征值指针

    // 设备名称
    String deviceName = "ESP32-BLE-Config";

    // BLE服务和特征值的UUID
    const char *SERVICE_UUID = "12345678-1234-1234-1234-1234567890ab";
    const char *CHAR_UUID = "12345678-1234-1234-1234-1234567890cd";

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
        std::vector<std::pair<String, String>> wifiCredentials;
        int numWifi = 0;
        bool isDirty = true;
    } cache;

    // 新增方法：从flash加载WiFi信息到缓存
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

public:
    // 构造函数
    Bluetooth_Configuration_Wi_Fi(String deviceName);
    
    // 初始化BLE服务
    void begin();
    
    // 获取存储的WiFi凭证
    void getCredentials(String &ssid, String &password);
};


#endif // BLUETOOTH_CONFIGURATION_WIFI_HPP