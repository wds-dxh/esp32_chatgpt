/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-12-10 20:06:14
 * @Description: 蓝牙配网使用示例
 * 邮箱：wdsnpshy@163.com 
 * Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved. 
 */

#include <WiFi.h>
#include "Bluetooth _Configuration_Wi-Fi/Bluetooth _Configuration_Wi_Fi.hpp"

Bluetooth_Configuration_Wi_Fi bleConfig("ESP32-BLE-Config");

void setup() {
    Serial.begin(115200);
    delay(1000); // 添加短暂延迟以确保系统稳定
    
    // 启动蓝牙配网服务
    bleConfig.begin();
    Serial.println("蓝牙配网服务已启动");
        

}

void loop() {
    delay(10); // 添加小延迟以防止看门狗复位
}