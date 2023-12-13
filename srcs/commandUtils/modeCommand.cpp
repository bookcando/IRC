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
                    oss << channel.getUserLimit();  // 유저 제한 모드일 경우 값을 문자열로 변환 l 뒤에 나오는 파라미터를 oss에 저장 (user_limit)
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

// static void 

void Command::mode(Client& client, std::string const& serverHost) {
    messageVector const& message = Message::getMessage(); //파싱이 완료된 메세지 라인을 가져옴
    ChannelMap::iterator it;
    std::string successMode = "";
    std::string successValue = "";
    std::string supportMode = "itkol";
    std::string last = "";
    std::ostringstream oss;
    bool flag = true; // 모드가 '+'인지 '-'인지 확인하기 위한 플래그

    int set[5]; // 비트마스킹을 위한 배열
    size_t val = 3; // 메시지 파라미터의 시작 위치 -> 무슨 말이지? 
    // -> MODE <channel name> <mode> <mode param1> <mode param2> <mode param3>... 인 경우, mode params의 첫 번째 위치를 가리킴
    int fd; //
    ChannelMap& tempChannelList = Lists::getChannelList();  // 채널 목록을 가져옴

    // 메세지가 2칸 (mode <channel name>)이고 채널이 존재하는 경우 : <channel name>의 mode를 확인/출력
    if (message.size() == 2 && (it = tempChannelList.find(message[1])) != tempChannelList.end()) {
        oss << it->second->getTime();  // 채널 생성 시간을 문자열로 변환
        createSetMode(it->second->getMode(), *it->second, successMode, successValue); // 채널 모드를 문자열로 변환
        // 모드 정보를 클라이언트에게 전송 + 채널 생성 시간을 클라이언트에게 전송
        Buffer::saveMessageToBuffer(client.getClientFd(), reply::RPL_CHANNELMODEIS(serverHost, client.getNickname(), message[1], successMode, successValue));
        Buffer::saveMessageToBuffer(client.getClientFd(), reply::RPL_CREATIONTIME(serverHost, client.getNickname(), message[1], oss.str()));
    }
    // 메세지가 3칸 미만 (mode <something>) or (mode) : NEEDMOREPARAMS 에러 반환
    else if (message.size() < 3)
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "MODE"));
    // 1번째 파라미터 <channel name>이 존재하지 않는 경우 : NOSUCHCHANNEL 에러 반환
    else if ((it = tempChannelList.find(message[1])) == tempChannelList.end())
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), message[1]));
    else if ((tempChannelList[message[1]]->getChannelOperator() == 0) || (tempChannelList[message[1]]->getChannelOperator()->getClientFd() != client.getClientFd())) {
        Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_CHANOPRIVSNEEDED(serverHost, client.getNickname(), message[1]));
    }
    else {
        for (size_t i = 0; i < message[2].size(); i++) {

            // 마지막 파라미터인 경우, last = ":"
            if (val == message.size() - 1)
                last = ":";

            // 현재 <mode>[i]가 '+' 또는 '-'인 경우
            if (message[2][i] == '+' || message[2][i] == '-') {
                // '+' 인 경우
                if (message[2][i] == '+') {
                    // 일단 "지금까지 성공한 모드 문자열"에 '+'를 추가
                    successMode += "+";
                    // 앞으로 나올 파라미터들은 모두 '+' 모드에 해당하는 파라미터이므로 flag = true로 설정
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
            // 지원하는 모드 문자열(itkol)에 포함된 경우
            if (supportMode.find(message[2][i]) != std::string::npos) {
                switch (message[2][i]) {
                    // 이번 문자열이 l인 경우 : limit 모드 -> 채널 접속 제한 인원을 변경
                    case 'l':
                        // '+' 인 경우 && (파라미터가 부족하거나(mode params가 전혀 없다는 뜻) || 숫자가 아닌 경우) -> 에러 전송
                        if (flag == true && (val >= message.size() || !chkNum(message[val])))
                            Buffer::saveMessageToBuffer(client.getClientFd(),
                                Error::ERR_INVALIDMODEPARAM(serverHost, client.getNickname(), message[1], message[2][i], "You must specify a parameter. Syntax: <limit>"));
                        // ()'+' && 오류가 없거나) '-'인 경우
                        else {
                            // 성공한 모드 문자열에 'l' 추가
                            // flag가 true 또는 false로 set 되었으니 setMode(set[0], flag)로 channel mode 변경
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
                        // switch case 빠져나가기
                        break;

                    case 'i':  
                    // invite 모드 set
                        successMode += "i";
                        it->second->setMode(set[1], flag);
                        break;

                    case 'k':
                        // 파라미터가 부족하거나 빈 문자열인 경우 에러 전송
                        if (val >= message.size() || message[val] == "")
                            Buffer::saveMessageToBuffer(client.getClientFd(),
                                Error::ERR_INVALIDMODEPARAM(serverHost, client.getNickname(), message[1], message[2][i], "You must specify a parameter. Syntax: <key>"));
                        // '-' 인 경우 키가 일치하지 않으면 에러 전송
                        else if ((flag == false && message[val] != it->second->getKey()))
                            Buffer::saveMessageToBuffer(client.getClientFd(),
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

                    case 't':
                        successMode += "t";
                        it->second->setMode(set[3], flag);
                        break;

                    case 'o':
                        if ((val >= message.size() || message[val] == ""))
                            Buffer::saveMessageToBuffer(client.getClientFd(),
                                Error::ERR_INVALIDMODEPARAM(serverHost, client.getNickname(), message[1], message[2][i], "You must specify a parameter. Syntax: <nick>"));
                        else if (!(fd = findNick(it->second->getUserList(), message[val])))
                            Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_NOSUCHNICK(serverHost, message[val]));
                        else {
                            successMode += "o";
                            it->second->setMode(set[4], flag);
                            if (flag == true) {
                                successValue += message[val] + " ";
                                it->second->setChannelOperator(it->second->getUserList().find(fd)->second);
                                val++;
                                successMode += "-o";
                                successValue += client.getNickname() + " ";
                            }
                            else {
                                successValue += message[val] + " ";
                                it->second->setChannelOperator(0);
                                val++;
                            }
                        }
                        break;
                }
            } else {
                // 지원하지 않는 모드(itkol안에 있는 문자가 아님)인 경우 에러 전송
                Buffer::saveMessageToBuffer(client.getClientFd(), Error::ERR_UNKNOWNMODE(serverHost, client.getNickname(), message[2][i]));
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
            Buffer::saveMessageToBuffer(iter->second->getClientFd(), reply::RPL_SUCCESSMODE(client.getNickname(), client.getUsername(), client.getHost(), message[1], successMode, successValue));
    }
}
