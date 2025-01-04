#include <Arduino.h>
#include "MicRecorder/MicRecorder.hpp"
#include "Megaphone/Megaphone.hpp"


#define PCM_FILE_PATH "/audio_output.pcm"

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

   
    // 初始化 Megaphone
    if (!megaphone.begin()) {
        Serial.println("Megaphone initialization failed!");
        while (1) { delay(1000); }
    }
    Serial.println("Megaphone initialized successfully.");

    // 设置音量增益（可选）
    megaphone.setVolume(0.2f);  // 1.0f 表示无增益，0.5f 表示减半音量，2.0f 表示音量加倍


    uint32_t samplesRecorded = 0;
    const size_t bufferSize = 1024; //定义字节缓冲区大小
    int16_t buffer[bufferSize];

    // 播放录制的音频
    Serial.println("Starting audio playback...");
    File audioFile = SPIFFS.open(PCM_FILE_PATH, FILE_READ);
    if (!audioFile) {
        Serial.println("Failed to open file for reading");
        return;
    }
    megaphone.startWriterTask();
    while (audioFile.available()) {
        size_t bytesRead = audioFile.readBytes((char*)buffer, sizeof(buffer));
        megaphone.queuePCM(buffer, bytesRead / sizeof(int16_t));
        Serial.printf("bufffre:%d\n",megaphone.getBufferFree());
        while (megaphone.getBufferFree() < 1)
        {
            delay(1);   //延迟1ms
        }
        
    }
    audioFile.close();
    Serial.println("Playback finished.");


}

void loop() {
    // 无需在循环中执行任何操作
}
