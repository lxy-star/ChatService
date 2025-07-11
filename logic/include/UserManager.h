// === UserManager.h ===
#pragma once
#include <memory>
#include <string>

class MySQLClient;
class RedisClient;

class UserManager {
public:
    UserManager(std::shared_ptr<MySQLClient> db, std::shared_ptr<RedisClient> cache);

    bool login(const std::string& user, const std::string& pass, const std::string& clientIp, std::string& err);
    bool registerUser(const std::string& user, const std::string& pass, const std::string& clientIp, std::string& err);
    bool isUserOnline(const std::string& user);
    bool refreshHeartbeat(const std::string& user);

private:
    std::shared_ptr<MySQLClient> db_;
    std::shared_ptr<RedisClient> cache_;
};