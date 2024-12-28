/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-11-20 18:34:23
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-12-29 04:40:55
 * @FilePath: /arduino-esp32/src/main.cpp
 * @Description: 
 * 微信: 15310638214 
 * 邮箱：wdsnpshy@163.com 
 * Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved. 
 */
#include <WiFi.h>
#include "WiFi_Network_Configuration/WiFi_Network_Configuration.hpp"    //文件名称不更改是因为改了会报错，稀奇古怪的错误
#include "Bluetooth_Configuration_Wi_Fi/Bluetooth_Configuration_Wi_Fi.hpp"


WiFi_Network_Configuration webServer("AI-toys", "12345678");
Bluetooth_Configuration_Wi_Fi bluetooth("AI-toys-ble");


void setup() {
    Serial.begin(115200);
    delay(1000); // 添加短暂延迟以确保系统稳定
    
    if(!webServer.connectWifi())
    {   

        //开启蓝牙配网
        Serial.println("开启蓝牙配网");
        bluetooth.begin();

        Serial.println("链接wifi失败，进入web服务器");
        webServer.openweb(true);
    }
    else
    {
        Serial.println("wifi链接成功");
    }
}

void loop() {
    delay(10); // 添加小延迟以防止看门狗复位
}
