/*
 * @Author: wds2dxh wdsnpshy@163.com
 * @Date: 2025-01-13 17:00:31
 * @Description: 
 * Copyright (c) 2025 by ${wds2dxh}, All Rights Reserved. 
 */
#include "Bluetooth_Configuration_Wi_Fi/Bluetooth_Configuration_Wi_Fi.hpp"

// 删除旧的构造函数
// Bluetooth_Configuration_Wi_Fi::Bluetooth_Configuration_Wi_Fi(const String& deviceName)...

Bluetooth_Configuration_Wi_Fi::CharacteristicCallbacks::CharacteristicCallbacks(Bluetooth_Configuration_Wi_Fi *p) 
    : parent(p) {}

//设置回调函数
void Bluetooth_Configuration_Wi_Fi::CharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
    std::string receivedData = pCharacteristic->getValue();
    if (parent->dataCallback) {
        parent->dataCallback(receivedData);
    }
}

void Bluetooth_Configuration_Wi_Fi::begin() {
    BLEDevice::init(deviceName.c_str());  // 使用构造时设置的设备名称
    pServer = BLEDevice::createServer();
    pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
    );

    pCharacteristic->setCallbacks(new CharacteristicCallbacks(this));
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    Serial.println("BLE配网服务已启动");
}

void Bluetooth_Configuration_Wi_Fi::sendNotification(const std::string& jsonStr) {
    if (pCharacteristic) {
        pCharacteristic->setValue(jsonStr);
        pCharacteristic->notify();
    }
}

void Bluetooth_Configuration_Wi_Fi::setDataCallback(DataCallback callback) {
    this->dataCallback = callback;
}
