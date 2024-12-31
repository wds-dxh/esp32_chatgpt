#include <Arduino.h>
#include <WiFi.h>
#include <functional> // 添加此行以确保 std::function 可用

// 这个是你的配网类
#include "WiFi_Network_Configuration/WiFi_Network_Configuration.hpp" 
// 不要改名字，否则会出现你说的“稀奇古怪的错误”

// 这是我们之前定义的头文件
#include "STT/XunFeiSttService.hpp"
// 确保 XunFeiSttService.hpp 文件中包含 setMessageCallback 和 setEventCallback 方法的声明

// 1) 创建配网对象(开启AP的默认名称和密码)
WiFi_Network_Configuration webServer("AI-toys", "12345678");

// 2) 讯飞相关配置 (示例写死)
String APP_ID     = "0b0cacf7";
String API_SECRET = "YzJmNGQzMjI4ZjQxN2RlM2EzNzk4ZDA1";
String API_KEY    = "acdcad01a0b1d08454de0595b5115f33";


// 3) 创建 XunFeiSttService 对象
XunFeiSttService stt(APP_ID, API_SECRET, API_KEY, "zh_cn");

/** 
 * @brief 当 STT 返回识别结果时，这里处理
 */
void myMessageCallback(const String& recognizedText) {
    // 比如直接打印到串口
    Serial.println("[myMessageCallback] recognized text: " + recognizedText);
    // 你也可以将其传给 TTS 播放，或显示到屏幕等
}

/** 
 * @brief 当 WebSocket 连接事件发生时，这里处理 
 */
void myEventCallback(SttWebsocketEvent event, const String& eventData) {
    switch (event) {
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
    // eventData 如果你在 onEventsCallback 里传了额外的字符串，这里可处理
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // 1) 先尝试用 WiFi_Network_Configuration 连接已配置的 Wi-Fi
    if (!webServer.connectWifi()) {
        Serial.println("链接wifi失败，进入web服务器");

        // 开启热点 AP + 配置网页
        // 用户可通过手机或电脑连“AI-toys”，访问 192.168.4.1 配置 Wi-Fi
        webServer.openweb(true);
    } else {
        Serial.println("WiFi 链接成功");

        // 2) 配置语音识别回调
        stt.setMessageCallback(myMessageCallback);
        stt.setEventCallback(myEventCallback);

        // 3) 从服务器(比如百度)获取当前时间
        String dateString = stt.fetchServerTime("https://www.baidu.com");
        if (dateString.isEmpty()) {
            Serial.println("[Warn] 获取时间失败, 可能会导致鉴权不正确");
            // 也可以考虑在此处做多次重试
        }

        // 4) 生成 ws:// 的鉴权 URL
        //    这里示例 host="spark-api.xf-yun.com", path="/v2/iat" 等，看你自己实际地址
        String wsUrl = stt.generateWsUrl("iat-api.xfyun.cn", "/v2/iat", dateString);
        // wsUrl = "ws://iat-api.xfyun.cn/v2/iat?authorization=YXBpX2tleT0iYWNkY2FkMDFhMGIxZDA4NDU0ZGUwNTk1YjUxMTVmMzMiLCBhbGdvcml0aG09ImhtYWMtc2hhMjU2IiwgaGVhZGVycz0iaG9zdCBkYXRlIHJlcXVlc3QtbGluZSIsIHNpZ25hdHVyZT0iaVl0SVJvUzBtdVFCSCttcjJOa3F2UENjM1V6NUVlaFhNdGQ0bUc3azgzST0i&date=Tue%2C+31+Dec+2024+02%3A45%3A19+GMT&host=iat-api.xfyun.cn";
        // 5) 连接 WebSocket
        bool ok = stt.connect(wsUrl);
        if (!ok) {
            Serial.println("[setup] XunFeiSttService connect failed!");
            // 你可在此做重试或给出提示
        } else {
            Serial.println("XunFeiSttService connect success!");
        }
    }
}

void loop() {
    // 如果用户失败后进入配网模式，需要看 WiFi_Network_Configuration 是否要在 loop() 里执行一些函数
    // (有些配网库需要 server.handleClient() 等, 具体看你库的实现)
    // 如果 openweb() 内部不需要，你可以忽略

    // 语音识别部分，需要定时 poll() 来处理 WebSocket
    stt.poll();

    // 模拟：如果你有录音数据，就调用 stt.sendAudioData(...)
    // 比如:
    //    static bool done = false;
    //    if (!done) {
    //       uint8_t dummyAudio[320] = {0}; // 演示用随便填的
    //       stt.sendAudioData(dummyAudio, 320, false); // 首帧
    //       stt.sendAudioData(dummyAudio, 320, false); // 中间帧
    //       stt.sendAudioData(dummyAudio, 320, true);  // 最后一帧
    //       done = true;
    //    }

    delay(10);  // 防止看门狗复位(或根据实际需求安排延时)
}
