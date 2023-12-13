#include "../../includes/Command.hpp"
#include "../../includes/Buffer.hpp"
#include "../../includes/Message.hpp"
#include "../../includes/utils/Error.hpp"
#include "../../includes/utils/reply.hpp"
#include "../../includes/Lists.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Channel.hpp"
#include <iostream>
#include <sstream>

// 클라이언트 목록에서 닉네임을 기반으로 클라이언트 ID를 찾는 함수
static int findNick(ClientMap cltList, std::string const& tgt) {
    for (ClientMap::const_iterator it = cltList.begin(); it != cltList.end(); it++) {
        if (it->second->getNickname() == tgt)
            return it->first;
    }
    return 0;
}

// PRIVMSG 명령어의 구현
void Command::privmsg(Client& client, std::string const& serverHost) {
    // 메시지 파라미터를 가져옴
    messageVector const& message = Message::getMessage();
    int fd;
    std::string tgtStr;
    std::string zipPrefix;
    std::istringstream str;
    size_t chanPrefix;
    Channel* chan;

    // 최소한 하나의 수신자와 전송할 텍스트가 있는지 확인
    if (message.size() == 1)
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NORECIPIENT(serverHost, client.getNickname(), "PRIVMSG"));
    else if (message.size() == 2)
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOTEXTTOSEND(serverHost, client.getNickname()));
    else {
        str.str(message[1]);
        // ','를 구분자로 사용하여 수신자 문자열을 분할
        while (std::getline(str, tgtStr, ',')) {
            zipPrefix = "";
            chanPrefix = 0;

            // 수신자 문자열에 '#'이 포함되어 있으면 채널을 나타냄
            if ((chanPrefix = tgtStr.find('#')) != std::string::npos) {
                zipPrefix = tgtStr.substr(0, chanPrefix);

                // 수신자 문자열이 '@'로 시작하면 제거
                if (zipPrefix.size() == 1 && zipPrefix[0] == '@')
                    tgtStr.substr(1, tgtStr.size());

                // 채널이 존재하지 않으면 에러 메시지 전송
                if (!Lists::hasChannel(tgtStr))
                    Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), tgtStr));
                // 수신자 문자열이 '@'로 시작하면
                else if (zipPrefix[0] == '@') {
                    // 클라이언트가 채널 오퍼레이터가 아니거나 오퍼레이터의 닉네임이 일치하지 않으면 에러 전송
                    if ((chan = &Lists::findChannel(tgtStr))->getChannelOperator() == NULL
                        || chan->getChannelOperator()->getNickname() != tgtStr)
                        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOSUCHNICK(serverHost, client.getNickname()));
                    // 클라이언트가 채널의 멤버가 아니면 에러 전송
                    else if (chan->getUserList().find(client.getClientFd()) == chan->getUserList().end())
                        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_CANNOTSENDTOCHAN(serverHost, client.getNickname(), chan->getChannelName()));
                    else
                        Buffer::saveMessageToBuffer(chan->getChannelOperator()->getClientFd(), message[2]);
                }
                else {
                    // 채널을 찾음
                    chan = &Lists::findChannel(tgtStr);
                    ClientMap::const_iterator iter;
                    ClientMap const& userList = chan->getUserList();
                    // 전송 클라이언트가 채널의 멤버가 아니면 에러 전송
                    if (!findNick(chan->getUserList(), client.getNickname()))
                        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_CANNOTSENDTOCHAN(serverHost, client.getNickname(), chan->getChannelName()));
                    else {
                        // 메시지를 채널의 모든 멤버에게 전송
                        for (iter = userList.begin(); iter != userList.end(); iter++)
                            if (iter->second->getNickname() != client.getNickname())
                                Buffer::saveMessageToBuffer(iter->second->getClientFd(), reply::RPL_SUCCESSPRIVMSG(client.getNickname(), client.getUsername(), client.getHost(), tgtStr, message[2]));
                    }
                }
            }
            else {
                // '#'이 포함되어 있지 않으면 닉네임에 대한 클라이언트 ID를 찾아 메시지를 전송
                if (!(fd = findNick(Lists::getClientList(), tgtStr)))
                    Error::ERR_NOSUCHNICK(serverHost, tgtStr);
                else
                    Buffer::saveMessageToBuffer(fd, reply::RPL_SUCCESSPRIVMSG(client.getNickname(), client.getUsername(), client.getHost(), tgtStr, message[2]));
            }
        }
    }
}
