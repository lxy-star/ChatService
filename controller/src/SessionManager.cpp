// === SessionManager.cpp ===
#include "SessionManager.h"
#include "Session.h"

SessionManager &SessionManager::instance()
{
    static SessionManager instance;
    return instance;
}

void SessionManager::addSession(std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(session);
}

void SessionManager::removeSession(std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(session);
    if (!session->getUsername().empty())
    {
        sessionsByUsername_.erase(session->getUsername());
    }
}

void SessionManager::broadcastMessage(const std::string &message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &session : sessions_)
    {
        session->send(message);
    }
}

void SessionManager::sendToUser(const std::string &username, const std::string &message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessionsByUsername_.find(username);
    if (it != sessionsByUsername_.end())
    {
        it->second->send(message);
    }
}

void SessionManager::registerUserSession(const std::string &username, std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessionsByUsername_[username] = session;
}

std::shared_ptr<Session> SessionManager::getSessionByUsername(const std::string &username)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessionsByUsername_.find(username);
    if (it != sessionsByUsername_.end())
    {
        return it->second;
    }
    return nullptr;
}