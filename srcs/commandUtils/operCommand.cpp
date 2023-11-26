#include "../../includes/Command.hpp"
#include "../../includes/Buffer.hpp"
#include "../../includes/utils/reply.hpp"
#include "../../includes/utils/Error.hpp"
#include "../../includes/Lists.hpp"
#include "../../includes/utils/Containers.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/utils/Headers.hpp"
#include "../../includes/Message.hpp"

// findNick 함수 정의: 클라이언트 리스트에서 닉네임을 찾아 해당하는 클라이언트의 소켓 파일 디스크립터를 반환하는 함수
static int findNick(ClientMap const& cltList, std::string const& targetName) {
    // 클라이언트 리스트를 순회하며 닉네임을 찾음
    for (ClientMap::const_iterator it = cltList.begin(); it !=  cltList.end(); it++) {
        // 클라이언트의 닉네임이 목표 닉네임과 일치하는 경우 해당 클라이언트의 소켓 파일 디스크립터 반환
        if (it->second->getNickname() == targetName)
            return it->first;
    }
    // 일치하는 닉네임을 찾지 못한 경우 0 반환
    return 0;
}

// oper 함수 정의: 클라이언트를 오퍼레이터로 등록하는 함수
bool Command::oper(Client& client, std::string const& operatorName, std::string const& operatorPass, std::string const& serverHost, bool oper) {
    messageVector const& message = Message::getMessage();
    bool flag = false;

    // 메세지의 길이가 올바르지 않은 경우 에러 메세지 전송
    if (message.size() != 3)
        Buffer::sendMessage(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "OPER"));
    // 오퍼레이터 이름이 일치하지 않는 경우 에러 메세지 전송
    else if (message[1] != operatorName)
        Buffer::sendMessage(client.getClientFd(), Error::ERR_NOOPERHOST(serverHost, client.getNickname(), "you don't match operName"));
    // 비밀번호가 일치하지 않는 경우 에러 메세지 전송
    else if (message[2] != operatorPass)
        Buffer::sendMessage(client.getClientFd(), Error::ERR_PASSWDMISMATCH(serverHost));
    // 이미 오퍼레이터인 경우 에러 메세지 전송
    else if (oper)
        Buffer::sendMessage(client.getClientFd(), Error::ERR_ALREADYOTHEROPER(serverHost, client.getNickname()));
    else {
        // 오퍼레이터로 등록되었음을 클라이언트에게 알림
        Buffer::sendMessage(client.getClientFd(), reply::RPL_YOUREOPER(serverHost, client.getNickname()));
        flag = true;
    }
    return flag;
}

// kill 함수 정의: 클라이언트를 강제로 종료하는 함수
int Command::kill(Client& client, std::string const& serverHost, bool op) {
    messageVector const& message = Message::getMessage();
    std::string reason;
    int fd = 0;

    // 메세지의 길이가 올바르지 않은 경우 에러 메세지 전송
    if (message.size() != 3)
        Buffer::sendMessage(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "KILL"));
    // 해당 닉네임을 가진 클라이언트가 존재하지 않는 경우 에러 메세지 전송
    else if (!(fd = findNick(Lists::getClientList(), message[1])))
        Buffer::sendMessage(client.getClientFd(), Error::ERR_NOSUCHNICK(serverHost, message[1]));
    // 오퍼레이터 권한이 없는 경우 에러 메세지 전송
    else if (!op)
        Buffer::sendMessage(client.getClientFd(), Error::ERR_NOOPIVILEGES(serverHost, client.getNickname()));
    
    // 종료 사유 설정 및 메세지 파싱
    reason = "QUIT :" + message[2] + "\r\n";
    Message::parseMessage(reason);
    
    // 클라이언트 소켓 파일 디스크립터 반환
    return (fd);
}
