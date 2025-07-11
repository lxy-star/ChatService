#pragma once
#include <string>
#include <memory>

class UserManager;
class ChatManager;
class Session;

class Dispatcher
{
public:
    Dispatcher(std::shared_ptr<UserManager> userManager,
               std::shared_ptr<ChatManager> chatManager);

    void handleRequest(std::shared_ptr<Session> session, const std::string &jsonString);

private:
    std::shared_ptr<UserManager> userManager_;
    std::shared_ptr<ChatManager> chatManager_;
};
