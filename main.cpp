#include "Server.h"
#include <iostream>

int main() {
    try {
        auto server = Server::create("../../config/config.json");
        server->start();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }
}
