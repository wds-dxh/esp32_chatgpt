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

/**
 * @brief 蓝牙配网服务类
 * 提供BLE服务器功能，用于接收WiFi配置信息
 */
class Bluetooth_Configuration_Wi_Fi {
    // 声明友元类，允许WifiPair访问protected成员
    friend class APP::WifiPair;

public:
    // 使用using声明简化回调函数类型
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

    /**
     * @brief BLE特征值回调处理类
     * 处理BLE数据写入事件
     */
    class CharacteristicCallbacks : public BLECharacteristicCallbacks {
    private:
        Bluetooth_Configuration_Wi_Fi *parent;

    public:
        explicit CharacteristicCallbacks(Bluetooth_Configuration_Wi_Fi *p);
        void onWrite(BLECharacteristic *pCharacteristic) override;
    };

protected:
    // 保护级别的begin方法，只允许友元类调用
    void begin();

public:
    /**
     * @brief 构造函数
     * @param deviceName 蓝牙设备名称,默认为"ai-toys"
     */
    explicit Bluetooth_Configuration_Wi_Fi(const String& deviceName = "ai-toys") 
        : deviceName(deviceName), pServer(nullptr), pService(nullptr), pCharacteristic(nullptr) {}

    void setDataCallback(DataCallback callback);    // 设置数据回调函数
    void sendNotification(const std::string& jsonStr);  // 发送通知
};

