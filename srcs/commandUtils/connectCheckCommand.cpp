#include "../../includes/Command.hpp"
#include "../../includes/Message.hpp"
#include "../../includes/Buffer.hpp"
#include "../../includes/utils/reply.hpp"
#include "../../includes/utils/Error.hpp"
#include "../../includes/Lists.hpp"
#include "../../includes/utils/Containers.hpp"
#include "../../includes/Client.hpp"

void Command::ping(Client& client, std::string const& serverHost) {
    messageVector const& message = Message::getMessage();

    if (message.size() == 1) {
        Error::ERR_NOORIGIN(serverHost, client.getNickname());
    }
    else if (message.size() != 2) {
        Error::ERR_NEEDMOREPARAMS(serverHost, message[0]);
    }
    else {
        Command::pong(client, serverHost, message[1]);
    }
}

void Command::pong(Client& client, std::string const& serverHost, std::string const& token) {
    Buffer::saveMessageToBuffer(client.getClientFd(), ":" + serverHost + " PONG " + serverHost + " :" + token + "\r\n");
}
