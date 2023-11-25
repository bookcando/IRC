#include "../includes/Validator.hpp"
#include "../includes/Buffer.hpp"

long long Validator::validatePort(const std::string& port) {
    long long portNum = std::strtoll(port.c_str(), NULL, 10);
    if (portNum < 1024 || portNum > 65535) {
        throw std::runtime_error("ERROR: Invalid port number");
    }
    return portNum;
}

void Validator::validatePassword(const std::string& password) {
    if (password.length() < 8) {
        throw std::runtime_error("ERROR: Password must be at least 8 characters long");
    }
    if (password.find(" ") != std::string::npos) {
        throw std::runtime_error("ERROR: Password cannot contain spaces");
    }
    if (password.find("\r") != std::string::npos || password.find("\n") != std::string::npos) {
        throw std::runtime_error("ERROR: Password cannot contain newlines");
    }
}

bool Validator::validateMessage(const int fd, const intptr_t data) {
    int byte = 0;

    byte = Buffer::readMessage(fd, data);

    if (byte <= 0) {
        return false;
    }
    return true;
}