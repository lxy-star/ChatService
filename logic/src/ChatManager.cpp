#include "ChatManager.h"

ChatManager::ChatManager(std::shared_ptr<MySQLClient> db) : db_(std::move(db)) {}

bool ChatManager::saveMessage(const std::string& fromUser, const std::string& toUser, const std::string& message) {
    return db_->saveChatMessage(fromUser, toUser, message);
}

std::vector<ChatMessage> ChatManager::getRecentMessages(const std::string& username, size_t limit) {
    return db_->queryRecentChatMessages(username, limit);
}
