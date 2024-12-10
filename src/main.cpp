/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-12-10 16:05:48
 * @Description: 
 * 邮箱：wdsnpshy@163.com 
 * Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved. 
 */
#include <Arduino.h>
#include "MicRecorder/MicRecorder.hpp"

// 创建 MicRecorder 实例
MicRecorder mic;

void setup() {
  Serial.begin(115200);  // 初始化串口
  
  // 初始化 MicRecorder 并检查驱动是否正常工作
  if (!mic.begin()) {
    Serial.println("MicRecorder initialization failed! Check I2S configuration or wiring.");
    while (1);  // 驱动初始化失败时停止运行
  }

  Serial.println("MicRecorder initialized successfully!");
}

void loop() {
  const size_t BUFFER_SIZE = 1024;  // 缓冲区大小
  int16_t buffer[BUFFER_SIZE];     // 定义读取缓冲区

  // 读取 PCM 数据
  size_t samplesRead = mic.readPCM(buffer, BUFFER_SIZE);

  // 判断是否成功读取到数据
  if (samplesRead > 0) {
    // 计算 RMS 值
    float rms = mic.calculateRMS(buffer, samplesRead);

    // 输出 RMS 值到串口
    Serial.print("RMS Value: ");
    Serial.println(rms);
  } else {
    Serial.println("Failed to read PCM data.");
  }

  delay(1000);
}
