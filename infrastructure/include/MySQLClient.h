// MySQLClient.h - 新增会话管理和聊天记录
#pragma once
#include <memory>
#include <string>
#include <cppconn/connection.h>

struct ChatMessage;

class MySQLClient {
public:
    MySQLClient(const std::string& host, const std::string& user,
                const std::string& password, const std::string& db);

    bool userExists(const std::string& username);
    bool validateUser(const std::string& username, const std::string& password);
    bool registerUser(const std::string& username, const std::string& password);

    // 新增会话管理
    bool saveUserSession(const std::string& username, const std::string& token, const std::string& ip);
    bool updateHeartbeat(const std::string& username);
    bool setUserOffline(const std::string& username);

    // 聊天记录存储
    bool saveChatMessage(const std::string& fromUser, const std::string& toUser,
                         const std::string& message);

    // 查询某用户相关的最新聊天记录（包括单聊和广播）
    std::vector<ChatMessage> queryRecentChatMessages(const std::string& username, size_t limit);

private:
    std::unique_ptr<sql::Connection> conn_;
};
