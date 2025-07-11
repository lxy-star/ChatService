#include "Dispatcher.h"
#include "Session.h"
#include "UserManager.h"
#include "ChatManager.h"
#include "JsonHelper.h"
#include "SessionManager.h"
#include <nlohmann/json.hpp>

Dispatcher::Dispatcher(std::shared_ptr<UserManager> userManager,
                       std::shared_ptr<ChatManager> chatManager)
    : userManager_(std::move(userManager)), chatManager_(std::move(chatManager)) {}

void Dispatcher::handleRequest(std::shared_ptr<Session> session, const std::string &jsonString)
{
    try
    {
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
                }
            }
            if (userManager_->login(username, password,clientIp,errMsg))
            {
                session->setUsername(username);
                SessionManager::instance().registerUserSession(username, session);
                session->send(JsonHelper::buildLoginResult(true));

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
                session->send(JsonHelper::buildLoginResult(false, errMsg));
            }
        }
        else if (type == "register")
        {
            if (userManager_->registerUser(username, password, clientIp, errMsg))
            {
                session->setUsername(username);
                SessionManager::instance().registerUserSession(username, session);
                session->send(JsonHelper::buildRegisterResult(true));
            }
            else
            {
                session->send(JsonHelper::buildRegisterResult(false, errMsg));
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
            }
            session->send(R"({"type":"heartbeat_ack"})");
        }
        else
        {
            session->send(JsonHelper::buildLoginResult(false, "Unknown request type: " + type));
        }
    }
    catch (const std::exception &ex)
    {
        session->send(JsonHelper::buildLoginResult(false, "Invalid JSON: " + std::string(ex.what())));
    }
}
