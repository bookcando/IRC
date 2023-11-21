#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Usage: ./ircserv [port] [password]" << std::endl;
        return 1;
    }
    try {
        Server server(argv[1], argv[2]);
        std::cout << "Check 1" << std::endl;
        server.initializeServer();
        std::cout << "Check 2" << std::endl;
        server.runServer();
        std::cout << "Check 3" << std::endl;
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}