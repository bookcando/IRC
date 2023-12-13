#include "../includes/utils/Headers.hpp"
#include "../includes/utils/Containers.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"

// Client 클래스의 생성자
Client::Client(int clientFd, in_addr info, Server* server)
: _passConnect(0), _operator(false), _finalTime(time(NULL)), 
  _clientFd(clientFd), _info(info), _host(inet_ntoa(info)), 
  _nickname(""), _realname(""), _server(""), _serverPtr(server) {
    // 클라이언트 객체 생성 시 기본값으로 초기화
}

// Client 클래스의 소멸자
Client::~Client() {
    ChannelMap::iterator it = _joinList.begin(); // 채널 목록을 순회하기 위한 반복자를 초기화합니다.

    // 채널 목록의 모든 채널에 대해 반복합니다.
    for (; it != _joinList.end(); it++) {
        // 현재 클라이언트가 채널의 오퍼레이터인 경우, 오퍼레이터를 NULL로 설정합니다.
        if (it->second->getChannelOperator() != NULL && it->second->getChannelOperator()->getClientFd() == _clientFd)
            it->second->setChannelOperator(NULL);

        // 채널의 클라이언트 목록과 초대 목록에서 현재 클라이언트를 제거합니다.
        it->second->deleteClientList(this);
        it->second->deleteInviteList(this);
    }

    // 클라이언트의 파일 기술자를 닫습니다.
    close(_clientFd);
}

// 클라이언트가 채널에 참여할 때, 참여한 채널 목록에 추가
void Client::addJoinList(Channel* channel) {
    if (_joinList.find(channel->getChannelName()) == _joinList.end())
        _joinList[channel->getChannelName()] = channel; // 채널을 참여 목록에 추가
}

// 클라이언트가 채널에서 나갈 때, 참여한 채널 목록에서 제거
void Client::deleteJoinList(Channel* channel) {
    if (_joinList.find(channel->getChannelName()) != _joinList.end())
        _joinList.erase(channel->getChannelName()); // 채널을 참여 목록에서 제거
}

// 클라이언트가 채널에 참여하는 메서드
int Client::joinChannel(Channel* channel, std::string const& key) {
	int channelMode = channel->getMode();

	if (_joinList.size() == CHANNEL_LIMIT_PER_USER)
		return TOOMANYCHANNELS;
	if (channelMode & INVITE_CHANNEL && !channel->isClientInvite(this))
		return INVITEONLYCHAN;
	if (channelMode & USER_LIMIT_PER_CHANNEL && static_cast<size_t>(channel->getUserLimit()) < channel->getUserList().size())
		return CHANNELISFULL;
	if (channelMode & KEY_CHANNEL && key != channel->getKey())
		return BADCHANNELKEY;
	channel->addClientList(this);
	if (channelMode & INVITE_CHANNEL)
		channel->deleteInviteList(this);
	this->addJoinList(channel);
	return IS_SUCCESS;
}

// PASS 명령에 대한 연결 상태 설정
void Client::setPassConnect(int flag) {
    _passConnect |= flag;
}

// 클라이언트의 닉네임 설정
void Client::setNickname(std::string nick) {
    _nickname = nick;
}

// 클라이언트의 실제 이름 설정
void Client::setRealname(std::string real) {
    _realname = real;
}

// 클라이언트의 호스트 이름 설정
void Client::setHost(std::string host) {
    _host = host;
}

// 클라이언트의 사용자 이름 설정
void Client::setUsername(std::string user) {
    _username = user;
}

// 클라이언트가 연결된 서버 이름 설정
void Client::setServer(std::string serv) {
    _server = serv;
}

// 클라이언트의 마지막 활동 시간 업데이트
void Client::setTime() {
    _finalTime = time(NULL);
}

// 클라이언트가 오퍼레이터인지 반환
bool Client::getOperator() const {
    return _operator;
}

// PASS 명령을 통한 연결 상태 반환
int Client::getPassConnect() const {
    return _passConnect;
}

// 클라이언트의 파일 기술자 반환
int Client::getClientFd() const {
    return _clientFd;
}

// 클라이언트의 호스트 이름 반환
std::string const& Client::getHost() const {
    return _host;
}

// 클라이언트의 닉네임 반환
std::string const& Client::getNickname() const {
    return _nickname;
}

// 클라이언트의 실제 이름 반환
std::string const& Client::getRealname() const {
    return _realname;
}

// 클라이언트의 사용자 이름 반환
std::string const& Client::getUsername() const {
    return _username;
}

// 클라이언트가 연결된 서버 이름 반환
std::string const& Client::getServer() const {
    return _server;
}

// 클라이언트의 마지막 활동 시간 반환
time_t const& Client::getTime() const {
    return _finalTime;
}

// 클라이언트의 IP 주소 정보 반환
in_addr const& Client::getAddr() const {
    return _info;
}

void Client::setServerPtr(Server* server) {
    _serverPtr = server;
}
Server* Client::getServerPtr() const {
    return _serverPtr;
}