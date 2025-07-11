#include "Session.h"
#include "Dispatcher.h"
#include "SessionManager.h"
#include "Logger.h"  // 替换 std::cerr/std::cout 用

Session::Session(asio::ip::tcp::socket socket, std::shared_ptr<Dispatcher> dispatcher)
    : socket_(std::move(socket)), dispatcher_(std::move(dispatcher)) {}

void Session::start() {
    Logger::instance().log(LogLevel::INFO, "Session started from IP: " + getClientIp());
    doRead();
}

void Session::doRead() {
    auto self = shared_from_this();
    asio::async_read_until(socket_, buffer_, '\n',
        [this, self](std::error_code ec, [[maybe_unused]] std::size_t length) {
            if (!ec) {
                std::istream is(&buffer_);
                std::string line;
                std::getline(is, line);
                if (!line.empty()) {
                    Logger::instance().log(LogLevel::DEBUG, "Received message from " + getClientIp() + ": " + line);
                    dispatcher_->handleRequest(self, line);
                }
                doRead();  // continue reading
            } else {
                Logger::instance().log(LogLevel::WARN, "Session disconnected from " + getClientIp() + ": " + ec.message());
                SessionManager::instance().removeSession(self);
            }
        });
}

void Session::send(const std::string& message) {
    std::lock_guard<std::mutex> lock(writeMutex_);
    bool writeIdle = writeQueue_.empty();
    writeQueue_.push_back(message + "\n");
    if (writeIdle && !writing_) {
        doWrite();
    }
}

void Session::doWrite() {
    auto self = shared_from_this();
    writing_ = true;

    asio::async_write(socket_, asio::buffer(writeQueue_.front()),
        [this, self](std::error_code ec, std::size_t /*length*/) {
            std::lock_guard<std::mutex> lock(writeMutex_);
            if (!ec) {
                Logger::instance().log(LogLevel::DEBUG, "Sent message to " + getClientIp() + ": " + writeQueue_.front());
                writeQueue_.pop_front();
                if (!writeQueue_.empty()) {
                    doWrite();
                } else {
                    writing_ = false;
                }
            } else {
                Logger::instance().log(LogLevel::ERROR, "Write error to " + getClientIp() + ": " + ec.message());
                writing_ = false;
                SessionManager::instance().removeSession(self);
            }
        });
}

void Session::close() {
    asio::post(socket_.get_executor(), [self = shared_from_this()]() {
        std::error_code ec;
        self->socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        self->socket_.close(ec);
        Logger::instance().log(LogLevel::INFO, "Closed session for IP: " + self->getClientIp());
        SessionManager::instance().removeSession(self);
    });
}

std::string Session::getClientIp() const {
    try {
        return socket_.remote_endpoint().address().to_string();
    } catch (...) {
        return "unknown";
    }
}
