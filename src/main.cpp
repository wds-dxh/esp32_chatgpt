/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-11-20 18:34:23
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2025-01-13 20:07:38
 * @FilePath: /arduino-esp32/src/main.cpp
 * @Description: 
 * 微信: 15310638214 
 * 邮箱：wdsnpshy@163.com 
 * Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved. 
 */
#include <WiFi.h>
#include "app/Wifi_pair.hpp"


APP::WifiPair wifiPair;
void setup() {
    
    wifiPair.begin();        // 显式初始化
}

void loop() {
    delay(10); // 添加小延迟以防止看门狗复位
}
