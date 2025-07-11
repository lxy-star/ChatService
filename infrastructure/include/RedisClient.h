// RedisClient.h - 新增心跳续期和踢人功能
#pragma once
#include <hiredis/hiredis.h>
#include <string>

class RedisClient {
public:
    RedisClient(const std::string& host, int port);
    ~RedisClient();

    void setLoginStatus(const std::string& user, const std::string& sessionToken);
    bool isUserOnline(const std::string& user);
    bool kickUser(const std::string& user);  // 踢出用户（删除登录状态）
    bool refreshHeartbeat(const std::string& user, int expireSeconds = 3600);

private:
    redisContext* ctx_;
};
