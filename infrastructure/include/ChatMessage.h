#pragma once
#include <string>

struct ChatMessage {
    std::string fromUser;
    std::string toUser;    // 如果是广播，可以是空字符串
    std::string message;
    std::string sentAt;    // 时间字符串，如 "2025-07-10 11:00:00"
};
