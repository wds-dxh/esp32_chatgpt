#include "Bluetooth_Configuration_Wi_Fi/Bluetooth_Configuration_Wi_Fi.hpp"

// 
Bluetooth_Configuration_Wi_Fi::Bluetooth_Configuration_Wi_Fi(String deviceName) 
    : deviceName(deviceName), pServer(nullptr), pService(nullptr), pCharacteristic(nullptr) {}

// 回调类构造函数实现
Bluetooth_Configuration_Wi_Fi::CharacteristicCallbacks::CharacteristicCallbacks(Bluetooth_Configuration_Wi_Fi *p) 
    : parent(p) {}

// 回调类的onWrite方法实现
void Bluetooth_Configuration_Wi_Fi::CharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
    std::string receivedData = pCharacteristic->getValue();
    parent->handleReceivedData(receivedData);
}

// 处理接收数据的方法实现
void Bluetooth_Configuration_Wi_Fi::handleReceivedData(const std::string &data) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
        Serial.println("JSON解析失败");
        return;
    }

    const char *ssid = doc["ssid"];
    const char *password = doc["password"];

    if (ssid && password) {
        loadWifiFromFlash();  // 确保缓存是最新的
        
        Preferences preferences;
        preferences.begin("wifi", false);
        
        // 更新缓存
        cache.wifiCredentials.push_back({String(ssid), String(password)});
        cache.numWifi++;
        
        // 一次性写入flash
        preferences.putString(("ssid" + String(cache.numWifi - 1)).c_str(), ssid);
        preferences.putString(("password" + String(cache.numWifi - 1)).c_str(), password);
        preferences.putInt("numWifi", cache.numWifi);
        preferences.end();

        Serial.printf("WiFi凭证已保存: SSID=%s, 密码=%s\n", ssid, password);
        Serial.printf("当前已保存 %d 个WiFi配置\n", cache.numWifi);

        // 尝试连接WiFi
        Serial.println("正在尝试连接WiFi...");
        WiFi.begin(ssid, password);
        
        // 等待WiFi连接，最多等待10秒
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            delay(1000);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi连接成功！");
            Serial.print("IP地址: ");
            Serial.println(WiFi.localIP());
            
            // 停止蓝牙广播（可选）
            BLEDevice::deinit(true);
            Serial.println("蓝牙配网服务已关闭");
        } else {
            Serial.println("\nWiFi连接失败，请检查凭证是否正确");
        }
    } else {
        Serial.println("收到无效数据");
    }
}

// begin方法实现
void Bluetooth_Configuration_Wi_Fi::begin() {
    // 初始化BLE设备
    BLEDevice::init(deviceName.c_str());

    // 创建BLE服务器
    pServer = BLEDevice::createServer();

    // 创建BLE服务
    pService = pServer->createService(SERVICE_UUID);

    // 创建BLE特征值
    pCharacteristic = pService->createCharacteristic(
        CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );

    // 设置写入事件的回调
    pCharacteristic->setCallbacks(new CharacteristicCallbacks(this));

    // 启动服务
    pService->start();

    // 开始广播
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    Serial.println("BLE配网服务已启动");
}

// 获取WiFi凭证
void Bluetooth_Configuration_Wi_Fi::getCredentials(String &ssid, String &password) {
    loadWifiFromFlash();
    
    if (!cache.wifiCredentials.empty()) {
        const auto& lastWifi = cache.wifiCredentials.back();
        ssid = lastWifi.first;
        password = lastWifi.second;
    } else {
        ssid = "";
        password = "";
    }
} 
