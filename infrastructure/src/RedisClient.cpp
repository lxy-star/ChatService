// RedisClient.cpp - 相关实现
#include "RedisClient.h"
#include <iostream>

RedisClient::RedisClient(const std::string& host, int port) {
    ctx_ = redisConnect(host.c_str(), port);
    if (!ctx_ || ctx_->err) {
        std::cerr << "Redis connection error: " << (ctx_ ? ctx_->errstr : "can't allocate redis context") << std::endl;
    }
}

RedisClient::~RedisClient() {
    if (ctx_) redisFree(ctx_);
}

void RedisClient::setLoginStatus(const std::string& user, const std::string& sessionToken) {
    redisCommand(ctx_, "SET login:%s %s EX 3600", user.c_str(), sessionToken.c_str());
}

bool RedisClient::isUserOnline(const std::string& user) {
    redisReply* reply = (redisReply*)redisCommand(ctx_, "GET login:%s", user.c_str());
    bool online = reply && reply->type == REDIS_REPLY_STRING;
    if (reply) freeReplyObject(reply);
    return online;
}

bool RedisClient::kickUser(const std::string& user) {
    redisReply* reply = (redisReply*)redisCommand(ctx_, "DEL login:%s", user.c_str());
    bool success = reply && reply->integer > 0;
    if (reply) freeReplyObject(reply);
    return success;
}

bool RedisClient::refreshHeartbeat(const std::string& user, int expireSeconds) {
    redisReply* reply = (redisReply*)redisCommand(ctx_, "EXPIRE login:%s %d", user.c_str(), expireSeconds);
    bool success = reply && reply->integer == 1;
    if (reply) freeReplyObject(reply);
    return success;
}
