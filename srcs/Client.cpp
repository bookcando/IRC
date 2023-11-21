#include "../includes/utils/Headers.hpp"
#include "../includes/utils/Containers.hpp"
#include "../includes/Client.hpp"

Client::Client(int clientFd) : _clientFd(clientFd), _nickname(""), _username(""), _realname(""), _recvBuffer(""), _sendBuffer("") {
}

Client::~Client() {
}

void Client::resetReadBuffer() {
    _recvBuffer.clear();
}

void Client::resetWriteBuffer() {
    _sendBuffer.clear();
}

std::string Client::getRecvBuffer() {
    return _recvBuffer;
}

std::string Client::getSendBuffer() {
    return _sendBuffer;
}

void Client::setRecvBuffer(std::string buffer) {
    _recvBuffer = buffer;
}

void Client::setSendBuffer(std::string buffer) {
    _sendBuffer = buffer;
}

int Client::getClientFd() {
    return _clientFd;
}