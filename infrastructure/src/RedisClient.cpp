#include "RedisClient.h"
#include "Logger.h"

RedisClient::RedisClient(const std::string& host, int port) {
    ctx_ = redisConnect(host.c_str(), port);
    if (!ctx_ || ctx_->err) {
        Logger::instance().log(LogLevel::ERROR, std::string("Redis connection error: ") + (ctx_ ? ctx_->errstr : "can't allocate redis context"));
    } else {
        Logger::instance().log(LogLevel::INFO, "Connected to Redis: " + host + ":" + std::to_string(port));
    }
}

RedisClient::~RedisClient() {
    if (ctx_) {
        redisFree(ctx_);
        Logger::instance().log(LogLevel::INFO, "Redis connection freed");
    }
}

void RedisClient::setLoginStatus(const std::string& user, const std::string& sessionToken) {
    redisReply* reply = (redisReply*)redisCommand(ctx_, "SET login:%s %s EX 3600", user.c_str(), sessionToken.c_str());
    if (reply) {
        Logger::instance().log(LogLevel::DEBUG, "Set login status for user: " + user);
        freeReplyObject(reply);
    } else {
        Logger::instance().log(LogLevel::ERROR, "Failed to set login status for user: " + user);
    }
}

bool RedisClient::isUserOnline(const std::string& user) {
    redisReply* reply = (redisReply*)redisCommand(ctx_, "GET login:%s", user.c_str());
    bool online = false;
    if (reply) {
        online = (reply->type == REDIS_REPLY_STRING);
        Logger::instance().log(LogLevel::DEBUG, "Checked online status for user: " + user + ", online: " + (online ? "true" : "false"));
        freeReplyObject(reply);
    } else {
        Logger::instance().log(LogLevel::ERROR, "Failed to check online status for user: " + user);
    }
    return online;
}

bool RedisClient::kickUser(const std::string& user) {
    redisReply* reply = (redisReply*)redisCommand(ctx_, "DEL login:%s", user.c_str());
    bool success = false;
    if (reply) {
        success = (reply->integer > 0);
        Logger::instance().log(LogLevel::INFO, std::string("Kick user: ") + user + ", success: " + (success ? "true" : "false"));
        freeReplyObject(reply);
    } else {
        Logger::instance().log(LogLevel::ERROR, "Failed to kick user: " + user);
    }
    return success;
}

bool RedisClient::refreshHeartbeat(const std::string& user, int expireSeconds) {
    redisReply* reply = (redisReply*)redisCommand(ctx_, "EXPIRE login:%s %d", user.c_str(), expireSeconds);
    bool success = false;
    if (reply) {
        success = (reply->integer == 1);
        Logger::instance().log(LogLevel::DEBUG, std::string("Refreshed heartbeat for user: ") + user + ", success: " + (success ? "true" : "false"));
        freeReplyObject(reply);
    } else {
        Logger::instance().log(LogLevel::ERROR, "Failed to refresh heartbeat for user: " + user);
    }
    return success;
}
