#include "STT/XunFeiSttService.hpp"

/*--------------实现部分！ -------------------------*/

XunFeiSttService* XunFeiSttService::s_instance = nullptr; // 静态成员初始化

XunFeiSttService::XunFeiSttService(const String& appId,
    const String& apiSecret,
    const String& apiKey,
    const String& language)
    : _appId(appId), _apiSecret(apiSecret), _apiKey(apiKey), _language(language)
{
    s_instance = this;
    // 注册静态回调函数
    _webSocketClient.onMessage(onMessageCallback);
    _webSocketClient.onEvent(onEventsCallback);
}

XunFeiSttService::~XunFeiSttService()
{
    close();
    s_instance = nullptr;
}

// ---------------------- 公共函数实现 ----------------------

void XunFeiSttService::setLanguage(const String& language)
{
    _language = language;
}

String XunFeiSttService::fetchServerTime(const String& timeServerUrl /*= "https://www.baidu.com"*/)
{
    HTTPClient http;
    http.begin(timeServerUrl);

    const char* headerKeys[] = { "Date" };                                         // 定义需要收集的HTTP头字段
    http.collectHeaders(headerKeys, sizeof(headerKeys) / sizeof(headerKeys[0])); // 设置要收集的HTTP头字段

    String dateString;
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        // 获取响应头中的 Date 字段
        dateString = http.header("Date");
    }
    else
    {
        Serial.printf("fetchServerTime failed, code: %d\n", httpCode);
    }
    http.end();

    Serial.println("Fetched Date: " + dateString);
    return dateString; // 如果为空，后续 generateWsUrl() 需注意处理
}

String XunFeiSttService::generateWsUrl(const String& host, const String& path, String date)
{
    // String Spark_url = "ws://" + host + path;
    String Spark_url = "ws://iat-api.xfyun.cn/v2/iat";

    // 拼接签名原始字符串
    String signature_origin = "host: " + host + "\n";
    signature_origin += "date: " + date + "\n";
    signature_origin += "GET " + path + " HTTP/1.1";
    // 示例：signature_origin="host: spark-api.xf-yun.com\ndate: Mon, 04 Mar 2024 19:23:20 GMT\nGET /v3.5/chat HTTP/1.1";

    // 使用 HMAC-SHA256 进行加密
    unsigned char hmac[32];                                 // 存储HMAC结果
    mbedtls_md_context_t ctx;                               // HMAC上下文
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;          // 使用SHA256哈希算法
    const size_t messageLength = signature_origin.length(); // 签名原始字符串的长度
    const size_t keyLength = _apiSecret.length();           // 密钥的长度

    // 初始化HMAC上下文
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    // 设置HMAC密钥
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)_apiSecret.c_str(), keyLength);
    // 更新HMAC上下文
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)signature_origin.c_str(), messageLength);
    // 完成HMAC计算
    mbedtls_md_hmac_finish(&ctx, hmac);
    // 释放HMAC上下文
    mbedtls_md_free(&ctx);

    // 将HMAC结果进行Base64编码
    String signature_sha_base64 = base64::encode(hmac, sizeof(hmac) / sizeof(hmac[0]));

    // 替换Date字符串中的特殊字符
    date.replace(",", "%2C");
    date.replace(" ", "+");
    date.replace(":", "%3A");

    // 构建Authorization原始字符串
    String authorization_origin = "api_key=\"" + _apiKey + "\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"" + signature_sha_base64 + "\"";

    // 将Authorization原始字符串进行Base64编码
    String authorization = base64::encode(authorization_origin);

    // 构建最终的URL
    String url = Spark_url + '?' + "authorization=" + authorization + "&date=" + date + "&host=" + host;

    // 向串口输出生成的URL
    Serial.println(url);

    // 返回生成的URL
    return url;
}

bool XunFeiSttService::connect(const String& url)
{
    if (_connected)
    {
        return true;
    }
    bool result = _webSocketClient.connect(url.c_str());
    if (result)
    {
        _connected = true;
        Serial.println("Connected to server!");
    }
    else
    {
        _connected = false;
        Serial.println("Failed to connect to server!");
    }
    return result;
}

void XunFeiSttService::close()
{
    if (_connected)
    {
        _webSocketClient.close();
        _connected = false;
        Serial.println("WebSocket closed.");
    }
}

void XunFeiSttService::poll()
{
    if (_connected)
    {
        _webSocketClient.poll();
    }
}

String XunFeiSttService::askquestion = "";  // 静态成员变量要在外面初始化，初始化为空字符串

void XunFeiSttService::sendAudioData(const uint8_t* data, size_t length, bool isEndFrame)
{
    if (!_connected)
    {
        return;
    }

    // 首/中/尾帧组装 JSON 示例
    // StaticJsonDocument<1024> doc;
    JsonDocument doc;
    // JsonObject dataObj = doc.createNestedObject("data");
    JsonObject dataObj = doc["data"].to<JsonObject>();

    static bool firstFrame = true;
    if (firstFrame)
    {
        dataObj["status"] = 0; // 首帧
        firstFrame = false;

        // 参照你最初示例中的字段
        // JsonObject common = doc.createNestedObject("common");
        JsonObject common = doc["common"].to<JsonObject>();
        common["app_id"] = _appId;

        // JsonObject business = doc.createNestedObject("business");
        JsonObject business = doc["business"].to<JsonObject>();
        business["domain"] = "iat";
        business["language"] = _language;
        business["accent"] = "mandarin";
        business["vad_eos"] = 2000;
    }
    else
    {
        dataObj["status"] = 1; // 中间帧
    }

    if (isEndFrame)
    {
        dataObj["status"] = 2; // 最后一帧
        firstFrame = true;
    }

    dataObj["format"] = "audio/L16;rate=8000";
    dataObj["audio"] = base64::encode(data, length);
    dataObj["encoding"] = "raw";

    // 序列化后发送
    String payload;
    serializeJson(doc, payload);
    _webSocketClient.send(payload);
}

