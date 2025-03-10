#include "Bluetooth_Configuration_Wi_Fi/Bluetooth_Configuration_Wi_Fi.hpp"
#include "interface/message_protocol.hpp"

int role = 0; 
// 
Bluetooth_Configuration_Wi_Fi::Bluetooth_Configuration_Wi_Fi(String deviceName) 
    : deviceName(deviceName), pServer(nullptr), pService(nullptr), pWriteCharacteristic(nullptr), pNotifyCharacteristic(nullptr) {}

// 回调类构造函数实现
Bluetooth_Configuration_Wi_Fi::Bluetooth_Configuration_Wi_Fi::CharacteristicCallbacks::CharacteristicCallbacks(Bluetooth_Configuration_Wi_Fi *p) 
    : parent(p) {}

// 回调类的onWrite方法实现
void Bluetooth_Configuration_Wi_Fi::CharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
    std::string receivedData = pCharacteristic->getValue();
    parent->handleReceivedData(receivedData);
}

// 处理接收数据的方法实现
void Bluetooth_Configuration_Wi_Fi::handleReceivedData(const std::string &data) {
    JsonDocument doc;  // 使用新的 JsonDocument 替代废弃的 StaticJsonDocument
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
        Serial.println("JSON解析失败");
        return;
    }

    // 设置角色消息
    if (doc["type"] == "set_role") {
        role = doc["role"];
        Serial.printf("角色已设置为: %d\n", role);
        
        // 创建并发送响应消息
        MessageProtocol::MessageData response("set_role");
        response.status = MessageProtocol::Status::OK;
        std::string responseJson = MessageProtocol::MessageHandler::serialize(response);
        pNotifyCharacteristic->setValue(responseJson);
        pNotifyCharacteristic->notify();
        return;
    }

    // 如果Wi-Fi是连接成功的就直接返回成功
    if (WiFi.status() == WL_CONNECTED) {
        MessageProtocol::MessageData response("wifi_config");
        response.status = MessageProtocol::Status::OK;
        std::string responseJson = MessageProtocol::MessageHandler::serialize(response);
        pNotifyCharacteristic->setValue(responseJson);
        pNotifyCharacteristic->notify();
        return;
    }

    const char *ssid = doc["ssid"];
    const char *password = doc["password"];

    if (ssid && password) {
        loadWifiFromFlash();  // 确保缓存是最新的

        // 检查是否已经存在相同的 WiFi 名称
        bool wifiExists = false;
        for (const auto& wifi : cache.wifiCredentials) {
            if (wifi.first == ssid) {
                wifiExists = true;
                break;
            }
        }

        if (!wifiExists) {
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

            Serial.printf("收到配网消息  WiFi凭证已保存: SSID=%s, 密码=%s\n", ssid, password);
            Serial.printf("当前已保存 %d 个WiFi配置\n", cache.numWifi);
        } else {
            Serial.printf("WiFi凭证已存在");
        }

        // 尝试连接WiFi
        Serial.println("正在尝试连接WiFi...");
        WiFi.begin(ssid, password);
        
        // 等待WiFi连接，最多等待10秒
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            Serial.print("...连接中");
            attempts++;
            if (attempts == 3) {
                attempts = 0;
                Serial.println("WiFi连接超时..........");
                break;
            }
        }
            
        MessageProtocol::MessageData response("wifi_config");
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi连接成功！");
            Serial.print("IP地址: ");
            Serial.println(WiFi.localIP());
            
            // 停止蓝牙广播（可选）
            // BLEDevice::deinit(true);
            // Serial.println("蓝牙配网服务已关闭");

            // 设置响应状态为成功
            response.status = MessageProtocol::Status::OK;
        } else {
            Serial.println("\nWiFi连接失败，请检查凭证是否正确");

            // 设置响应状态为失败并添加错误信息
            response.status = MessageProtocol::Status::NOT_OK;
            response.errorMessage = "WiFi连接失败，请检查凭证是否正确";
            //重新设置为ap模式
            WiFi.mode(WIFI_AP);
        }

        // 序列化响应消息并发送
        std::string responseJson = MessageProtocol::MessageHandler::serialize(response);
        pNotifyCharacteristic->setValue(responseJson);
        pNotifyCharacteristic->notify();
    } else {
        Serial.println("收到无效数据");
    }
}

// 服务器回调类构造函数实现
Bluetooth_Configuration_Wi_Fi::ServerCallbacks::ServerCallbacks(Bluetooth_Configuration_Wi_Fi *p) 
    : parent(p) {}

// 服务器回调类实现
void Bluetooth_Configuration_Wi_Fi::ServerCallbacks::onConnect(BLEServer* pServer) {
    Serial.println("蓝牙设备已连接");
    
    // 发送连接确认消息
    MessageProtocol::MessageData response("connection");
    response.status = MessageProtocol::Status::OK;
    response.errorMessage = "Connected successfully";
    std::string responseJson = MessageProtocol::MessageHandler::serialize(response);
    parent->pNotifyCharacteristic->setValue(responseJson);
    parent->pNotifyCharacteristic->notify();
}

void Bluetooth_Configuration_Wi_Fi::ServerCallbacks::onDisconnect(BLEServer* pServer) {
    Serial.println("蓝牙设备已断开连接，重新启动广播");
    parent->startAdvertising();
}

// begin方法实现
void Bluetooth_Configuration_Wi_Fi::begin() {
    // 初始化BLE设备
    BLEDevice::init(deviceName.c_str());

    // 创建BLE服务器
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(this));

    // 创建BLE服务
    pService = pServer->createService(SERVICE_UUID);

    // 创建写入特征值
    pWriteCharacteristic = pService->createCharacteristic(
        WRITE_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );

    // 创建通知特征值
    pNotifyCharacteristic = pService->createCharacteristic(
        NOTIFY_CHAR_UUID,
        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
    );
    
    // 为通知特征值创建描述符
    BLEDescriptor* pDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2902));
    pNotifyCharacteristic->addDescriptor(pDescriptor);

    // 设置写入事件的回调
    pWriteCharacteristic->setCallbacks(new CharacteristicCallbacks(this));

    // 启动服务
    pService->start();

    // 开始广播
    startAdvertising();

    Serial.println("BLE配网服务已启动");
}

void Bluetooth_Configuration_Wi_Fi::startAdvertising() {
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
    Serial.println("BLE广播已启动");
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