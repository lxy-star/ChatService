#pragma once
#include <asio.hpp>
#include <memory>
#include <string>

class Dispatcher;

class Server
{
public:
    static std::shared_ptr<Server> create(const std::string &configPath);

    void start();

    Server(asio::io_context &context,
           const std::string &ip,
           uint16_t port,
           std::shared_ptr<Dispatcher> dispatcher);

private:
    void doAccept();

    asio::io_context &ioContext_;
    asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<Dispatcher> dispatcher_;
};
