#include "../../includes/Command.hpp"
#include "../../includes/Buffer.hpp"
#include "../../includes/Message.hpp"
#include "../../includes/utils/Error.hpp"
#include "../../includes/utils/reply.hpp"
#include "../../includes/Lists.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Channel.hpp"
#include <sstream>

// chkNum 함수: 문자열이 숫자로만 이루어져 있는지 확인
static bool chkNum(std::string const& string) {
    for (size_t i = 0; i < string.size(); i++)
        if (string[i] < '0' || string[i] > '9')
            return false;
    return true;
}

// findNick 함수: 클라이언트 목록에서 특정 닉네임을 찾아 해당 클라이언트의 파일 디스크립터를 반환
static int findNick(ClientMap const& clientList, std::string const& tgt) {
    for (ClientMap::const_iterator it = clientList.begin(); it != clientList.end(); it++) {
        if (it->second->getNickname() == tgt)
            return it->first;
    }
    return 0;  // 닉네임을 찾지 못한 경우 0 반환
}

// createSetMode 함수: 채널의 현재 모드를 문자열로 변환하여 출력용 변수에 저장
static void createSetMode(int mode, Channel& channel, std::string& outputMode, std::string& outputValue) {
    std::ostringstream oss;

    outputMode += "+";  // 모든 모드는 추가되는 것으로 시작
    for (int i = 0; i < 5; i++) {
        if (mode & (1 << i)) {  // 현재 비트가 설정되어 있는지 확인
            switch (1 << i) {
                case USER_LIMIT_PER_CHANNEL:
                    oss << channel.getUserLimit();  // 유저 제한 모드일 경우 값을 문자열로 변환
                    outputMode += "l";
                    outputValue += oss.str() + " ";  // 변환된 값을 출력용 변수에 추가
                    oss.clear();  // 문자열 스트림 초기화
                    break;
                case INVITE_CHANNEL:
                    outputMode += "i";
                    break;
                case KEY_CHANNEL:
                    outputMode += "k";
                    outputValue += channel.getKey() + " ";  // 채널 키 값을 출력용 변수에 추가
                    break;
                case SAFE_TOPIC:
                    outputMode += "t";
                    break;
                case SET_CHANOP:
                    outputMode += "o";
                    outputValue += channel.getChannelOperator()->getNickname() + " ";  // 채널 오퍼레이터의 닉네임을 출력용 변수에 추가
                    break;
            }
        }
    }
    if (outputValue != "" && outputValue[outputValue.size() - 1] == ' ') {
        outputValue = outputValue.substr(0, outputValue.size() - 1);  // 마지막 공백 제거
    }
}

