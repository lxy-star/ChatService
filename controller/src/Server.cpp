#include "Server.h"
#include "Session.h"
#include "Dispatcher.h"
#include "UserManager.h"
#include "MySQLClient.h"
#include "ConfigLoader.h"
#include <iostream>
#include "RedisClient.h"
#include "ChatManager.h"

Server::Server(asio::io_context &context,
               const std::string &ip,
               uint16_t port,
               std::shared_ptr<Dispatcher> dispatcher)
    : ioContext_(context),
      acceptor_(context, asio::ip::tcp::endpoint(asio::ip::make_address(ip), port)),
      dispatcher_(std::move(dispatcher)) {}

std::shared_ptr<Server> Server::create(const std::string &configPath)
{
    if (!ConfigLoader::load(configPath))
    {
        throw std::runtime_error("Failed to load config file");
    }

    std::string ip = ConfigLoader::getString("server_ip");
    uint16_t port = ConfigLoader::getInt("server_port");

    std::string mysqlHost = ConfigLoader::getString("mysql_host");
    std::string mysqlUser = ConfigLoader::getString("mysql_user");
    std::string mysqlPass = ConfigLoader::getString("mysql_password");
    std::string mysqlDb = ConfigLoader::getString("mysql_db");

    std::string redisHost = ConfigLoader::getString("redis_host");  // 新增
    int redisPort = ConfigLoader::getInt("redis_port");             // 新增

    static asio::io_context ioContext;

    auto mysqlClient = std::make_shared<MySQLClient>(mysqlHost, mysqlUser, mysqlPass, mysqlDb);
    auto redisClient = std::make_shared<RedisClient>(redisHost, redisPort);
    auto userManager = std::make_shared<UserManager>(mysqlClient, redisClient);
    auto chatManager = std::make_shared<ChatManager>(mysqlClient);
    auto dispatcher = std::make_shared<Dispatcher>(userManager, chatManager);

    return std::make_shared<Server>(ioContext, ip, port, dispatcher);
}


void Server::start()
{
    std::cout << "Chat server listening..." << std::endl;
    doAccept();
    ioContext_.run();
}

void Server::doAccept()
{
    acceptor_.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket)
                           {
        if (!ec) {
            auto session = std::make_shared<Session>(std::move(socket), dispatcher_);
            session->start();
        }
        doAccept(); });
}
