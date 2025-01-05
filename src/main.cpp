#include <Arduino.h>
#include <WiFi.h>
#include <functional>
#include "WiFi_Network_Configuration/WiFi_Network_Configuration.hpp"
#include "STT/XunFeiSttService.hpp"
#include "MicRecorder/MicRecorder.hpp"
#include "Megaphone/Megaphone.hpp"
#include "llm/LLMWebSocketClient.hpp"
#include "Strip_light/Strip_light.hpp"

String APP_ID = "0b0cacf7";
String API_SECRET = "YzJmNGQzMjI4ZjQxN2RlM2EzNzk4ZDA1";
String API_KEY = "acdcad01a0b1d08454de0595b5115f33";

XunFeiSttService stt(APP_ID, API_SECRET, API_KEY, "zh_cn");
WiFi_Network_Configuration webServer("AI-toys", "12345678");
MicRecorder recorder; // 可以直接传入参数，也可以直接使用默认参数
Megaphone megaphone;
LLMWebSocketClient llmClient("device_002");
StripLight stripLight;

unsigned long lastFeedTime = 0; // 记录最后一次接收数据的时间,座位看门狗，停止播放任务
const unsigned long WATCHDOG_TIMEOUT = 2000; // 看门狗超时时间，单位：毫秒
int start_task = 0; // 确保有20个数据包
int send_exit = 0;  // 发送exit

/*******************llmtts************************** */
void onBinaryData(const int16_t *data, size_t len)
{
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

    if (megaphone.getBufferFree() < 30)
    {
        vTaskDelay(10);
    }
    else
    {
        llmClient.sendRequest("ok");
    }
    // llmClient.sendRequest("ok"); // 这里一定不能删除，否则会导致数据包不足，后续数据包无法补充就会卡顿
}

