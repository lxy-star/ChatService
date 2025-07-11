#include "Logger.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

Logger::Logger() : running_(false), minLevel_(LogLevel::DEBUG) {}

Logger::~Logger() {
    stop();
}

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(LogLevel level) {
    minLevel_ = level;
}

void Logger::start() {
    running_ = true;
    currentLogDate_ = currentDate();
    std::filesystem::create_directories("logs");
    logFile_.open("logs/log_" + currentLogDate_ + ".txt", std::ios::app);
    lastCleanTime_ = std::chrono::system_clock::now();
    worker_ = std::thread(&Logger::writeThread, this);
}

void Logger::stop() {
    running_ = false;
    condVar_.notify_all();
    if (worker_.joinable())
        worker_.join();
    if (logFile_.is_open())
        logFile_.close();
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < minLevel_) return;

    std::lock_guard<std::mutex> lock(mutex_);
    std::string formatted = timestamp() + " [" + levelToString(level) + "] " + message;
    logQueue_.push(formatted);
    condVar_.notify_one();
}

std::string Logger::timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "]";
    return oss.str();
}

std::string Logger::currentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

bool Logger::shouldDeleteOldLogs() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto diff = duration_cast<hours>(now - lastCleanTime_).count() / 24;
    return diff >= 1;  // 一天一次
}

void Logger::deleteOldLogs(int days) {
    namespace fs = std::filesystem;
    try {
        const auto now = std::chrono::system_clock::now();
        for (const auto& entry : fs::directory_iterator("logs")) {
            if (entry.is_regular_file()) {
                auto ftime = fs::last_write_time(entry);
                // 转换到 system_clock 时间点
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now()
                );

                auto age = std::chrono::duration_cast<std::chrono::hours>(now - sctp).count() / 24;
                if (age > days) {
                    fs::remove(entry.path());
                    log(LogLevel::INFO, "Deleted old log file: " + entry.path().string());
                }
            }
        }
    } catch (const std::exception& e) {
        log(LogLevel::ERROR, std::string("Error deleting old logs: ") + e.what());
    }
    lastCleanTime_ = std::chrono::system_clock::now();
}


void Logger::rotateLogIfNeeded() {
    std::string today = currentDate();
    if (currentLineCount_ >= maxLinesPerFile_ || today != currentLogDate_) {
        if (logFile_.is_open()) logFile_.close();
        currentLogDate_ = today;
        currentLineCount_ = 0;
        std::string filename = "logs/log_" + currentLogDate_ + ".txt";
        logFile_.open(filename, std::ios::app);

        // 只有一天执行一次旧日志清理，清理一周前的日志，防止频繁扫描
        if (shouldDeleteOldLogs()) {
            deleteOldLogs(7);
        }
    }
}

void Logger::writeThread() {
    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        condVar_.wait(lock, [&] { return !logQueue_.empty() || !running_; });

        while (!logQueue_.empty()) {
            rotateLogIfNeeded();
            if (logFile_.is_open()) {
                logFile_ << logQueue_.front() << "\n";
                ++currentLineCount_;
            }
            logQueue_.pop();
        }

        if (logFile_.is_open())
            logFile_.flush();
    }
}

LogLevel Logger::parseLogLevel(const std::string& levelStr) {
    if (levelStr == "DEBUG") return LogLevel::DEBUG;
    if (levelStr == "INFO") return LogLevel::INFO;
    if (levelStr == "WARN") return LogLevel::WARN;
    if (levelStr == "ERROR") return LogLevel::ERROR;
    return LogLevel::INFO; // 默认值
}
