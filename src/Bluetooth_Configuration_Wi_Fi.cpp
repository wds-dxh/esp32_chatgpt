#include "Bluetooth_Configuration_Wi_Fi/Bluetooth_Configuration_Wi_Fi.hpp"

Bluetooth_Configuration_Wi_Fi::Bluetooth_Configuration_Wi_Fi(const String& deviceName) 
    : deviceName(deviceName), pServer(nullptr), pService(nullptr), pCharacteristic(nullptr) {}

Bluetooth_Configuration_Wi_Fi::CharacteristicCallbacks::CharacteristicCallbacks(Bluetooth_Configuration_Wi_Fi *p) 
    : parent(p) {}

void Bluetooth_Configuration_Wi_Fi::CharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
    std::string receivedData = pCharacteristic->getValue();
    if (parent->dataCallback) {
        parent->dataCallback(receivedData);
    }
}

void Bluetooth_Configuration_Wi_Fi::begin() {
    BLEDevice::init(deviceName.c_str());
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

void Bluetooth_Configuration_Wi_Fi::sendNotification(const MessageProtocol::ResponseMessage& message) {
    if (pCharacteristic) {
        std::string jsonStr = message.toJSON();
        pCharacteristic->setValue(jsonStr);
        pCharacteristic->notify();
    }
}

void Bluetooth_Configuration_Wi_Fi::setDataCallback(DataCallback callback) {
    this->dataCallback = callback;
}
