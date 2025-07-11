// === UserManager.cpp ===
#include "UserManager.h"
#include "MySQLClient.h"
#include "RedisClient.h"

UserManager::UserManager(std::shared_ptr<MySQLClient> db, std::shared_ptr<RedisClient> cache)
    : db_(db), cache_(cache) {}

// UserManager.cpp - 修改登录和新增心跳函数
bool UserManager::login(const std::string& user, const std::string& pass, const std::string& clientIp, std::string& err) {
    if (!db_->validateUser(user, pass)) {
        err = "Invalid username or password";
        return false;
    }

    if (cache_->isUserOnline(user)) {
        // 踢掉旧用户
        cache_->kickUser(user);
        db_->setUserOffline(user);
    }

    // 生成session token（简单示例，可换成UUID或加密token）
    std::string token = user + "_session_token";

    // 保存数据库会话
    db_->saveUserSession(user, token, clientIp); // 实际用真实IP替代

    // Redis缓存登录状态
    cache_->setLoginStatus(user, token);

    return true;
}

bool UserManager::registerUser(const std::string& user, const std::string& pass, const std::string& clientIp, std::string& err) {
    if (!db_->registerUser(user, pass)) {
        err = "User already exists or failed to register";
        return false;
    }
        // 生成session token（简单示例，可换成UUID或加密token）
    std::string token = user + "_session_token";

    // 保存数据库会话
    db_->saveUserSession(user, token, clientIp); // 实际用真实IP替代

    // Redis缓存登录状态
    cache_->setLoginStatus(user, token);

    return true;
}

bool UserManager::isUserOnline(const std::string& user) {
    return cache_->isUserOnline(user);
}

bool UserManager::refreshHeartbeat(const std::string& user) {
    bool redisOk = cache_->refreshHeartbeat(user, 3600);
    bool dbOk = db_->updateHeartbeat(user);
    return redisOk && dbOk;
}
