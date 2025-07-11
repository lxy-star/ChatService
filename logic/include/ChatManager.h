#pragma once
#include <memory>
#include <string>
#include <vector>
#include "MySQLClient.h"

struct ChatMessage {
    std::string fromUser;
    std::string toUser;    // 空表示广播
    std::string message;
    std::string sentAt;    // 时间字符串，格式由数据库决定
};

class ChatManager {
public:
    explicit ChatManager(std::shared_ptr<MySQLClient> db);

    bool saveMessage(const std::string& fromUser, const std::string& toUser, const std::string& message);

    // 获取指定用户相关的聊天记录（可以是单聊或广播）
    std::vector<ChatMessage> getRecentMessages(const std::string& username, size_t limit = 100);

private:
    std::shared_ptr<MySQLClient> db_;
};
