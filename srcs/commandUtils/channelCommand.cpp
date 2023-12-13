#include "../../includes/Command.hpp"
#include "../../includes/Message.hpp"
#include "../../includes/Buffer.hpp"
#include "../../includes/utils/reply.hpp"
#include "../../includes/utils/Error.hpp"
#include "../../includes/Lists.hpp"
#include "../../includes/utils/Headers.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Channel.hpp"

static bool isChanName(std::string const& chanName) {

    if (chanName.find(',') != std::string::npos)
        return false;
    if (chanName.find(' ') != std::string::npos)
        return false;
    if (chanName.find(7) != std::string::npos)
        return false;
    // Channel 이름에 들어가면 안 되는 문자 실패로 제거
    if (chanName.size() < 2)
        return false;
    if (chanName[0] == '#')
        return true;
    return false;
}

// 특정 닉네임을 가진 클라이언트를 찾는 함수
static int findNick(ClientMap const& clientList, std::string const& tgt) {
    // 클라이언트 목록을 순회하며 닉네임이 일치하는 클라이언트를 찾습니다.
    for (ClientMap::const_iterator it = clientList.begin(); it !=  clientList.end(); it++) {
        if (it->second->getNickname() == tgt)
            return it->first; // 일치하는 클라이언트를 찾으면 해당 클라이언트의 파일 기술자를 반환합니다.
    }
    return 0; // 일치하는 클라이언트가 없으면 0을 반환합니다.
}

// JOIN 명령어 처리 함수
void Command::join(Client& client, std::string const& serverHost) {
    // JOIN 명령어의 파라미터를 처리하기 위한 변수들을 초기화합니다.
    std::istringstream chan;
    std::istringstream key;
    std::string chanStr = "";
    std::string keyStr = "";
    messageVector const& message = Message::getMessage();
    Channel* channel;

    // JOIN 명령어의 유효성 검사를 수행합니다.
    if (message.size() < 2 || message.size() > 3)
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "JOIN"));
    else {
        // 채널 이름과 키를 파싱합니다.
        chan.str(message[1]);
        if (message.size() == 3)
            key.str(message[2]);
        // 각 채널에 대해 처리를 수행합니다.
        while (std::getline(chan, chanStr, ',')) {
            if (message.size() == 3)
                std::getline(key, keyStr, ',');
            // 채널 이름의 유효성을 검사합니다.
            if (!isChanName(chanStr) || chanStr.size() > 200) {
                Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_BADCHANMASK(serverHost, client.getNickname(), chanStr));
                continue;
            }
            // 채널이 존재하지 않으면 새로 생성합니다.
            if (!Lists::hasChannel(chanStr))
                Lists::addChannelList(chanStr, &client);
            else {
                // 채널에 아무도 없으면 채널을 삭제하고 새로 생성합니다.
                if (Lists::findChannel(chanStr).getUserList().size() == 0) {
                    Lists::deleteChannelList(chanStr);
                    Lists::addChannelList(chanStr, &client);
                }
            }
            // 채널 객체를 가져와서 클라이언트의 가입 요청을 처리합니다.
            channel = &Lists::findChannel(chanStr);
            switch (client.joinChannel(&Lists::findChannel(chanStr), keyStr)) {
                // 가입 실패 상황에 따른 오류 메시지를 전송합니다.
                case TOOMANYCHANNELS:
                    Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_TOOMANYCHANNELS(serverHost, client.getNickname(), chanStr));
                    break;
                case CHANNELISFULL:
                    Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_CHANNELISFULL(serverHost, client.getNickname(), chanStr));
                    break;
                case INVITEONLYCHAN:
                    Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_INVITEONLYCHAN(serverHost, client.getNickname(), chanStr));
                    break;
                case BADCHANNELKEY:
                    Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_BADCHANNELKEY(serverHost, client.getNickname(), chanStr));
                    break;
                // 가입 성공 시 관련 메시지를 전송합니다.
                case IS_SUCCESS:
                    channel->deleteInviteList(&client);
                    Buffer::saveMessageToBuffer(client.getClientFd(), reply::RPL_SUCCESSJOIN(client.getNickname(), client.getUsername(), client.getHost(), chanStr));
                    if (channel->getTopic() != "")
                        Buffer::saveMessageToBuffer(client.getClientFd(), reply::RPL_TOPIC(serverHost, client.getNickname(), chanStr, channel->getTopic()));
                    Buffer::saveMessageToBuffer(client.getClientFd(), reply::RPL_NAMREPLY(serverHost, client.getNickname(), chanStr, channel->getStrUserList()));
                    Buffer::saveMessageToBuffer(client.getClientFd(), reply::RPL_ENDOFNAMES(serverHost, client.getNickname(), chanStr));
                    // 다른 채널 사용자에게도 가입 사실을 알립니다.
                    for (ClientMap::const_iterator iter = channel->getUserList().begin(); iter != channel->getUserList().end(); iter++)
                        if (iter->second != &client)
                            Buffer::saveMessageToBuffer(iter->second->getClientFd(), reply::RPL_SUCCESSJOIN(client.getNickname(), client.getUsername(), client.getHost(), chanStr));
                    break;
            }
            chanStr = "";
            keyStr = "";
        }
    }
}
// PART 명령어 처리 함수
void Command::part(Client& client, std::string const& serverHost) {
    // PART 명령어의 파라미터를 처리하기 위한 변수들을 초기화합니다.
	messageVector const& message = Message::getMessage();
    std::string chanStr = "";
    std::string reason = "";
    std::istringstream str;
    ClientMap userList;
    Channel* chan;
    ClientMap::const_iterator it;

    // PART 명령어의 유효성 검사를 수행합니다.
    if (message.size() < 2 || message.size() > 3)
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "PART"));
    else {
        // 각 채널에 대해 처리를 수행합니다.
        str.str(message[1]);
        while (std::getline(str, chanStr, ',')) {
            // 채널이 존재하는지 확인합니다.
            if (Lists::hasChannel(chanStr))
                chan = &Lists::findChannel(chanStr);
            else
                chan = NULL;
            // 채널이 없거나 사용자가 채널에 속하지 않은 경우 오류 메시지를 전송합니다.
            if (chan == NULL)
                Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), chanStr));
            else if (chan->getUserList().find(client.getClientFd()) == chan->getUserList().end())
                Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOTONCHANNEL(serverHost, client.getNickname(), chan->getChannelName()));
            else {
                // 채널에서 사용자를 제거하고 관련 메시지를 전송합니다.
                userList = chan->getUserList();
                for (it = userList.begin(); it != userList.end(); it++) {
                    if (message.size() == 3) // 이유가 제공된 경우 메시지에 추가합니다.
                        Buffer::saveMessageToBuffer(it->second->getClientFd(), reply::RPL_SUCCESSPART(client.getNickname(), client.getUsername(), client.getHost(), chan->getChannelName(), message[2]));
                    else
                        Buffer::saveMessageToBuffer(it->second->getClientFd(), reply::RPL_SUCCESSPART(client.getNickname(), client.getUsername(), client.getHost(), chan->getChannelName()));
                }
                chan->deleteClientList(&client);
                client.deleteJoinList(chan);
                // 채널에 아무도 남지 않은 경우 채널을 제거합니다.
                if (chan->getUserList().size() == 0)
                    Lists::deleteChannelList(chan->getChannelName());
            }
        }
    }
}

