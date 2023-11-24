#include "../includes/Buffer.hpp"
#include "../includes/utils/Containers.hpp"

BufferMap Buffer::_bufferForRead;
BufferMap Buffer::_bufferForWrite;

void Buffer::resetReadBuffer(int fd) {
    if (_bufferForRead.find(fd) != _bufferForRead.end()) {
        _bufferForRead[fd] = "";
    } else {
        _bufferForRead.insert(std::make_pair(fd, ""));
    }
}

void Buffer::resetWriteBuffer(int fd) {
    if (_bufferForWrite.find(fd) != _bufferForWrite.end()) {
        _bufferForWrite[fd] = "";
    } else {
        _bufferForWrite.insert(std::make_pair(fd, ""));
    }
}

void Buffer::eraseReadBuffer(int fd) {
    if (_bufferForRead.find(fd) != _bufferForRead.end()) {
        _bufferForRead.erase(fd);
    }
}

void Buffer::eraseWriteBuffer(int fd) {
    if (_bufferForWrite.find(fd) != _bufferForWrite.end()) {
        _bufferForWrite.erase(fd);
    }
}

std::string Buffer::getReadBuffer(int fd) {
    if (_bufferForRead.find(fd) != _bufferForRead.end()) {
        return _bufferForRead[fd];
    } else {
        return "";
    }
}

std::string Buffer::getWriteBuffer(int fd) {
    if (_bufferForWrite.find(fd) != _bufferForWrite.end()) {
        return _bufferForWrite[fd];
    } else {
        return "";
    }
}

void Buffer::setReadBuffer(std::pair<int, std::string> val) {
    if (_bufferForRead.find(val.first) != _bufferForRead.end()) {
        _bufferForRead[val.first] += val.second;
    } else {
        _bufferForRead.insert(val);
    }
}

void Buffer::setWriteBuffer(std::pair<int, std::string> val) {
    if (_bufferForWrite.find(val.first) != _bufferForWrite.end()) {
        _bufferForWrite[val.first] += val.second;
    } else {
        _bufferForWrite.insert(val);
    }
}

int Buffer::readMessage(int fd, intptr_t data) {
    char buffer[data + 1];
    int byte;

    memset(buffer, 0, sizeof(buffer));
    byte = recv(fd, buffer, data, 0);
    if (byte <= 0) {
        return byte;
    }
    _bufferForRead[fd] += buffer;
    return byte;
}

int Buffer::sendMessage(int fd) {
    int byte;

    std::string message = _bufferForWrite[fd];
    byte = send(fd, message.c_str(), message.length(), 0);
    if (byte == -1) {
        return -1;
    }
    if (static_cast<size_t>(byte) < message.length())
        _bufferForWrite[fd] = _bufferForWrite[fd].substr(byte);
    else
        _bufferForWrite[fd] = "";
    return byte;
}

int Buffer::sendMessage(int fd, std::string message) {
    int byte;

    byte = send(fd, message.c_str(), message.length(), 0);
    if (byte == -1) {
        return -1;
    }
    if (static_cast<size_t>(byte) < message.length())
        _bufferForWrite[fd] = _bufferForWrite[fd].substr(byte);
    else
        _bufferForWrite[fd] = "";
    return byte;
}