# WebServer 类说明文档

## 概述

`WebServer` 类提供了一个封装的接口来创建和管理ESP32的Web服务器，支持WiFi连接管理和DNS服务。该类简化了WiFi配置过程，提供了用户友好的Web界面来管理WiFi连接。

## 功能

- **WiFi管理**：保存、删除和连接WiFi网络
- **AP模式**：创建访问点供设备连接
- **DNS拦截**：可选的DNS拦截功能，将所有域名请求重定向到ESP32
- **Web界面**：提供直观的HTML界面进行WiFi管理

## 主要函数介绍

### 构造函数

```cpp
WebServer::WebServer(String apSsid, String apPassword)
```

- **简介**：初始化WebServer实例，设置AP模式的SSID和密码
- **参数**：
  - `apSsid`：AP模式的网络名称
  - `apPassword`：AP模式的密码
- **说明**：创建WebServer实例时需要指定AP模式的网络名称和密码，这些信息将在设备无法连接到已保存的WiFi网络时使用

### 打开Web服务

```cpp
void WebServer::openweb(bool Dns)
```

- **简介**：启动Web服务器和可选的DNS服务
- **参数**：
  - `Dns`：是否启用DNS拦截功能（true/false）
- **说明**：启动Web服务器，设置路由处理函数，如果启用DNS拦截，将创建DNS服务器任务

### 连接WiFi

```cpp
bool WebServer::connectWifi()
```

- **简介**：尝试连接已保存的WiFi网络
- **返回值**：连接成功返回true，失败返回false
- **说明**：从Flash存储中读取保存的WiFi信息，并尝试按顺序连接，直到成功连接或尝试完所有保存的网络

## 路由说明

- **/** - 根路由，显示主页面
- **/wifi** - WiFi管理页面
- **/saveWifiSuccessfully** - 保存WiFi信息
- **/deleteWifiSuccessfully** - 删除指定WiFi
- **/deleteAllWifiSuccessfully** - 删除所有WiFi
- **/loadWifi** - 显示已保存的WiFi列表

## 示例代码

```cpp
#include "web_server.hpp"

void setup() {
    Serial.begin(115200);
    
    // 创建WebServer实例，设置AP模式的SSID和密码
    WebServer webServer("ESP32-AP", "12345678");
    
    // 尝试连接已保存的WiFi
    if (!webServer.connectWifi()) {
        Serial.println("无法连接到已保存的WiFi，启动AP模式");
    }
    
    // 启动Web服务器，启用DNS拦截
    webServer.openweb(true);
    
    Serial.println("Web服务器已启动");
}

void loop() {
    // Web服务器在后台运行，主循环可以执行其他任务
    delay(1000);
}
```

## 注��事项

1. **存储限制**：Flash存储空间有限，建议限制保存的WiFi网络数量
2. **DNS拦截**：启用DNS拦截功能会占用额外的系统资源
3. **安全性**：
   - AP模式密码建议使用8位以上的强密码
   - Web界面未实现认证机制，建议在实际应用中添加
4. **兼容性**：
   - 确保使用支持的ESP32开发板
   - 需要安装ESPAsyncWebServer库
5. **内存使用**：
   - DNS任务需要2048字节的堆栈空间
   - 注意监控内存使用情况，避免内存泄漏

## 依赖库

- ESPAsyncWebServer
- Preferences
- DNSServer
- WiFi
