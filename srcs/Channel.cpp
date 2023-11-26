#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"

// Channel 클래스의 생성자
Channel::Channel(std::string channelName, Client* client) 
: _channelName(channelName), _channelOperator(client), _userLimit(0), 
  _mode(0), _topic(""), _password(""), _creationTime(time(NULL)), _key("") {
    // 채널 생성 시 사용자를 사용자 목록에 추가합니다.
    _userList.insert(std::make_pair(client->getClientFd(), client));
}

// Channel 클래스의 소멸자
Channel::~Channel() {
	ClientMap::iterator iter;
    // 초대 리스트와 사용자 리스트를 순회하며 모든 항목을 제거합니다.
    while (!_inviteList.empty()) {
        iter = _inviteList.begin(); // 리스트의 첫 번째 항목을 가리킵니다.
        _inviteList.erase(iter->first); // 초대 리스트의 첫 번째 항목을 제거합니다.
    }
    while (!_userList.empty()) {
        iter = _userList.begin(); // 리스트의 첫 번째 항목을 가리킵니다.
        _userList.erase(iter->first); // 사용자 리스트의 첫 번째 항목을 제거합니다.
    }
}

// 채널 이름 설정
void Channel::setChannelName(std::string& name) {
    _channelName = name; // 채널 이름을 설정합니다.
}

// 채널 운영자 설정
void Channel::setChannelOperator(Client* client) {
    _channelOperator = client; // 채널 운영자를 설정합니다.
}

// 채널의 사용자 제한 설정
void Channel::setUserLimit(int userLimit) {
    _userLimit = userLimit; // 사용자 제한 수를 설정합니다.
}

// 채널의 토픽 설정
void Channel::setTopic(std::string topic) {
    _topic = topic; // 채널 토픽을 설정합니다.
}

// 채널의 비밀번호 설정
void Channel::setPassword(std::string pw) {
    _password = pw; // 채널 비밀번호를 설정합니다.
}

// 채널 모드 설정
void Channel::setMode(int mode, bool flag) {
    if (flag)
        _mode |= mode; // 주어진 모드를 추가합니다.
    else
        _mode &= ~mode; // 주어진 모드를 제거합니다.
}

// 채널의 키 설정
void Channel::setKey(std::string key) {
    _key = key; // 채널 키를 설정합니다.
}

// 채널의 초대 리스트에 클라이언트 추가
void Channel::addInviteList(Client* client) {
    if (_inviteList.find(client->getClientFd()) == _inviteList.end())
        _inviteList.insert(std::make_pair(client->getClientFd(), client)); // 클라이언트를 초대 리스트에 추가합니다.
}

// 채널의 초대 리스트에서 클라이언트 제거
void Channel::deleteInviteList(Client* client) {
    if (_inviteList.find(client->getClientFd()) != _inviteList.end()) {
        _inviteList.erase(client->getClientFd()); // 클라이언트를 초대 리스트에서 제거합니다.
    }
}

// 특정 클라이언트가 초대되었는지 확인
bool Channel::isClientInvite(Client* client) {
    return _inviteList.find(client->getClientFd()) != _inviteList.end(); // 클라이언트가 초대 리스트에 있는지 확인합니다.
}

// 채널의 사용자 리스트에 클라이언트 추가
void Channel::addClientList(Client* client) {
    if (_userList.find(client->getClientFd()) == _userList.end())
        _userList.insert(std::make_pair(client->getClientFd(), client)); // 클라이언트를 사용자 리스트에 추가합니다.
}

// 채널의 사용자 리스트에서 클라이언트 제거
void Channel::deleteClientList(Client* client) {
    if (_userList.find(client->getClientFd()) != _userList.end()) {
        _userList.erase(client->getClientFd()); // 클라이언트를 사용자 리스트에서 제거합니다.
    }
}

// 채널 운영자 정보 가져오기
Client const* Channel::getChannelOperator() const {
    return _channelOperator; // 채널 운영자를 반환합니다.
}

// 채널의 사용자 리스트 정보 가져오기
ClientMap const& Channel::getUserList() const {
    return _userList; // 채널의 사용자 리스트를 반환합니다.
}

// 채널의 사용자 제한 정보 가져오기
int Channel::getUserLimit() const {
    return _userLimit; // 채널의 사용자 제한 수를 반환합니다.
}

// 채널 이름 정보 가져오기
std::string Channel::getChannelName() const {
    return _channelName; // 채널 이름을 반환합니다.
}

// 채널의 토픽 정보 가져오기
std::string Channel::getTopic() const {
    return _topic; // 채널 토픽을 반환합니다.
}

// 채널 생성 시간 정보 가져오기
time_t Channel::getTime() const {
    return _creationTime; // 채널 생성 시간을 반환합니다.
}

// 채널 모드 정보 가져오기
int Channel::getMode() const {
    return _mode; // 채널 모드를 반환합니다.
}

// 채널 키 정보 가져오기
std::string Channel::getKey() const {
    return _key; // 채널 키를 반환합니다.
}

// 채널의 사용자 목록을 문자열 형태로 반환
std::string Channel::getStrUserList() const {
    std::string list = ""; // 사용자 목록을 저장할 문자열 초기화

    // 채널 운영자가 있으면 목록의 시작에 추가
    if (_channelOperator) {
        list = "@" + _channelOperator->getNickname() + " ";
    }

    // 채널의 모든 사용자에 대해 반복
    for (ClientMap::const_iterator it = _userList.begin(); it != _userList.end(); it++) {
        if (it->second != _channelOperator) // 채널 운영자가 아니면 목록에 추가
            list += it->second->getNickname() + " ";
    }

    // 마지막 공백 제거
    if (list[list.size() - 1] == ' ')
        list = list.substr(0, list.size() - 1);

    return list; // 사용자 목록 문자열 반환
}
