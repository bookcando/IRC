#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "./utils/Headers.hpp"

class Massage {
private:
    Massage();

public:
    ~Massage();
    static bool parseMessage(std::string message);
};

#endif