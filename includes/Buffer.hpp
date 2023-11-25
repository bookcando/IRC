#ifndef BUFFER_HPP
#define BUFFER_HPP

#include "./utils/Headers.hpp"
#include "./utils/Containers.hpp"

class Buffer {
private:
    static BufferMap _bufferForRead;
    static BufferMap _bufferForWrite;
public:
    static void resetReadBuffer(int fd);
    static void resetWriteBuffer(int fd);
    static void eraseReadBuffer(int fd);
    static void eraseWriteBuffer(int fd);
    static std::string getReadBuffer(int fd);
    static std::string getWriteBuffer(int fd);
    static int readMessage(int fd, intptr_t data);
    static int sendMessage(int fd);
    static int sendMessage(int fd, std::string message);
    static void setReadBuffer(std::pair<int, std::string> val);
    static void setWriteBuffer(std::pair<int, std::string> val);
};

#endif