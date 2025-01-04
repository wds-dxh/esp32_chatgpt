#include "llm/LLMWebSocketClient.hpp"

// 静态成员初始化
LLMWebSocketClient* LLMWebSocketClient::s_instance = nullptr;
String LLMWebSocketClient::_lastResponse = "";

// 构造函数
LLMWebSocketClient::LLMWebSocketClient(const String& device_id)
    : _device_id(device_id) {
    s_instance = this;
}

// 析构函数
LLMWebSocketClient::~LLMWebSocketClient() {
    close();
}

// 设置响应回调
void LLMWebSocketClient::setResponseCallback(ResponseCallback cb) {
    _responseCallback = cb;
}

// 设置事件回调
void LLMWebSocketClient::setEventCallback(EventCallback cb) {
    _eventCallback = cb;
}

// 新增设置二进制回调方法
void LLMWebSocketClient::setBinaryCallback(BinaryCallback cb) {
    _binaryCallback = cb;
}

// 连接到 WebSocket 服务器
bool LLMWebSocketClient::connect(const String& url) {

    //如果没连接wifi，返回false
    if (!WiFi.status() == WL_CONNECTED) {
        return false;
    }
    if (_connected) {
        return true;
    }
    if (_webSocketClient.connect(url)) {
        _connected = true;

        // 发送鉴权信息
        _webSocketClient.send(_device_id);

        // 设置 WebSocket 库回调
        _webSocketClient.onMessage(onMessageCallback);
        _webSocketClient.onEvent(onEventsCallback);

        if (_eventCallback) {
            _eventCallback(LLMWebsocketEvent::ConnectionOpened, "Connection opened");
        }
        return true;
    } else {
        _connected = false;
        return false;
    }
}

// 关闭 WebSocket 连接
void LLMWebSocketClient::close() {
    if (_connected) {
        _webSocketClient.close();
        _connected = false;

        if (_eventCallback) {
            _eventCallback(LLMWebsocketEvent::ConnectionClosed, "Connection closed");
        }
    }
}

// 轮询 WebSocket
void LLMWebSocketClient::poll() {
    if (_connected) {
        _webSocketClient.poll();
    }
}

// 发送请求
bool LLMWebSocketClient::sendRequest(const String& question) {
    if (!_connected) {
        return false;
    }

    // 构造 JSON 请求
    JsonDocument doc;
    doc["text"] = question;
    String requestData;
    serializeJson(doc, requestData);

    _webSocketClient.send(requestData);
    return true;
}

// 静态消息回调函数
void LLMWebSocketClient::onMessageCallback(WebsocketsMessage message) {
    // if (s_instance && s_instance->_responseCallback) {
    //     s_instance->_responseCallback(message.data());
    // }
    if (s_instance && s_instance->_binaryCallback) {
        const int16_t* data = (const int16_t*)message.c_str();
        size_t len = message.length();
        s_instance->_binaryCallback(data, len);
    }

    // 更新最后的响应
    // _lastResponse = message.data();
}

// 静态事件回调函数
void LLMWebSocketClient::onEventsCallback(WebsocketsEvent event, String data) {
    if (!s_instance || !s_instance->_eventCallback) {
        return;
    }

    switch (event) {
        case WebsocketsEvent::ConnectionOpened:
            s_instance->_eventCallback(LLMWebsocketEvent::ConnectionOpened, data);
            break;
        case WebsocketsEvent::ConnectionClosed:
            s_instance->_eventCallback(LLMWebsocketEvent::ConnectionClosed, data);
            break;
        case WebsocketsEvent::GotPing:
            s_instance->_eventCallback(LLMWebsocketEvent::GotPing, data);
            break;
        case WebsocketsEvent::GotPong:
            s_instance->_eventCallback(LLMWebsocketEvent::GotPong, data);
            break;
        default:
            break;
    }
}