// KICK 명령어 처리 함수
void Command::kick(Client& client, std::string const& serverHost) {
    // KICK 명령어의 파라미터를 처리하기 위한 변수들을 초기화합니다.
	messageVector const& message = Message::getMessage();
    std::string reason = "";
    std::string userStr = "";
    std::istringstream str;
    ClientMap::const_iterator it;
    int tgtFd = 0;
    ClientMap userList;
    Channel* chan;

    // KICK 명령어의 유효성 검사를 수행합니다.
    if (message.size() < 3 || message.size() > 4)
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "KICK"));
    else if (!Lists::hasChannel(message[1]))
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), message[1]));
    else if ((chan = &Lists::findChannel(message[1]))->getChannelOperator() == NULL || chan->getChannelOperator()->getClientFd() != client.getClientFd())
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_CHANOPRIVSNEEDED(serverHost, client.getNickname(), message[1]));
    else {
        // 각 사용자에 대해 처리를 수행합니다.
        str.str(message[2]);
        while (std::getline(str, userStr, ',')) {
            userList = chan->getUserList();
            if ((tgtFd = findNick(userList, userStr)) == 0)
                Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_USERNOTINCHANNEL(serverHost, client.getNickname(), userStr, chan->getChannelName()));
            else {
                // 지정된 사용자를 채널에서 제거하고 관련 메시지를 전송합니다.
                for (it = userList.begin(); it != userList.end(); it++) {
                    if (message.size() == 4) // 이유가 제공된 경우 메시지에 추가합니다.
                        Buffer::saveMessageToBuffer(it->second->getClientFd(), reply::RPL_SUCCESSKICK(client.getNickname(), client.getUsername(), client.getHost(), message[1], userStr, message[3]));
                    else
                        Buffer::saveMessageToBuffer(it->second->getClientFd(), reply::RPL_SUCCESSKICK(client.getNickname(), client.getUsername(), client.getHost(), message[1], userStr));
                }
                chan->deleteClientList(userList.find(tgtFd)->second);
            }
        }
    }
}

