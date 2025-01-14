/*
 * @Author: wds2dxh wdsnpshy@163.com
 * @Date: 2025-01-13 17:00:31
 * @Description: 
 * Copyright (c) 2025 by ${wds2dxh}, All Rights Reserved. 
 */
#ifndef MESSAGE_PROTOCOL_HPP
#define MESSAGE_PROTOCOL_HPP

#include <string>
#include <ArduinoJson.h>

/**
 * @brief 消息协议命名空间
 * 封装所有与消息通信相关的数据结构和工具函数
 */
namespace MessageProtocol {

    /**
     * @brief 操作状态枚举
     * 用于表示操作的执行结果
     */
    enum class Status {
        OK,        // 操作成功
        NOT_OK     // 操作失败
    };

    /**
     * @brief 消息数据结构
     * 使用结构体封装消息相关字段，提供更好的数据组织
     */
    struct MessageData {
        std::string type;           // 消息类型
        Status status;              // 操作状态
        std::string errorMessage;   // 错误信息

        // 构造函数，提供默认值
        MessageData(const std::string& t = "", 
                   Status s = Status::OK,
                   const std::string& err = "")
            : type(t), status(s), errorMessage(err) {}
    };

    /**
     * @brief 消息处理类
     * 提供消息序列化和反序列化功能
     */
    struct MessageHandler {
        static std::string statusToString(Status status) {
            return status == Status::OK ? "OK" : "NOT_OK";
        }

        static std::string serialize(const MessageData& data) {
            JsonDocument doc;  // 使用新的 JsonDocument 替代废弃的 DynamicJsonDocument
            doc["type"] = data.type;
            doc["status"] = statusToString(data.status);
            doc["error_message"] = data.errorMessage;

            std::string json;
            serializeJson(doc, json);
            return json;
        }
    };

} // namespace MessageProtocol

#endif // MESSAGE_PROTOCOL_HPP
