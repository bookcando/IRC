#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <string>

class Validator {
public:
    static long long validatePort(const std::string& port);
    static void validatePassword(const std::string& password);
};

#endif