// INVITE 명령어 처리 함수
void Command::invite(Client& client, std::string const& serverHost) {
    // INVITE 명령어의 파라미터를 처리하기 위한 변수들을 초기화합니다.
	messageVector const& message = Message::getMessage();
    ClientMap userList;
    Channel* chan;
    int tgtFd;

    // INVITE 명령어의 유효성 검사를 수행합니다.
    if (message.size() != 3)
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "INVITE"));
    else if (!Lists::hasChannel(message[2]))
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), message[2]));
    else if ((userList = (chan = &Lists::findChannel(message[2]))->getUserList()).find(client.getClientFd()) == userList.end())
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOTONCHANNEL(serverHost, client.getNickname(), message[2]));
    else if (chan->getMode() & INVITE_CHANNEL
                && (chan->getChannelOperator() == NULL || chan->getChannelOperator()->getClientFd() != client.getClientFd()))
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_CHANOPRIVSNEEDED(serverHost, client.getNickname(), message[2]));
    else if (findNick(userList, message[1]))
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_USERONCHANNEL(serverHost, client.getNickname(), message[1], message[2]));
    else {
        // 초대할 사용자를 찾고, 채널에 초대합니다.
        if (!(tgtFd = findNick(Lists::getClientList(), message[1])))
            return ;
        else {
            chan->addInviteList(&Lists::findClient(tgtFd));
            Buffer::saveMessageToBuffer(client.getClientFd(), reply::RPL_INVITING(serverHost, client.getNickname(), message[2], message[1]));
            Buffer::saveMessageToBuffer(tgtFd, reply::RPL_SUCCESSINVITING(client.getNickname(), client.getUsername(), client.getHost(), message[2], message[1]));
        }
    }
}

// TOPIC 명령어 처리 함수
void Command::topic(Client& client, std::string const& serverHost) {
    messageVector const& message = Message::getMessage(); // 메시지 벡터를 받아옵니다.
    ClientMap::const_iterator it;
    ClientMap userList;
    Channel* chan;

    // 메시지의 길이에 따라 유효성을 검사합니다.
    if (message.size() < 2 || message.size() > 3)
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "TOPIC"));
    else if (!Lists::hasChannel(message[1])) // 해당 채널이 존재하는지 확인합니다.
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), message[1]));
    else if (message.size() == 2) { // 메시지가 2개인 경우 (채널 토픽 조회)
        chan = &Lists::findChannel(message[1]); // 채널을 찾습니다.
        if (chan->getTopic() == "") // 토픽이 설정되지 않은 경우
            Buffer::saveMessageToBuffer(client.getClientFd(), reply::RPL_NOTOPIC(serverHost, client.getNickname(), message[1]));
        else // 토픽이 있는 경우, 해당 토픽을 클라이언트에게 전송합니다.
            Buffer::saveMessageToBuffer(client.getClientFd(), reply::RPL_TOPIC(serverHost, client.getNickname(), message[1], chan->getTopic()));
    }
    else { // 메시지가 3개인 경우 (채널 토픽 설정)
        chan = &Lists::findChannel(message[1]); // 채널을 찾습니다.

        if ((chan->getMode() & SAFE_TOPIC) && (chan->getChannelOperator() == NULL || chan->getChannelOperator()->getClientFd() != client.getClientFd()))
            Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_CHANOPRIVSNEEDED(serverHost, client.getNickname(), message[1])); // 토픽 설정 권한이 있는지 확인

        // 채널에 가입되어 있는지 확인
        // 채널에 가입되어 있지 않으면 ERROR : NOT ON CHANNEL;
        else if (chan->getUserList().find(client.getClientFd()) == chan->getUserList().end())
            Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOTONCHANNEL(serverHost, client.getNickname(), message[1]));

        else {
            userList = chan->getUserList(); // 채널의 사용자 목록을 가져옵니다.
            chan->setTopic(message[2]); // 토픽을 설정합니다.

            // 채널의 모든 사용자에게 새로운 토픽을 알립니다.
            for (it = userList.begin(); it != userList.end(); it++)
                Buffer::saveMessageToBuffer(it->second->getClientFd(), reply::RPL_SUCCESSTOPIC(client.getNickname(), client.getUsername(), client.getHost(), message[1], message[2]));
        }
    }
}
