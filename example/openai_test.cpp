#include <Arduino.h>
#include "llm/LLMWebSocketClient.hpp"
#include "WiFi_Network_Configuration/WiFi_Network_Configuration.hpp"


// 创建 WebSocket 客户端实例
LLMWebSocketClient llmClient("device_002");
WiFi_Network_Configuration webServer("AI-toys", "88888888");



int time_1 = 0;
int time_2 = 0;
// 定义回调函数
void onResponse(const String &response) //回调函数
{
    time_2 = millis();  
    Serial.print("Received response: ");
    Serial.println(time_2 - time_1);
    Serial.println(response);
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
    llmClient.setResponseCallback(onResponse);
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
    if (llmClient.sendRequest("你是谁？简单介绍一下！"))
    {
        Serial.println("发送成功");
    }
    else
    {
        Serial.println("发送失败");
    }
}

void loop()
{
    // 轮询 WebSocket
    llmClient.poll();
}
