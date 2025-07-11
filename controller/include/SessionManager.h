// === SessionManager.h ===
#pragma once
#include <unordered_set>
#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Session;

class SessionManager {
public:
    static SessionManager& instance();  // 单例获取

    void addSession(std::shared_ptr<Session> session);
    void removeSession(std::shared_ptr<Session> session);
    void broadcastMessage(const std::string& message);
    void sendToUser(const std::string &username,const std::string& message);
    void registerUserSession(const std::string &username, std::shared_ptr<Session> session);
    std::shared_ptr<Session> getSessionByUsername(const std::string& username);

private:
    SessionManager() = default;
    ~SessionManager() = default;

    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    std::unordered_set<std::shared_ptr<Session>> sessions_;
    std::unordered_map<std::string, std::shared_ptr<Session>> sessionsByUsername_;
    std::mutex mutex_;
};
