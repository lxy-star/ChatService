// MySQLClient.cpp - 实现新增方法示例
#include "MySQLClient.h"
#include <cppconn/prepared_statement.h>
#include <ctime>
#include "ChatMessage.h" 
#include <mysql_driver.h>       // for sql::mysql::MySQL_Driver
#include <mysql_connection.h>   // for sql::Connection

MySQLClient::MySQLClient(const std::string& host,
                         const std::string& user,
                         const std::string& password,
                         const std::string& db)
{
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        conn_.reset(driver->connect("tcp://" + host, user, password));
        conn_->setSchema(db);
    } catch (const sql::SQLException &e) {
        std::cerr << "MySQLClient constructor error: " << e.what() << std::endl;
        throw;
    }
}

bool MySQLClient::validateUser(const std::string& username, const std::string& password) {
    try {
        auto stmt = conn_->prepareStatement(
            "SELECT COUNT(*) FROM users WHERE username=? AND password=?");
        stmt->setString(1, username);
        stmt->setString(2, password);
        auto res = stmt->executeQuery();
        if (res->next()) {
            return res->getInt(1) > 0;
        }
    } catch (...) {
    }
    return false;
}

bool MySQLClient::registerUser(const std::string& username, const std::string& password) {
    try {
        auto stmt = conn_->prepareStatement(
            "INSERT INTO users(username, password) VALUES (?, ?)");
        stmt->setString(1, username);
        stmt->setString(2, password);
        stmt->executeUpdate();
        return true;
    } catch (...) {
        return false;
    }
}

bool MySQLClient::userExists(const std::string& username) {
    try {
        auto stmt = conn_->prepareStatement(
            "SELECT COUNT(*) FROM users WHERE username=?");
        stmt->setString(1, username);
        auto res = stmt->executeQuery();
        if (res->next()) {
            return res->getInt(1) > 0;
        }
    } catch (...) {
    }
    return false;
}


bool MySQLClient::saveUserSession(const std::string& username, const std::string& token, const std::string& ip) {
    try {
        auto stmt = conn_->prepareStatement(
            "INSERT INTO user_sessions(username, session_token, login_time, last_heartbeat, ip, is_online) "
            "VALUES (?, ?, NOW(), NOW(), ?, TRUE) "
            "ON DUPLICATE KEY UPDATE session_token=?, last_heartbeat=NOW(), ip=?, is_online=TRUE");
        stmt->setString(1, username);
        stmt->setString(2, token);
        stmt->setString(3, ip);
        stmt->setString(4, token);
        stmt->setString(5, ip);
        return stmt->executeUpdate() > 0;
    } catch (...) {
        return false;
    }
}

bool MySQLClient::updateHeartbeat(const std::string& username) {
    try {
        auto stmt = conn_->prepareStatement(
            "UPDATE user_sessions SET last_heartbeat=NOW() WHERE username=? AND is_online=TRUE");
        stmt->setString(1, username);
        return stmt->executeUpdate() > 0;
    } catch (...) {
        return false;
    }
}

bool MySQLClient::setUserOffline(const std::string& username) {
    try {
        auto stmt = conn_->prepareStatement(
            "UPDATE user_sessions SET is_online=FALSE WHERE username=?");
        stmt->setString(1, username);
        return stmt->executeUpdate() > 0;
    } catch (...) {
        return false;
    }
}

bool MySQLClient::saveChatMessage(const std::string& fromUser, const std::string& toUser,
                                  const std::string& message) {
    try {
        auto stmt = conn_->prepareStatement(
            "INSERT INTO chat_messages(from_user, to_user, message, sent_at) VALUES (?, ?, ?, NOW())");
        stmt->setString(1, fromUser);
        if (toUser.empty()) stmt->setNull(2, sql::DataType::VARCHAR);
        else stmt->setString(2, toUser);
        stmt->setString(3, message);
        return stmt->executeUpdate() > 0;
    } catch (...) {
        return false;
    }
}

std::vector<ChatMessage> MySQLClient::queryRecentChatMessages(const std::string& username, size_t limit) {
    std::vector<ChatMessage> result;
    try {
        // 查询对该用户的单聊消息，和广播消息(to_user IS NULL)
        // 按时间倒序，限制条数
        auto stmt = conn_->prepareStatement(
            "SELECT from_user, to_user, message, sent_at FROM chat_messages "
            "WHERE to_user = ? OR to_user IS NULL "
            "ORDER BY sent_at DESC LIMIT ?");
        stmt->setString(1, username);
        stmt->setInt(2, static_cast<int>(limit));
        auto res = stmt->executeQuery();

        while (res->next()) {
            ChatMessage msg;
            msg.fromUser = res->getString("from_user");
            msg.toUser = res->isNull("to_user") ? "" : res->getString("to_user");
            msg.message = res->getString("message");
            msg.sentAt = res->getString("sent_at");
            result.push_back(std::move(msg));
        }

        // 返回结果按时间正序排列（客户端按顺序显示）
        std::reverse(result.begin(), result.end());
    } catch (const std::exception &ex) {
        std::cerr << "queryRecentChatMessages error: " << ex.what() << std::endl;
    }
    return result;
}
