
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>
#include <UrlEncode.h>
#include "Megaphone/Megaphone.hpp"
Megaphone megaphone;
int time_1 = 0;
int time_2 = 0;


// Baidu API credentials
const char *baidu_api_key = "";
const char *baidu_secret_key = "";

// Get Baidu API access token
String getAccessToken(const char *api_key, const char *secret_key)
{
    String access_token = "";
    HTTPClient http;

    // 创建http请求
    http.begin("https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=" + String(api_key) + "&client_secret=" + String(secret_key));
    int httpCode = http.POST("");

    if (httpCode == HTTP_CODE_OK)
    {
        String response = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, response);
        access_token = doc["access_token"].as<String>();

        Serial.printf("[HTTP] GET access_token: %s\n", access_token);
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();

    return access_token;
}

void baiduTTS_Send(String access_token, String text)
{
    if (access_token == "")
    {
        Serial.println("access_token is null");
        return;
    }

    if (text.length() == 0)
    {
        Serial.println("text is null");
        return;
    }

    const int per = 1;
    const int spd = 5;
    const int pit = 5;
    const int vol = 10;
    const int aue = 6;

    // 进行 URL 编码
    String encodedText = urlEncode(urlEncode(text));

    // URL http请求数据封装
    String url = "https://tsn.baidu.com/text2audio";

    const char *header[] = {"Content-Type", "Content-Length"};

    url += "?tok=" + access_token;
    url += "&tex=" + encodedText;
    url += "&per=" + String(per);
    url += "&spd=" + String(spd);
    url += "&pit=" + String(pit);
    url += "&vol=" + String(vol);
    url += "&aue=" + String(aue);
    url += "&cuid=esp32s3";
    url += "&lan=zh";
    url += "&ctp=1";

    // http请求创建
    HTTPClient http;

    http.begin(url);
    http.collectHeaders(header, 2);

    // http请求
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0)
    {
        if (httpResponseCode == HTTP_CODE_OK)
        {
            String contentType = http.header("Content-Type");
            Serial.println(contentType);
            if (contentType.startsWith("audio"))
            {
                Serial.println("合成成功");

                // 获取返回的音频数据流
                Stream *stream = http.getStreamPtr();
                uint8_t buffer[512];
                size_t bytesRead = 0;

                // 设置timeout为200ms 避免最后出现杂音
                stream->setTimeout(300);

                while (http.connected() && (bytesRead = stream->readBytes(buffer, sizeof(buffer))) > 0)
                {
                    // 音频输出
                    // 转换为int16_t类型
                    int16_t *buffer16 = (int16_t *)buffer;   
                    megaphone.playPCM(buffer16, bytesRead / sizeof(int16_t));
                    delay(1);
                }

                // // 清空I2S DMA缓冲区
                // megaphone.clearBuffer();
            }
            else if (contentType.equals("application/json"))
            {
                Serial.println("合成出现错误");
            }
            else
            {
                Serial.println("未知的Content-Type");
            }
        }
        else
        {
            Serial.println("Failed to receive audio file");
        }
    }
    else
    {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
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

    // 设置音量增益（可选）
    // megaphone.setVolume(0.5f); // 1.0f 表示无增益，0.5f 表示减半音量，2.0f 表示音量加倍
    megaphone.startWriterTask();

    // 连接WiFi
    WiFi.begin("wds", "wds666666");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");

    // 获取百度API的access_token
    String access_token = getAccessToken("Tr0NBXENhXgr294rQKawoAhz", "hBWXoBV5wp8kJasDTJZ4o7ZJPnw3JyX7");

    // 合成语音
    time_1 = millis();
    baiduTTS_Send(access_token, "你好，虾哥！ 你在干嘛");
}

void loop()
{
    // put your main code here, to run repeatedly:
}
