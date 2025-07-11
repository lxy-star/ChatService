#include "Dispatcher.h"
#include "Session.h"
#include "UserManager.h"
#include "ChatManager.h"
#include "JsonHelper.h"
#include "SessionManager.h"
#include "Logger.h"
#include <nlohmann/json.hpp>

Dispatcher::Dispatcher(std::shared_ptr<UserManager> userManager,
                       std::shared_ptr<ChatManager> chatManager)
    : userManager_(std::move(userManager)), chatManager_(std::move(chatManager)) {}

void Dispatcher::handleRequest(std::shared_ptr<Session> session, const std::string &jsonString)
{
    try
    {
        Logger::instance().log(LogLevel::DEBUG, "Received request: " + jsonString);

        auto json = JsonHelper::parseJson(jsonString);
        std::string type = json.value("type", "");
        std::string username = json.value("username", "");
        std::string password = json.value("password", "");
        std::string clientIp = session->getClientIp();

        std::string errMsg;

        if (type == "login")
        {
            if (userManager_->isUserOnline(username))
            {
                auto oldSession = SessionManager::instance().getSessionByUsername(username);
                if (oldSession)
                {
                    oldSession->send(JsonHelper::buildLoginResult(false, "You have been logged out due to another login."));
                    oldSession->close();
                    Logger::instance().log(LogLevel::WARN, "User " + username + " logged in from another location. Previous session closed.");
                }
            }

            if (userManager_->login(username, password, clientIp, errMsg))
            {
                session->setUsername(username);
                SessionManager::instance().registerUserSession(username, session);
                session->send(JsonHelper::buildLoginResult(true));
                Logger::instance().log(LogLevel::INFO, "User " + username + " logged in from " + clientIp);

                // 登录成功，发送历史聊天记录
                auto history = chatManager_->getRecentMessages(username, 100);
                for (const auto &msg : history)
                {
                    nlohmann::json jmsg = {
                        {"type", "chat"},
                        {"from", msg.fromUser},
                        {"to", msg.toUser},
                        {"message", msg.message},
                        {"sent_at", msg.sentAt}};
                    session->send(jmsg.dump());
                }
            }
            else
            {
                session->send(JsonHelper::buildLoginResult(false, errMsg, type));
                Logger::instance().log(LogLevel::WARN, "Login failed for user " + username + ": " + errMsg);
            }
        }
        else if (type == "register")
        {
            if (userManager_->registerUser(username, password, clientIp, errMsg))
            {
                session->setUsername(username);
                SessionManager::instance().registerUserSession(username, session);
                session->send(JsonHelper::buildRegisterResult(true));
                Logger::instance().log(LogLevel::INFO, "User " + username + " registered from " + clientIp);
            }
            else
            {
                session->send(JsonHelper::buildRegisterResult(false, errMsg));
                Logger::instance().log(LogLevel::WARN, "Register failed for user " + username + ": " + errMsg);
            }
        }
        else if (type == "chat")
        {
            std::string text = json.value("message", "");
            std::string from = session->getUsername();
            std::string toUser = json.value("to", "");

            nlohmann::json response = {
                {"type", "chat"},
                {"from", from},
                {"message", text}};

            if (!from.empty())
            {
                chatManager_->saveMessage(from, toUser, text);
                Logger::instance().log(LogLevel::DEBUG, "Chat saved from " + from + " to " + (toUser.empty() ? "all" : toUser));
            }

            if (!toUser.empty())
            {
                SessionManager::instance().sendToUser(toUser, response.dump());
            }
            else
            {
                SessionManager::instance().broadcastMessage(response.dump());
            }
        }
        else if (type == "heartbeat")
        {
            if (!username.empty())
            {
                userManager_->refreshHeartbeat(username);
                Logger::instance().log(LogLevel::DEBUG, "Heartbeat received from user " + username);
            }
            session->send(R"({"type":"heartbeat_ack"})");
        }
        else
        {
            session->send(JsonHelper::buildUnkownTypeResult(false, "Unknown request type: " + type, type));
            Logger::instance().log(LogLevel::ERROR, "Unknown request type: " + type);
        }
    }
    catch (const std::exception &ex)
    {
        std::string err = "Invalid JSON: ";
        err += ex.what();
        session->send(JsonHelper::buildLoginResult(false, err));
        Logger::instance().log(LogLevel::ERROR, "Dispatcher parse error: " + std::string(ex.what()));
    }
}
