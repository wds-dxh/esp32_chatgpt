#include <Arduino.h>
#include "MicRecorder/MicRecorder.hpp"
#include "Megaphone/Megaphone.hpp"

// 定义录音时长（秒）和 PCM 文件路径
#define RECORD_DURATION_SEC 3
#define PCM_FILE_PATH "/record.pcm"

// 创建 MicRecorder 和 Megaphone 实例，使用默认参数
MicRecorder mic;
Megaphone megaphone;

void setup() {
    Serial.begin(115200);
    delay(1000); // 等待串口初始化

    // 初始化 SPIFFS 文件系统
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        while (1) { delay(1000); }
    }
    Serial.println("SPIFFS mounted successfully.");

    // 初始化 MicRecorder
    if (!mic.begin()) {
        Serial.println("MicRecorder initialization failed!");
        while (1) { delay(1000); }
    }
    Serial.println("MicRecorder initialized successfully.");

    // 初始化 Megaphone
    if (!megaphone.begin()) {
        Serial.println("Megaphone initialization failed!");
        while (1) { delay(1000); }
    }
    Serial.println("Megaphone initialized successfully.");

    // 设置音量增益（可选）
    megaphone.setVolume(1.0f);  // 1.0f 表示无增益，0.5f 表示减半音量，2.0f 表示音量加倍

    // 开始录音
    Serial.println("Starting audio recording...");
    File audioFile = SPIFFS.open(PCM_FILE_PATH, FILE_WRITE);
    if (!audioFile) {
        Serial.println("Failed to open file for writing");
        return;
    }

    uint32_t totalSamples = MicRecorder_DEFAULT_SAMPLE_RATE * RECORD_DURATION_SEC;
    uint32_t samplesRecorded = 0;
    const size_t bufferSize = 1024;
    int16_t buffer[bufferSize];

    while (samplesRecorded < totalSamples) {
        size_t samplesToRead = min(bufferSize, (size_t)(totalSamples - samplesRecorded));
        size_t samplesRead = mic.readPCM(buffer, samplesToRead);
        if (samplesRead == 0) {
            Serial.println("No samples read, continuing...");
            continue;
        }

        audioFile.write((uint8_t*)buffer, samplesRead * sizeof(int16_t));
        samplesRecorded += samplesRead;

        Serial.printf("Recorded %u/%u samples\r", samplesRecorded, totalSamples);
    }
    audioFile.close();
    Serial.println("\nRecording finished.");

    // 播放录制的音频
    Serial.println("Starting audio playback...");
    megaphone.playFromFile(PCM_FILE_PATH);
    Serial.println("Audio playback finished.");
}

void loop() {
    // 无需在循环中执行任何操作
}
