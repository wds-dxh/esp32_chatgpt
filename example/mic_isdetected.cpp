#include <Arduino.h>
#include "MicRecorder/MicRecorder.hpp"


MicRecorder mic;


void setup() {
    Serial.begin(115200);
    delay(1000); // 等待串口初始化
    // 初始化 MicRecorder
    if (!mic.begin()) {
        Serial.println("MicRecorder initialization failed.");
        while (1) delay(1000);
    }
    Serial.println("MicRecorder initialized successfully.");
}

void loop() {
    // 判断是否有人声输入
    if (mic.isVoiceDetected()) {
        Serial.println("Voice detected!");
    }
    else {
        Serial.println("No voice detected.");
    }
}