#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Usage: ./ircserv [port] [password]" << std::endl;
        return 1;
    }
    try {
        Server server(argv[1], argv[2]);
        server.initializeServer();
        server.runServer();
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}