// 创建一个任务确保queue有20个数据包
void task(void *pvParameters)
{
    while (1)
    {
        size_t bufferFree_task = megaphone.getBufferFree();
        // Serial.println("bufferFree_task: " + String(bufferFree_task));
        if (bufferFree_task > 30)
        {
            llmClient.sendRequest("ok");
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        vTaskDelay(1);
        // if (send_exit == 1)
        // {
        //     llmClient.sendRequest("exit");
        //     send_exit = 0;
        // }
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
            // llmClient.close();          // 关闭 WebSocket 连接

            megaphone.startWriterTask(); // 重新启动播放任务    看门狗读取队列的数据防止在回调函数卡死
            vTaskDelete(NULL);          // 删除看门狗任务
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
/*******************llmtts************************** */

/*******************stt************************** */
void myMessageCallback(const String &recognizedText)
{

    Serial.println("[STT] Recognized: " + recognizedText);
    megaphone.startWriterTask();

    if (send_exit == 1)
    {
        llmClient.sendRequest("exit");
        send_exit = 0;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // 要加入延时，否则会导致堵塞然后不能正常播放，反应会很慢
    // 将识别结果发送给大模型服务
    if (llmClient.sendRequest(recognizedText))
    {
        Serial.println("[LLM] Request sent: " + recognizedText);
    }
    else
    {
        Serial.println("[LLM] Failed to send request");
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // 要加入延时，否则会导致堵塞然后不能正常播放，反应会很慢
    llmClient.sendRequest("ok");
    vTaskDelay(100 / portTICK_PERIOD_MS);
    llmClient.sendRequest("ok");
    vTaskDelay(100 / portTICK_PERIOD_MS);
    llmClient.sendRequest("ok");
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

/*******************stt************************** */


//sttllm轮询任务
xTaskHandle taskHandle;
void stt_llm_poll(void *pvParameters)
{
    while (1)
    {
        stt.poll();
        llmClient.poll();
        vTaskDelay(1);
    }
    vTaskDelete(NULL);  //一定要删除任务

}


int heath = 0;
String wsUrl;
// 声明一个任务
void sttMonitorTask(void *parameter);
void setup()
{
    Serial.begin(115200);
    delay(1000);

    // 1.初始化wifi
    if (!webServer.connectWifi())
    {
        Serial.println("链接wifi失败，进入web服务器");
        webServer.openweb(true);
    }
    else
    {
        heath++;
        Serial.println("WiFi 链接成功");
    }
    // 2. 初始化录音
    if (!recorder.begin())
    {
        Serial.println("MicRecorder 初始化失败");
    }
    else
    {
        heath++;
        Serial.println("MicRecorder 初始化成功");
    }

    //  3. 初始化llmtts
    if (!megaphone.begin())
    {
        Serial.println("Megaphone initialization failed!");
    }
    else
    {
        heath++;
        Serial.println("Megaphone initialization success!");
    }

    megaphone.startWriterTask(); // 启动写入任务
    megaphone.setVolume(0.1);    // 设置音量

    // 4. 初始化llmtts（）设置回调。连接到 WebSocket 服务
    llmClient.setBinaryCallback(onBinaryData);
    llmClient.setEventCallback(onEvent);
    if (llmClient.connect("ws://172.20.10.3:8000/ws"))
    {
        heath++;
        Serial.println("WebSocket connected");
    }
    else
    {
        Serial.println("WebSocket connection failed");
    }
    lastFeedTime = millis(); // 初始化看门狗时间

    // 5. 初始化stt
    stt.setMessageCallback(myMessageCallback);
    stt.setEventCallback(myEventCallback);

    String dateString = stt.fetchServerTime("https://www.baidu.com");
    if (dateString.isEmpty())
    {
        Serial.println("[Warn] 获取时间失败, 可能会导致鉴权不正确");
    }
    wsUrl = stt.generateWsUrl("iat-api.xfyun.cn", "/v2/iat", dateString);
    // bool ok = stt.connect(wsUrl);
    // if (!ok)
    // {
    //     Serial.println("[setup] XunFeiSttService connect failed!");
    // }
    // else
    // {
    //     Serial.println("XunFeiSttService connect success!");
    // }

    if (heath == 4)
    {
        stripLight.show_flash(100, {0, 255, 0});
    }
    else
    {
        stripLight.show_flash(100, {255, 0, 0});
    }

    /**********pin*********** */
    // 设置引脚模式为上拉输入
    int inputPin = 0;
    pinMode(inputPin, INPUT_PULLUP);

    /*启动stturl监控任务*/
    xTaskCreatePinnedToCore(sttMonitorTask, "sttMonitorTask", 4096, NULL, 5, NULL, 1);

    xTaskCreatePinnedToCore(stt_llm_poll, "stt_llm_poll", 4096, NULL, 5, &taskHandle, 1);
}

int havepeople = 6;
void loop()
{
    // 启动轮询任务
    if (taskHandle == NULL)
    {
        llmClient.connect("ws://172.20.10.3:8000/ws");
        xTaskCreatePinnedToCore(stt_llm_poll, "stt_llm_poll", 4096, NULL, 5, &taskHandle, 1);
    }

    /******************llmtts***************** */
    // 轮询 WebSocket
    // llmClient.poll();
    // 确保有20个数据包
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

    /******************stt***************** */
    // stt.poll();

    // 读取音频数据
    const size_t bufferSize = 1024;
    int16_t buffer[bufferSize];
    size_t samplesToRead = bufferSize;

    // size_t samplesRead = recorder.readPCMProcessed(buffer, samplesToRead, false);
    // float rms = recorder.calculateRMS(buffer, samplesRead);
    // Serial.println("RMS: " + String(rms));
    int state = digitalRead(0);

    if (state == LOW)
    {

        havepeople = 6; // 如果是打断的情况下，重新计数，否则就会导致打断的情况下很快就判断为无人

        stripLight.setBrightness(20);
        stripLight.show_flash(100, {255, 0, 0});
        vTaskDelay(10 / portTICK_PERIOD_MS);

        // megaphone.stopWriterTask(); /// 多轮对话打断的时候，不正常
        vTaskDelay(10 / portTICK_PERIOD_MS);
        megaphone.clearBuffer();
        vTaskDelay(10 / portTICK_PERIOD_MS);
        megaphone.startWriterTask(); // 防止停止任务后，队列快速堆积导致在websocket消息回调的时候堵死
        Serial.println("开始录音");
        send_exit = 1;

        bool ok = stt.connect(wsUrl);
        if (!ok)
        {
            Serial.println("[setup] XunFeiSttService connect failed!");
        }
        else
        {
            Serial.println("XunFeiSttService connect success!");
        }
        while (1)
        {

            Serial.println("开始录音");
            // size_t samplesRead = recorder.readPCMProcessed(buffer, samplesToRead, false);
            size_t samplesRead = recorder.readPCM(buffer, samplesToRead);
            float rms = recorder.calculateRMS(buffer, samplesRead);
            stt.sendAudioData((uint8_t *)buffer, samplesRead * sizeof(int16_t), false);

            Serial.println("RMS: " + String(rms));
            if (rms < 30)
            {
                havepeople--;
                Serial.println("检测到声音");
            }
            if (havepeople == 0)
            {
                stt.sendAudioData((uint8_t *)buffer, samplesRead * sizeof(int16_t), true);
                havepeople = 6;
                // 关闭录音的时候，显示绿色
                stripLight.setBrightness(20);
                stripLight.show_flash(100, {0, 255, 0});
                break;
            }
        }
    }
}
void sttMonitorTask(void *parameter)
{
    String newDateString;
    while (1)
    {
        newDateString = stt.fetchServerTime("https://www.baidu.com");
        if (!newDateString.isEmpty())
        {
            Serial.println("已更新STT连接时间戳");
        }
        wsUrl = stt.generateWsUrl("iat-api.xfyun.cn", "/v2/iat", newDateString);
        vTaskDelay(30000 / portTICK_PERIOD_MS); // 每30秒检查一次
    }
    llmClient.connect("ws://172.20.10.3:8000/ws");
}