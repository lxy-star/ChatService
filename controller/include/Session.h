#pragma once
#include <asio.hpp>
#include <memory>
#include <deque>
#include <mutex>
#include <string>

class Dispatcher;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(asio::ip::tcp::socket socket, std::shared_ptr<Dispatcher> dispatcher);

    void start();
    void send(const std::string& message);

    void setUsername(const std::string& name) { username_ = name; }
    std::string getUsername() const { return username_; }
    void close();
    std::string getClientIp() const;

private:
    void doRead();
    void doWrite();

    asio::ip::tcp::socket socket_;
    asio::streambuf buffer_;
    std::shared_ptr<Dispatcher> dispatcher_;

    std::string username_;

    std::deque<std::string> writeQueue_;
    std::mutex writeMutex_;
    bool writing_ = false;
};
