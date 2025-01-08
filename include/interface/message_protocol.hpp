#ifndef MESSAGE_PROTOCOL_HPP
#define MESSAGE_PROTOCOL_HPP

#include <string>
#include <ArduinoJson.h>

// 命名空间封装消息协议
namespace MessageProtocol {

    // 状态枚举
    enum class Status {
        OK,        // 操作成功
        NOT_OK     // 操作失败
    };

    // 将状态枚举转换为字符串的工具函数
    inline std::string statusToString(Status status) {
        return status == Status::OK ? "OK" : "NOT_OK";
    }

    // 响应消息类
    class ResponseMessage {
    public:
        std::string type;       // 消息类型
        Status status;          // 操作状态
        std::string errorMessage; // 错误信息

        // 构造函数
        ResponseMessage(const std::string& type, Status status, const std::string& errorMessage)
            : type(type), status(status), errorMessage(errorMessage) {}

        // 将消息序列化为 JSON 格式
        std::string toJSON() const {
            JsonDocument doc;  // 更新为新的 JsonDocument
            doc["type"] = type;
            doc["status"] = statusToString(status);
            doc["error_message"] = errorMessage;

            std::string json;
            serializeJson(doc, json);
            return json;
        }
    };

} 

#endif // MESSAGE_PROTOCOL_HPP
