#include "../includes/Message.hpp"
#include "../includes/Buffer.hpp"
#include "../includes/utils/Error.hpp"

Message::Message() {}

Message::~Message() {}

bool Message::parseMessage(std::string originMessage) {
    Message::_commandElements.clear();

    bool first = true;
    std::istringstream str;
    
}

void Message::getMessage(int fd, std::string buffer, std::string host) {
    std::string message;
    size_t size = 0;
    int cut;

    while (1) {
        if ((size = buffer.find("\r\n")) != std::string::npos) {
            cut = size + 2;
        } else if ((size = buffer.find("\r")) != std::string::npos || (size = buffer.find("\n")) != std::string::npos) {
            cut = size + 1;
        } else {
            break;
        }

        message = "";
        message = buffer.substr(0, cut);
        buffer = buffer.substr(cut, buffer.size());
        if (message.size() > 512) {
            Buffer::sendMessage(fd, Error::ERR_INPUTTOOLONG(host));
            continue;
        }
        if (Message::parseMessage(message))
            executeCommand(message);
    }
    Buffer::setReadBuffer(std::make_pair(fd, buffer));
}