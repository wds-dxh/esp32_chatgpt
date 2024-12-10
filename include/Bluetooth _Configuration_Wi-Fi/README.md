# Bluetooth_Configuration_Wi_Fi

## 概述

`Bluetooth_Configuration_Wi_Fi` 类是一个用于通过蓝牙配置 ESP32 WiFi 连接的类。它使用 BLE (Bluetooth Low Energy) 技术，允许用户通过手机应用向 ESP32 发送 WiFi 凭证，实现无线配网功能。

## 功能

- **BLE 服务初始化**：创建并启动 BLE 服务器和特征值
- **WiFi 凭证存储**：使用 Preferences 库将 WiFi 凭证保存到 Flash 存储器
- **自动连接 WiFi**：接收到凭证后自动尝试连接 WiFi
- **多组凭证管理**：支持存储多组 WiFi 凭证
- **凭证读取**：可以读取已保存的 WiFi 凭证

## 构造函数

```cpp
Bluetooth_Configuration_Wi_Fi(String deviceName);
```

- **参数**：
  - `deviceName`：蓝牙设备显示的名称

## 主要方法

### 初始化 BLE 服务

```cpp
void begin();
```

- 初始化并启动 BLE 服务
- 创建特征值并开始广播

### 获取存储的 WiFi 凭证

```cpp
void getCredentials(String &ssid, String &password);
```

- **参数**：
  - `ssid`：用于存储读取到的 WiFi SSID
  - `password`：用于存储读取到的 WiFi 密码
- 从 Flash 存储器中读取最新保存的 WiFi 凭证

## BLE 通信协议

设备接收 JSON 格式的数据，格式如下：

```json
{
    "ssid": "Your_WiFi_SSID",
    "password": "Your_WiFi_Password"
}
```

## 示例代码

```cpp
#include "Bluetooth_Configuration_Wi_Fi.hpp"

void setup() {
    Serial.begin(115200);
    
    // 创建配网对象
    Bluetooth_Configuration_Wi_Fi bleConfig("ESP32-Config");
    
    // 启动配网服务
    bleConfig.begin();
    
    // 读取已保存的凭证
    String ssid, password;
    bleConfig.getCredentials(ssid, password);
    
    if (!ssid.isEmpty()) {
        Serial.println("已保存的 WiFi 凭证：");
        Serial.println("SSID: " + ssid);
        Serial.println("Password: " + password);
    }
}

void loop() {
    // 主循环中不需要添加代码
}
```

## 工作流程

1. 调用 `begin()` 启动 BLE 服务
2. 手机通过 BLE 连接设备
3. 发送 WiFi 凭证（JSON 格式）
4. 设备接收并解析凭证
5. 保存凭证到 Flash 存储器
6. 自动尝试连接 WiFi
7. 连接成功后可选择关闭 BLE 服务

## 注意事项

- 确保在使用前已包含必要的库：
  - BLEDevice
  - Preferences
  - ArduinoJson
  - WiFi
- WiFi 连接尝试超时时间为 10 秒
- 支持存储多组 WiFi 凭证，便于在不同���景下使用
- 建议在 WiFi 连接成功后关闭 BLE 服务以节省资源
- 使用 Preferences 库存储凭证，数据掉电不丢失

## UUID 信息

- 服务 UUID: "12345678-1234-1234-1234-1234567890ab"
- 特征值 UUID: "12345678-1234-1234-1234-1234567890cd"