void Command::mode(Client& client, std::string const& serverHost) {
    messageVector const& message = Message::getMessage();  // 현재 메시지를 가져옴
    ChannelMap::iterator it;  // 채널 맵의 이터레이터
    std::string successMode = "";  // 모드 변경 성공 여부를 나타내는 문자열
    std::string successValue = "";  // 모드 변경 값
    std::string supportMode = "itkol";  // 지원하는 모드 문자열
    std::string last = "";  // 마지막 메시지에 대한 특수 처리 문자열
    std::ostringstream oss;  // 문자열을 빌드하기 위한 스트림
    bool flag = true;  // 모드가 '+' 인지 '-' 인지 나타내는 플래그
    int set[5];  // 모드에 대한 비트 플래그를 저장하는 배열
    size_t val = 3;  // 메시지 파라미터의 시작 위치
    int fd;  // 파일 디스크립터
    ChannelMap& chlList = Lists::getChannelList();  // 채널 목록을 가져옴

    // 채널이 존재하고 메시지 파라미터가 2개인 경우
    if (message.size() == 2 && (it = chlList.find(message[1])) != chlList.end()) {
        oss << it->second->getTime();  // 채널 생성 시간을 문자열로 변환
        createSetMode(it->second->getMode(), *it->second, successMode, successValue);  // 채널 모드를 문자열로 변환
        Buffer::sendMessage(client.getClientFd(), reply::RPL_CHANNELMODEIS(serverHost, client.getNickname(), message[1], successMode, successValue));  // 모드 정보를 클라이언트에게 전송
        Buffer::sendMessage(client.getClientFd(), reply::RPL_CREATIONTIME(serverHost, client.getNickname(), message[1], oss.str()));  // 채널 생성 시간을 클라이언트에게 전송
    }
    // 메시지 파라미터가 3개 미만인 경우
    else if (message.size() < 3)
        Buffer::sendMessage(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "MODE"));
    // 채널이 존재하지 않는 경우
    else if ((it = chlList.find(message[1])) == chlList.end())
        Buffer::sendMessage(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), message[1]));
    // 채널 오퍼레이터가 아닌 경우
    else if ((chlList[message[1]]->getChannelOperator() == 0) || (chlList[message[1]]->getChannelOperator()->getClientFd() != client.getClientFd())) {
        std::cout << "Client is not channel operator||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||" << std::endl;
        Buffer::sendMessage(client.getClientFd(), Error::ERR_CHANOPRIVSNEEDED(serverHost, client.getNickname(), message[1]));
    }
    else {
        // 메시지의 세 번째 파라미터부터 반복
        for (size_t i = 0; i < message[2].size(); i++) {
            if (val == message.size() - 1)
                last = ":";  // 마지막 메시지 처리를 위한 설정
            // 모드 변경이나 해제를 나타내는 기호인 경우
            if (message[2][i] == '+' || message[2][i] == '-') {
                // '+' 인 경우
                if (message[2][i] == '+') {
                    successMode += "+";
                    flag = true;
                    for (int j = 0; j < 5; j++)
                        set[j] = (1 << j);  // 해당 비트를 1로 설정
                }
                // '-' 인 경우
                else {
                    successMode += "-";
                    flag = false;
                    for (int k = 0; k < 5; k++)
                        set[k] = ~(1 << k);  // 해당 비트를 0으로 설정
                }
                continue;
            }
            // 지원하는 모드 문자열에 포함된 경우
            if (supportMode.find(message[2][i]) != std::string::npos) {
                switch (message[2][i]) {
                    // 유저 제한 모드
                    case 'l':
                        // '+' 인 경우 파라미터가 부족하거나 숫자가 아닌 경우 에러 전송
                        if (flag == true && (val >= message.size() || !chkNum(message[val])))
                            Buffer::sendMessage(client.getClientFd(),
                                Error::ERR_INVALIDMODEPARAM(serverHost, client.getNickname(), message[1], message[2][i], "You must specify a parameter. Syntax: <limit>"));
                        else {
                            successMode += "l";
                            it->second->setMode(set[0], flag);
                            if (flag == true) {
                                successValue += last + message[val] + " ";
                                it->second->setUserLimit(atoi(message[val].c_str()));
                                val++;
                            }
                            else
                                it->second->setUserLimit(0);
                        }
                        break;
                    // 초대 모드
                    case 'i':
                        successMode += "i";
                        it->second->setMode(set[1], flag);
                        break;
                    // 채널 키 모드
                    case 'k':
                        // 파라미터가 부족하거나 빈 문자열인 경우 에러 전송
                        if (val >= message.size() || message[val] == "")
                            Buffer::sendMessage(client.getClientFd(),
                                Error::ERR_INVALIDMODEPARAM(serverHost, client.getNickname(), message[1], message[2][i], "You must specify a parameter. Syntax: <key>"));
                        // '-' 인 경우 키가 일치하지 않으면 에러 전송
                        else if ((flag == false && message[val] != it->second->getKey()))
                            Buffer::sendMessage(client.getClientFd(),
                                Error::ERR_INVALIDMODEPARAM(serverHost, client.getNickname(), it->second->getChannelName(), 'k', "You must specify a parameter for the key mode. Syntax: <key>."));
                        else {
                            successMode += "k";
                            it->second->setMode(set[2], flag);
                            if (flag == true) {
                                successValue += last + message[val] + " ";
                                it->second->setKey(message[val]);
                            }
                            else
                                it->second->setKey("");
                        }
                        val++;
                        break;
                    // 안전한 토픽 모드
                    case 't':
                        successMode += "t";
                        it->second->setMode(set[3], flag);
                        break;
                    // 채널 오퍼레이터 모드
                    case 'o':
                        // '+' 인 경우 파라미터가 부족하거나 빈 문자열인 경우 에러 전송
                        if (flag == true && (val >= message.size() || message[val] == ""))
                            Buffer::sendMessage(client.getClientFd(),
                                Error::ERR_INVALIDMODEPARAM(serverHost, client.getNickname(), message[1], message[2][i], "You must specify a parameter. Syntax: <nick>"));
                        // '+' 인 경우 지정된 닉네임을 찾을 수 없는 경우 에러 전송
                        else if (flag == true && !(fd = findNick(it->second->getUserList(), message[val])))
                            Buffer::sendMessage(client.getClientFd(), Error::ERR_NOSUCHNICK(serverHost, message[val]));
                        else {
                            successMode += "o";
                            it->second->setMode(set[4], flag);
                            if (flag == true) {
                                successValue += last + message[val] + " ";
                                it->second->setChannelOperator(it->second->getUserList().find(fd)->second);
                                val++;
                            }
                        }
                        break;
                }
            } else {
                // 지원하지 않는 모드인 경우 에러 전송
                Buffer::sendMessage(client.getClientFd(), Error::ERR_UNKNOWNMODE(serverHost, client.getNickname(), message[2][i]));
            }
        }
    }
    // 모드 변경이 성공하고 메시지 파라미터가 2개가 아닌 경우
    if (message.size() != 2 && successMode != "") {
        ClientMap const& userList = it->second->getUserList();
        // 성공한 값이 존재하고 마지막 값이 공백인 경우 공백 제거
        if (successValue != "" && successValue[successValue.size() - 1] == ' ')
            successValue = successValue.substr(0, successValue.size() - 1);
        // 모든 사용자에게 성공한 모드 정보 전송
        for (ClientMap::const_iterator iter = userList.begin(); iter != userList.end(); iter++)
            Buffer::sendMessage(iter->second->getClientFd(), reply::RPL_SUCCESSMODE(client.getNickname(), client.getUsername(), client.getHost(), message[1], successMode, successValue));
    }
}
