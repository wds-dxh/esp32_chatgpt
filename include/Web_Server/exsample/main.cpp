/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-11-20 18:34:23
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-12-10 20:07:46
 * @FilePath: /arduino-esp32/src/main.cpp
 * @Description: 
 * 微信: 15310638214 
 * 邮箱：wdsnpshy@163.com 
 * Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved. 
 */
#include <WiFi.h>
#include "WiFi_Network_Configuration/web_server.hpp"

WebServer webServer("ESP32-AP", "12345678");


void setup() {
    Serial.begin(115200);
    delay(1000); // 添加短暂延迟以确保系统稳定
    
    // 尝试连接已保存的WiFi
    if(!webServer.connectWifi()) {
        Serial.println("WiFi连接失败，启动配网服务");

        // 同时启动Web配网服务作为备选
        webServer.openweb(true);
    } else {
        Serial.println("WiFi连接成功");
    }
}

void loop() {
    delay(10); // 添加小延迟以防止看门狗复位
}