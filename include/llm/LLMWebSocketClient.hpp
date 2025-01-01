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
    using EventCallback = std::function<void(LLMWebsocketEvent event, const String& eventData)>;    //用户自定义回调函数

    LLMWebSocketClient(const String& device_id);
    virtual ~LLMWebSocketClient();  //析构函数

    void setResponseCallback(ResponseCallback cb);
    void setEventCallback(EventCallback cb);
    bool connect(const String& url);
    void close();
    void poll();
    bool sendRequest(const String& question);

private:
    static LLMWebSocketClient* s_instance;

    static void onMessageCallback(WebsocketsMessage message);
    static void onEventsCallback(WebsocketsEvent event, String data);

private:
    WebsocketsClient _webSocketClient;
    ResponseCallback _responseCallback = nullptr;
    EventCallback _eventCallback = nullptr;

    bool _connected = false;
    String _device_id;
    static String _lastResponse;   //上一次的回复
};
