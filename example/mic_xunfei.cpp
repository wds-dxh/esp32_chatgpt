#include <Arduino.h>
#include <WiFi.h>
#include <functional>
#include "WiFi_Network_Configuration/WiFi_Network_Configuration.hpp"
#include "STT/XunFeiSttService.hpp"
#include "MicRecorder/MicRecorder.hpp"

String APP_ID = "0b0cacf7";
String API_SECRET = "YzJmNGQzMjI4ZjQxN2RlM2EzNzk4ZDA1";
String API_KEY = "acdcad01a0b1d08454de0595b5115f33";

XunFeiSttService stt(APP_ID, API_SECRET, API_KEY, "zh_cn");
WiFi_Network_Configuration webServer("AI-toys", "12345678");
MicRecorder recorder; // 可以直接传入参数，也可以直接使用默认参数

int time_1 = 0;
int time_2 = 0;
// 自定义回调接口
void myMessageCallback(const String &recognizedText)
{
    time_2 = millis();
    Serial.println("识别时间：" + String(time_2 - time_1) + "ms");
    // 比如直接打印到串口
    Serial.println("[myMessageCallback] recognized text: " + recognizedText);
    // 你也可以将其传给 TTS 播放，或显示到屏幕等
}
void myEventCallback(SttWebsocketEvent event, const String &eventData)
{
    switch (event)
    {
    case SttWebsocketEvent::ConnectionOpened:
        Serial.println("[myEventCallback] WebSocket连接成功");
        break;
    case SttWebsocketEvent::ConnectionClosed:
        Serial.println("[myEventCallback] WebSocket连接关闭");
        break;
    case SttWebsocketEvent::GotPing:
        Serial.println("[myEventCallback] 收到Ping");
        break;
    case SttWebsocketEvent::GotPong:
        Serial.println("[myEventCallback] 收到Pong");
        break;
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // 1) 先尝试用 WiFi_Network_Configuration 连接已配置的 Wi-Fi
    if (!webServer.connectWifi())
    {
        Serial.println("链接wifi失败，进入web服务器");
        webServer.openweb(true);
    }
    else
    {
        Serial.println("WiFi 链接成功");
    }
    // 初始化录音模块
    if (!recorder.begin())
    {
        Serial.println("MicRecorder 初始化失败");
    }
    else
    {
        Serial.println("MicRecorder 初始化成功");
    }

    stt.setMessageCallback(myMessageCallback);
    stt.setEventCallback(myEventCallback);
    String dateString = stt.fetchServerTime("https://www.baidu.com");
    if (dateString.isEmpty())
    {
        Serial.println("[Warn] 获取时间失败, 可能会导致鉴权不正确");
    }
    String wsUrl = stt.generateWsUrl("iat-api.xfyun.cn", "/v2/iat", dateString);
    bool ok = stt.connect(wsUrl);
    if (!ok)
    {
        Serial.println("[setup] XunFeiSttService connect failed!");
    }
    else
    {
        Serial.println("XunFeiSttService connect success!");
    }
}

int sendCount = 0;
int count = 0;
void loop()
{
    // 语音识别部分，需要定时 poll() 来处理 WebSocket
    stt.poll();

    const size_t bufferSize = 1024;
    int16_t buffer[bufferSize];
    size_t samplesToRead = bufferSize;

    size_t samplesRead = recorder.readPCM(buffer, samplesToRead);
    if (samplesRead == 0)
    {
        Serial.println("No samples read, continuing...");
    }

    // 开始发送音频数据到讯飞服务器
    stt.sendAudioData((uint8_t *)buffer, samplesRead * sizeof(int16_t), false);
    sendCount++;
    if (sendCount > 19)
    {
        stt.sendAudioData((uint8_t *)buffer, samplesRead * sizeof(int16_t), true);
        sendCount = 0;
        count = 1;
        time_1 = millis();
    }

    Serial.println("send_count: " + String(sendCount));
}
