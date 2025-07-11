#include "MySQLClient.h"
#include "Logger.h"
#include <cppconn/prepared_statement.h>
#include <ctime>
#include "ChatMessage.h" 
#include <mysql_driver.h>
#include <mysql_connection.h>

MySQLClient::MySQLClient(const std::string& host,
                         const std::string& user,
                         const std::string& password,
                         const std::string& db) {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        conn_.reset(driver->connect("tcp://" + host, user, password));
        conn_->setSchema(db);
        Logger::instance().log(LogLevel::INFO, "Connected to MySQL: " + host + ", DB: " + db);
    } catch (const sql::SQLException &e) {
        Logger::instance().log(LogLevel::ERROR, std::string("MySQL connection failed: ") + e.what());
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
        if (res->next() && res->getInt(1) > 0) {
            Logger::instance().log(LogLevel::INFO, "User login validated: " + username);
            return true;
        } else {
            Logger::instance().log(LogLevel::WARN, "Invalid login attempt: " + username);
        }
    } catch (const std::exception& e) {
        Logger::instance().log(LogLevel::ERROR, std::string("validateUser error: ") + e.what());
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
        Logger::instance().log(LogLevel::INFO, "New user registered: " + username);
        return true;
    } catch (const std::exception& e) {
        Logger::instance().log(LogLevel::ERROR, std::string("registerUser error: ") + e.what());
    }
    return false;
}

bool MySQLClient::userExists(const std::string& username) {
    try {
        auto stmt = conn_->prepareStatement(
            "SELECT COUNT(*) FROM users WHERE username=?");
        stmt->setString(1, username);
        auto res = stmt->executeQuery();
        if (res->next() && res->getInt(1) > 0) {
            return true;
        }
    } catch (const std::exception& e) {
        Logger::instance().log(LogLevel::ERROR, std::string("userExists error: ") + e.what());
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
        bool success = stmt->executeUpdate() > 0;
        Logger::instance().log(LogLevel::DEBUG, "Session saved for user: " + username + " IP: " + ip);
        return success;
    } catch (const std::exception& e) {
        Logger::instance().log(LogLevel::ERROR, std::string("saveUserSession error: ") + e.what());
        return false;
    }
}

bool MySQLClient::updateHeartbeat(const std::string& username) {
    try {
        auto stmt = conn_->prepareStatement(
            "UPDATE user_sessions SET last_heartbeat=NOW() WHERE username=? AND is_online=TRUE");
        stmt->setString(1, username);
        bool success = stmt->executeUpdate() > 0;
        Logger::instance().log(LogLevel::DEBUG, "Heartbeat updated for user: " + username);
        return success;
    } catch (const std::exception& e) {
        Logger::instance().log(LogLevel::ERROR, std::string("updateHeartbeat error: ") + e.what());
        return false;
    }
}

bool MySQLClient::setUserOffline(const std::string& username) {
    try {
        auto stmt = conn_->prepareStatement(
            "UPDATE user_sessions SET is_online=FALSE WHERE username=?");
        stmt->setString(1, username);
        bool success = stmt->executeUpdate() > 0;
        Logger::instance().log(LogLevel::INFO, "User set offline: " + username);
        return success;
    } catch (const std::exception& e) {
        Logger::instance().log(LogLevel::ERROR, std::string("setUserOffline error: ") + e.what());
        return false;
    }
}

bool MySQLClient::saveChatMessage(const std::string& fromUser, const std::string& toUser,
                                  const std::string& message) {
    try {
        auto stmt = conn_->prepareStatement(
            "INSERT INTO chat_messages(from_user, to_user, message, sent_at) VALUES (?, ?, ?, NOW())");
        stmt->setString(1, fromUser);
        if (toUser.empty())
            stmt->setNull(2, sql::DataType::VARCHAR);
        else
            stmt->setString(2, toUser);
        stmt->setString(3, message);
        bool success = stmt->executeUpdate() > 0;
        Logger::instance().log(LogLevel::DEBUG, "Message saved: from " + fromUser + " to " + (toUser.empty() ? "ALL" : toUser));
        return success;
    } catch (const std::exception& e) {
        Logger::instance().log(LogLevel::ERROR, std::string("saveChatMessage error: ") + e.what());
        return false;
    }
}

std::vector<ChatMessage> MySQLClient::queryRecentChatMessages(const std::string& username, size_t limit) {
    std::vector<ChatMessage> result;
    try {
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

        std::reverse(result.begin(), result.end());
        Logger::instance().log(LogLevel::DEBUG, "Fetched recent chat messages for user: " + username);
    } catch (const std::exception &ex) {
        Logger::instance().log(LogLevel::ERROR, std::string("queryRecentChatMessages error: ") + ex.what());
    }
    return result;
}
