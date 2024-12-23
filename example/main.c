#include <Arduino.h>
#include "Megaphone/Megaphone.hpp"

// 创建一个 Megaphone 实例，使用默认参数
Megaphone megaphone;

void setup() {
    Serial.begin(115200);
    Serial.println("初始化开始...");

    // 初始化 Megaphone
    if (!megaphone.begin()) {
        Serial.println("Megaphone 初始化失败！");
        while (true) { delay(1000); } // 停止执行
    }
    Serial.println("Megaphone 初始化成功！");

    // 设置音量增益（可选）
    megaphone.setVolume(0.3f);  // 1.0f 表示无增益，0.5f 表示减半音量，2.0f 表示音量加倍

    // 播放存储在 SPIFFS 中的 PCM 文件
    Serial.println("开始播放音频文件...");
    megaphone.playFromFile("/output.pcm");  // 替换为你的 PCM 文件路径
    Serial.println("音频播放完成！");
}

void loop() {
    // 此处无需执行任何操作
}
