#include "../includes/utils/Headers.hpp"
#include "../includes/utils/Containers.hpp"
#include "../includes/Client.hpp"

Client::Client(int clientFd, in_addr info) : _passConnect(0), _operator(false), _finalTime(time(NULL)), _clientFd(fd), _info(info), _host(inet_ntoa(info)), _nickname(""), _realname(""), _servname("") {
}

Client::~Client() {
	ChannelMap::iterator it = _joinList.begin();

	for (; it != _joinList.end(); it++) {
		if (it->second->getChanOp() != NULL && it->second->getChanOp()->getClientFd() == _clientFd)
			it->second->setChanOp(NULL);
		it->second->delClientList(this);
		it->second->delInviteList(this);
	}
	close(_clientFd);
}

void Client::addJoinList(Channel* channel) {
	if (_joinList.find(channel->getChannelName()) == _joinList.end())
		// _joinList.insert(std::make_pair(channel->getChannelName(), channel)); 대체 왜 안 되는 거지?
		_joinList[channel->getChannelName()] = channel;
}

void Client::deleteJoinList(Channel* channel) {
	if (_joinList.find(channel->getChannelName()) != _joinList.end())
		_joinList.erase(channel->getChannelName());
}

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
	addJoinList(channel);
	return IS_SUCCESS;
}

void Client::setPassConnect(int flag) {
	_passConnect |= flag;
}

void Client::setNickname(std::string nick) {
	_nickname = nick;
}

void Client::setRealname(std::string real) {
	_realname = real;
}

void Client::setHost(std::string host) {
	_host = host;
}

void Client::setUsername(std::string user) {
	_username = user;
}

void Client::setServer(std::string serv) {
	_server = serv;
}

void Client::setTime() {
	_finalTime = time(NULL);
}

bool Client::getOperator() const {
	return _operator;
}

int Client::getPassConnect() const {
	return _passConnect;
}

int Client::getClientFd() const {
	return _clientFd;
}

std::string const& Client::getHost() const {
	return _host;
}

std::string const& Client::getNickname() const {
	return _nickname;
}

std::string const& Client::getRealname() const {
	return _realname;
}

std::string const& Client::getUsername() const {
	return _username;
}

std::string const& Client::getServer() const {
	return _server;
}

time_t const& Client::getTime() const {
	return _finalTime;
}

in_addr const& Client::getAddr() const {
	return _info;
}
