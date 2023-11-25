#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "./utils/Headers.hpp"
#include "./utils/Containers.hpp"

class Message {
private:
    Message();
    static messageVector _commandElements;

public:
    ~Message();
    static bool parseMessage(std::string message);
    static void getMessage(int fd, std::string buffer, std::string host);
};

#endif