#pragma once

#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <base64.h>       // Base64 encoding for any binary data
#include <functional>     // For std::function callbacks

using namespace websockets;

enum class LLMWebsocketEvent {
    ConnectionOpened,
    ConnectionClosed,
    GotPing,
    GotPong,
};

class LLMWebSocketClient {
public:
    using ResponseCallback = std::function<void(const String& response)>;   //用户自定义回调函数
    using BinaryCallback = std::function<void(const int16_t* data, size_t len)>;  // 新增二进制数据回调
    using EventCallback = std::function<void(LLMWebsocketEvent event, const String& eventData)>;    //用户自定义回调函数

    LLMWebSocketClient(const String& device_id);
    virtual ~LLMWebSocketClient();  //析构函数

    void setResponseCallback(ResponseCallback cb);
    void setBinaryCallback(BinaryCallback cb);  // 新增设置二进制回调方法
    void setEventCallback(EventCallback cb);
    bool connect(const String& url);
    void close();
    void poll();
    bool sendRequest(const String& question);

private:
    static LLMWebSocketClient* s_instance;

    static void onMessageCallback(WebsocketsMessage message);
    // static void onBinaryCallback(WebsocketsMessage message);  //websocket库不支持二进制数据回调，直接在onMessageCallback中处理
    static void onEventsCallback(WebsocketsEvent event, String data);

private:
    WebsocketsClient _webSocketClient;
    ResponseCallback _responseCallback = nullptr;
    BinaryCallback _binaryCallback = nullptr;  // 新增二进制回调成员
    EventCallback _eventCallback = nullptr;

    bool _connected = false;
    String _device_id;
    static String _lastResponse;   //上一次的回复
};
