#include "SessionManager.h"
#include "Session.h"
#include "Logger.h"

SessionManager &SessionManager::instance() {
    static SessionManager instance;
    return instance;
}

void SessionManager::addSession(std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(session);
    Logger::instance().log(LogLevel::INFO, "New session added from IP: " + session->getClientIp());
}

void SessionManager::removeSession(std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(session);

    std::string username = session->getUsername();
    if (!username.empty()) {
        sessionsByUsername_.erase(username);
        Logger::instance().log(LogLevel::INFO, "Session removed for user: " + username);
    } else {
        Logger::instance().log(LogLevel::INFO, "Anonymous session removed from IP: " + session->getClientIp());
    }
}

void SessionManager::broadcastMessage(const std::string &message) {
    std::lock_guard<std::mutex> lock(mutex_);
    Logger::instance().log(LogLevel::DEBUG, "Broadcasting message to " + std::to_string(sessions_.size()) + " sessions");
    for (const auto &session : sessions_) {
        session->send(message);
    }
}

void SessionManager::sendToUser(const std::string &username, const std::string &message) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessionsByUsername_.find(username);
    if (it != sessionsByUsername_.end()) {
        it->second->send(message);
        Logger::instance().log(LogLevel::DEBUG, "Message sent to user: " + username);
    } else {
        Logger::instance().log(LogLevel::WARN, "Attempted to send message to offline user: " + username);
    }
}

void SessionManager::registerUserSession(const std::string &username, std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessionsByUsername_[username] = session;
    Logger::instance().log(LogLevel::INFO, "User " + username + " registered with session");
}

std::shared_ptr<Session> SessionManager::getSessionByUsername(const std::string &username) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessionsByUsername_.find(username);
    if (it != sessionsByUsername_.end()) {
        return it->second;
    }
    return nullptr;
}
