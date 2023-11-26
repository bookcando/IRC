#include "../../includes/Command.hpp"
#include "../../includes/Message.hpp"
#include "../../includes/Buffer.hpp"
#include "../../includes/utils/reply.hpp"
#include "../../includes/utils/Error.hpp"
#include "../../includes/Lists.hpp"
#include "../../includes/utils/Containers.hpp"
#include "../../includes/Client.hpp"

// PING 명령어의 처리를 담당하는 함수
void Command::ping(Client& client, std::string const& serverHost) {
    // 메시지 파라미터를 가져옴
    messageVector const& message = Message::getMessage();

    // 메시지 파라미터의 크기가 1이 아니면 오류 메시지 전송
    if (message.size() != 1)
        Error::ERR_NOORIGIN(serverHost, client.getNickname());
    // 메시지 파라미터의 크기가 2가 아니면 추가적인 파라미터 필요 오류 메시지 전송
    else if (message.size() != 2)
        Error::ERR_NEEDMOREPARAMS(serverHost, message[0]);
    else
        // PONG 명령어 호출
        Command::pong(client, serverHost, message[1]);
}

// PONG 명령어의 처리를 담당하는 함수
void Command::pong(Client& client, std::string const& serverHost, std::string const& token) {
    // PONG 메시지를 클라이언트에게 전송
    Buffer::sendMessage(client.getClientFd(), ":" + serverHost + " PONG " + serverHost + " :" + token + "\r\n");
}
