#include <Arduino.h>
#include "llm/LLMWebSocketClient.hpp"
#include "WiFi_Network_Configuration/WiFi_Network_Configuration.hpp"
#include "Megaphone/Megaphone.hpp"

// 创建 WebSocket 客户端实例
LLMWebSocketClient llmClient("device_002");
WiFi_Network_Configuration webServer("AI-toys", "88888888");
Megaphone megaphone;

int time_1 = 0;
int time_2 = 0;
// 修改回调函数为二进制处理
int start_task = 0;
void onBinaryData(const int16_t *data, size_t len)
{
    if (start_task == 0)
    {
        start_task = 1;
    }
    Serial.println("bufferFree: " + String(megaphone.getBufferFree()));
    megaphone.queuePCM(data, 1024);
    megaphone.queuePCM(data + 1024, 1024);
    megaphone.queuePCM(data + 2048, 1024);
    megaphone.queuePCM(data + 3072, 1024);

    while (megaphone.getBufferFree() < 30)
    {
        delay(1);
    }
}
// 创建一个任务确保queuqe有20个数据包
void task(void *pvParameters)
{
    while (1)
    {
        if (megaphone.getBufferFree() > 40)
        {
            llmClient.sendRequest("ok");
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        delay(1);
    }
}

void onEvent(LLMWebsocketEvent event, const String &eventData)
{
    Serial.print("Event: ");
    switch (event)
    {
    case LLMWebsocketEvent::ConnectionOpened:
        Serial.println("Connection Opened");
        break;
    case LLMWebsocketEvent::ConnectionClosed:
        Serial.println("Connection Closed");
        break;
    case LLMWebsocketEvent::GotPing:
        Serial.println("Received Ping");
        break;
    case LLMWebsocketEvent::GotPong:
        Serial.println("Received Pong");
        break;
    }
    Serial.println(eventData);
}

void setup()
{
    Serial.begin(115200);

    // 初始化 Megaphone
    if (!megaphone.begin())
    {
        Serial.println("Megaphone initialization failed!");
        while (1)
        {
            delay(1000);
        }
    }
    Serial.println("Megaphone initialized successfully.");

    megaphone.startWriterTask(); // 启动写入任务
    megaphone.setVolume(0.3);    // 设置音量

    if (!webServer.connectWifi())
    {
        Serial.println("链接wifi失败，进入web服务器");
        webServer.openweb(true);
    }
    else
    {
        Serial.println("WiFi 链接成功");
    }

    // 输出本地IP地址
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // 设置回调
    llmClient.setBinaryCallback(onBinaryData);
    llmClient.setEventCallback(onEvent);

    // 连接到 WebSocket 服务
    // if (llmClient.connect("ws://lab-cqu.dxh-wds.top:8000/ws"))
    if (llmClient.connect("ws://172.20.10.3:8000/ws"))
    {
        Serial.println("WebSocket connected");
    }
    else
    {
        Serial.println("WebSocket connection failed");
    }
    time_2 = millis();
    // 模拟发送问题
    if (llmClient.sendRequest("你是谁？简单介绍一下！十个字以内"))
    {
        Serial.println("发送成功");
    }
    else
    {
        Serial.println("发送失败");
    }
    llmClient.sendRequest("ok");
}

void loop()
{
    // 轮询 WebSocket
    llmClient.poll();
    if (start_task == 1)
    {
        xTaskCreatePinnedToCore(task, "task", 4096, NULL, 5, NULL, 1);
        start_task = 2;
    }
}
