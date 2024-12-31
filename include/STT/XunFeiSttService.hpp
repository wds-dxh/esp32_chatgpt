#pragma once

#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <base64.h>       // 录音数据需要 Base64 编码
#include <functional>     // 若使用 std::function 回调

using namespace websockets; // 使用WebSocket命名空间

/**
 * @brief WebSocket 事件类型示例（可根据实际需求增加/修改）
 */
enum class SttWebsocketEvent {
    ConnectionOpened,
    ConnectionClosed,
    GotPing,
    GotPong,
};

class XunFeiSttService {
public:
    /**
     * @brief 当成功解析出了可用文本(或一次识别完整结束)时，
     *        通过此回调将识别结果通知上层。
     */
    using MessageCallback = std::function<void(const String& recognizedText)>;

    /**
     * @brief 事件回调函数类型：当连接成功、连接关闭、Ping、Pong 等事件发生时，
     *        将相应事件和附带信息通知上层。
     */
    using EventCallback = std::function<void(SttWebsocketEvent event, const String& eventData)>;

public:
    /**
     * @brief 构造函数：在这里直接传入 讯飞平台的鉴权相关信息
     * @param appId     讯飞 AppId
     * @param apiKey    讯飞 APIKey
     * @param apiSecret 讯飞 APISecret
     * @param language  语音识别使用的语言, 缺省为 "zh_cn"
     */
    XunFeiSttService(const String& appId,
                     const String& apiSecret,
                     const String& apiKey,
                     const String& language = "zh_cn");

    virtual ~XunFeiSttService();

    /**
     * @brief 如果运行中需要更改语言(如中英文切换)，可调用此方法
     */
    void setLanguage(const String& language);

    /**
     * @brief 从外部网络获取当前 GMT 时间(可替换成你需要的时源地址)
     * @param timeServerUrl 默认使用百度首页 "https://www.baidu.com" 获取 HTTP Header 中的 Date 字段
     * @return 返回 GMT 时间字符串，如 "Mon, 04 Mar 2024 19:23:20 GMT"
     */
    String fetchServerTime(const String& timeServerUrl = "https://www.baidu.com");

    /**
     * @brief 根据当前时间、APIKey、APISecret 等鉴权信息生成可用的 WebSocket URL
     * @param host 例 "iat-api.xfyun.cn"
     * @param path 例 "/v2/iat"
     * @param date fetchServerTime() 返回的 GMT 时间
     * @return 鉴权后的完整 ws/wss URL
     */
    String generateWsUrl(const String& host, const String& path, String date);

    /**
     * @brief 注册识别结果回调
     */
    void setMessageCallback(MessageCallback cb);

    /**
     * @brief 注册事件回调
     */
    void setEventCallback(EventCallback cb);

    /**
     * @brief 连接到指定的 WebSocket URL，一般是上面 generateWsUrl(...) 生成的地址
     */
    bool connect(const String& url);

    /**
     * @brief 主动关闭连接
     */
    void close();

    /**
     * @brief 轮询 WebSocket 客户端的消息接收与事件处理，应在 loop() 中定时调用
     */
    void poll();

    /**
     * @brief 发送音频数据帧给服务器
     * @param data 音频PCM数据指针 (8k/16bit/单声道)
     * @param length data 长度（字节数）
     * @param isEndFrame 是否最后一帧(若 true，则发送 status=2 标记结束)
     */
    void sendAudioData(const uint8_t* data, size_t length, bool isEndFrame);

private:

    static XunFeiSttService* s_instance; ///< 用于静态回调函数中访问实例

    /**
     * @brief WebSocket库自带的消息回调(收到服务端文本/二进制消息时触发)
     */
    static void onMessageCallback(WebsocketsMessage message);

    /**
     * @brief WebSocket库自带的事件回调(连接成功/断开/Ping/Pong)
     */
    static void onEventsCallback(WebsocketsEvent event, String data);

private:
    WebsocketsClient _webSocketClient; ///< 实际的 WebSocket 客户端
    MessageCallback  _messageCallback  = nullptr; ///< 识别结果回调
    EventCallback    _eventCallback    = nullptr; ///< 事件通知回调

    bool   _connected  = false; ///< 是否已连接标记
    String _appId;              ///< 讯飞 AppId
    String _apiKey;             ///< 讯飞 APIKey
    String _apiSecret;          ///< 讯飞 APISecret
    String _language;           ///< 语种(如 "zh_cn")
};

