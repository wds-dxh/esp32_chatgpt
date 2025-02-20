#include <Arduino.h>
#include "llm/LLMWebSocketClient.hpp"
#include "WiFi_Network_Configuration/WiFi_Network_Configuration.hpp"
#include "Megaphone/Megaphone.hpp"

// 创建 WebSocket 客户端实例
LLMWebSocketClient llmClient("device_002");
WiFi_Network_Configuration webServer("AI-toys", "88888888");
Megaphone megaphone;

unsigned long lastFeedTime = 0; // 记录最后一次接收数据的时间
const unsigned long WATCHDOG_TIMEOUT = 5000; // 看门狗超时时间，单位：毫秒

int start_task = 0; //确保有20个数据包
int time_1 = 0;
int time_2 = 0;

void onBinaryData(const int16_t *data, size_t len)
{   
    time_2 = millis();
    Serial.println("time: " + String(time_2 - time_1));
    lastFeedTime = millis(); // 更新看门狗时间

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

// 创建一个任务确保queue有20个数据包
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

// 看门狗任务
void watchdogTask(void *pvParameters)
{
    while (1) 
    {
        unsigned long currentTime = millis();
        if ((currentTime - lastFeedTime) > WATCHDOG_TIMEOUT)
        {
            Serial.println("Watchdog timeout! Closing WebSocket...");
            llmClient.close(); // 关闭 WebSocket 连接
            megaphone.stopWriterTask(); // 停止播放任务
            vTaskDelete(NULL); // 删除看门狗任务
        }
        vTaskDelay(500 / portTICK_PERIOD_MS); // 每 500ms 检查一次
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
    if (llmClient.connect("ws://47.108.223.146:8000/ws"))
    {
        Serial.println("WebSocket connected");
    }
    else
    {
        Serial.println("WebSocket connection failed");
    }

    // 模拟发送问题
    time_1 = millis();
    if (llmClient.sendRequest("你是谁？简单介绍一下！十个字以内"))
    {
        Serial.println("发送成功");
    }
    else
    {
        Serial.println("发送失败");
    }

    llmClient.sendRequest("ok");
    lastFeedTime = millis(); // 初始化看门狗时间
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

    // 启动看门狗任务
    static bool watchdogStarted = false;
    if (!watchdogStarted)
    {
        xTaskCreatePinnedToCore(watchdogTask, "WatchdogTask", 2048, NULL, 5, NULL, 1);
        watchdogStarted = true;
    }
}
