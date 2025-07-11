#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <fstream>
#include <chrono>

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static Logger& instance();

    void setLogLevel(LogLevel level);
    void start();
    void stop();

    void log(LogLevel level, const std::string& message);

private:
    Logger();
    ~Logger();

    // 不允许复制和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string timestamp();
    std::string currentDate();
    std::string levelToString(LogLevel level);

    void rotateLogIfNeeded();
    void writeThread();

    void deleteOldLogs(int days = 30);
    bool shouldDeleteOldLogs();

private:
    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable condVar_;
    std::queue<std::string> logQueue_;
    std::ofstream logFile_;

    bool running_;
    LogLevel minLevel_;

    int currentLineCount_ = 0;
    const int maxLinesPerFile_ = 2000;
    std::string currentLogDate_;

    // 用于控制清理频率
    std::chrono::system_clock::time_point lastCleanTime_;
};