// -------------------- 静态回调函数 ----------------------
void XunFeiSttService::onMessageCallback(WebsocketsMessage message)
{
    // 先判断全局指针是否存在
    if (!s_instance)
    {
        return;
    }
    // 如果用户没设置 callback，就直接返回
    if (!s_instance->_messageCallback)
    {
        return;
    }

    // // 这里演示：简单打印
    // Serial.print("[WebSocket Msg] ");
    // Serial.println(message.data());

    // // 如果是 JSON，需要解析 JSON 并提取识别结果。举个例：
    // //  (以下演示 parse JSON, 假设返回格式形如 `{"data":{"result":"识别内容"}}` )
    // // StaticJsonDocument<512> doc;
    // JsonDocument doc;     //新版本的ArduinoJson库，不需要指定大小
    // DeserializationError err = deserializeJson(doc, message.data());
    // if (!err) {
    //     // 只是举例，具体看接口返回
    //     String recognized = doc["data"]["result"].as<String>();
    //     // 调用用户提供的回调
    //     s_instance->_messageCallback(recognized);
    // } else {
    //     Serial.println("[onMessageCallback] JSON parse failed");
    // }


    // 创建一个动态JSON文档对象，用于存储解析后的JSON数据，最大容量为4096字节
    // DynamicJsonDocument jsonDocument(4096);
    JsonDocument jsonDocument;

    // 解析收到的JSON数据
    DeserializationError error = deserializeJson(jsonDocument, message.data());

    if (error)
    {
        // 如果解析出错，输出错误信息和收到的消息数据
        Serial.println("error:");
        Serial.println(error.c_str());
        Serial.println(message.data());
        return;
    }
    // 如果解析没有错误，从JSON数据中获取返回码，如果返回码不为0，表示出错
    // if (jsonDocument["code"] != 0)
    int code = jsonDocument["code"];
    if (code != 0)
    {
        // 输出完整的JSON数据
        Serial.println(message.data());
        // 关闭WebSocket客户端
        s_instance->close();
    }
    else
    {
        // 输出收到的讯飞云返回消息
        // Serial.println("xunfeiyun stt return message:");
        // Serial.println(message.data());

        // 获取JSON数据中的结果部分，并提取文本内容
        JsonArray ws = jsonDocument["data"]["result"]["ws"].as<JsonArray>();

        // if (jsonDocument["data"]["status"] != 2) // 处理流式返回的内容，讯飞stt最后一次会返回一个标点符号，需要和前一次返回结果拼接起来
        int status = jsonDocument["data"]["status"];        //ArduinoJson v7 版本中的 MemberProxy 不能直接用于比较操作。需要先将值保存到变量中再比较:
        if (status != 2)
        {
            askquestion = "";
        }

        for (JsonVariant i : ws)
        {
            for (JsonVariant w : i["cw"].as<JsonArray>())
            {
                askquestion += w["w"].as<String>();
            }
        }

        // 输出提取的问句
        Serial.println(askquestion);

        // 获取状态码，等于2表示文本已经转换完成
        if (jsonDocument["data"]["status"] == 2)
        {
            // 如果状态码为2，表示消息处理完成
            // Serial.println("status == 2");
            // s_instance->close();
            s_instance->close();
        }

        s_instance->_messageCallback(askquestion);
        s_instance->close(); //稀奇古怪，不关闭的话可能还回来一个空的结果
        askquestion = "";
    }
}

void XunFeiSttService::onEventsCallback(WebsocketsEvent event, String data)
{
    if (!s_instance)
    {
        return;
    }
    if (!s_instance->_eventCallback)
    {
        return;
    }

    // 先把 websocketsEvent 转成我们自定义的 SttWebsocketEvent
    SttWebsocketEvent sttEvent;
    switch (event)
    {
    case WebsocketsEvent::ConnectionOpened:
        sttEvent = SttWebsocketEvent::ConnectionOpened;
        break;
    case WebsocketsEvent::ConnectionClosed:
        sttEvent = SttWebsocketEvent::ConnectionClosed;
        break;
    case WebsocketsEvent::GotPing:
        sttEvent = SttWebsocketEvent::GotPing;
        break;
    case WebsocketsEvent::GotPong:
        sttEvent = SttWebsocketEvent::GotPong;
        break;
    }
    // 调用用户的回调
    s_instance->_eventCallback(sttEvent, data);
}

/*----------------------设置用户自定义对应的回调函数--------------------------------------------*/
void XunFeiSttService::setMessageCallback(MessageCallback cb)
{
    _messageCallback = cb;
}
void XunFeiSttService::setEventCallback(EventCallback cb)
{
    _eventCallback = cb;
}
