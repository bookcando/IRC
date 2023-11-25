#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include "./utils/Headers.hpp"

class Validator {
public:
    static long long validatePort(const std::string& port);
    static void validatePassword(const std::string& password);
    static bool validateMessage(const int fd, const intptr_t data);
};

#endif