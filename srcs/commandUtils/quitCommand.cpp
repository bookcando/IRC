#include "../../includes/Command.hpp"
#include "../../includes/Buffer.hpp"
#include "../../includes/Message.hpp"
#include "../../includes/utils/reply.hpp"
#include "../../includes/Lists.hpp"
#include "../../includes/Client.hpp"

// CommandExecute 클래스의 quit 함수 정의
void Command::quit(Client& client) {
    // Message 클래스에서 메세지 벡터를 가져옴
    messageVector const& message = Message::getMessage();

    // List 클래스에서 클라이언트 목록을 가져옴
    ClientMap const& clientList = Lists::getClientList();

    // 퇴장 사유를 저장할 변수 초기화
    std::string reason = "";

    // 메세지 벡터의 두 번째 이후의 모든 단어를 퇴장 사유로 결합
    for (size_t i = 1; i < message.size(); i++)
        reason += message[i] + " ";

    // 퇴장 사유 문자열의 마지막에 공백이 있다면 삭제
    if (reason != "" && reason[reason.size() - 1] == ' ')
        reason = reason.substr(0, reason.size() - 1);

    // 클라이언트 목록을 순회하며 메세지 전송
    for (ClientMap::const_iterator it = clientList.begin(); it != clientList.end(); it++) {
        // 현재 클라이언트가 아닌 경우 성공적인 퇴장 메세지 전송
        if (it->second != &client)
            Buffer::sendMessage(it->second->getClientFd(), reply::RPL_SUCCESSQUIT(client.getNickname(), client.getUsername(), client.getHost(), reason));
        // 현재 클라이언트인 경우 에러 메세지 전송
        else
            Buffer::sendMessage(client.getClientFd(), "ERROR :Connection out");
    }
}
