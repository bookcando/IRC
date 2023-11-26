#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"

Channel::Channel(std::string channelName, Client* client) : _channelName(channelName), _channelOperator(client), _userLimit(0), _mode(0), _topic(""), _password(""), _creationTime(time(NULL)), _key("") {
	_userList.insert(std::make_pair(client->getClientFd(), client));
}

Channel::~Channel() {
	ClientMap::iterator iter;

	while (_inviteList.size() != 0) {
		iter = _inviteList.begin();
		iter->second = NULL;
		_inviteList.erase(iter->first);
	}
	while (_userList.size() != 0) {
		iter = _userList.begin();
		iter->second = NULL;
		_userList.erase(iter->first);
	}
}

void Channel::setChannelName(std::string& name) {
	_channelName = name;
}

void Channel::setChannelOperator(Client* client) {
	_channelOperator = client;
}

void Channel::setUserLimit(int userLimit) {
	userLimit = userLimit;
}

void Channel::setTopic(std::string topic) {
	_topic = topic;
}

void Channel::setPassword(std::string pw) {
	_password = pw;
}

void Channel::setMode(int mode, bool flag) {
	if (flag)
		mode |= mode;
	else
		mode &= mode;
}

void Channel::setKey(std::string key) {
	_key = key;
}

void Channel::addInviteList(Client* client) {
	if (_inviteList.find(client->getClientFd()) == _inviteList.end())
		_inviteList.insert(std::make_pair(client->getClientFd(), client));
}

void Channel::deleteInviteList(Client* client) {
	if (_inviteList.find(client->getClientFd()) != _inviteList.end()) {
		_inviteList[client->getClientFd()] = NULL;
		_inviteList.erase(client->getClientFd());
	}
}

bool Channel::isClientInvite(Client* client) {
	if (_inviteList.find(client->getClientFd()) != _inviteList.end())
		return true;
	return false;
}

void Channel::addClientList(Client* client) {
	if (_userList.find(client->getClientFd()) == _userList.end())
		_userList.insert(std::make_pair(client->getClientFd(), client));
}

void Channel::deleteClientList(Client* client) {
	if (_userList.find(client->getClientFd()) != _userList.end()) {
		_userList[client->getClientFd()] = NULL;
		_userList.erase(client->getClientFd());
	}
}

Client const* Channel::getChannelOperator() const {
	return _channelOperator;
}

ClientMap const& Channel::getUserList() const {
	return _userList;
}

int Channel::getUserLimit() const {
	return _userLimit;
}

std::string Channel::getChannelName() const {
	return _channelName;
}

std::string Channel::getTopic() const {
	return _topic;
}

time_t Channel::getTime() const {
	return _creationTime;
}

int Channel::getMode() const {
	return _mode;
}

std::string Channel::getKey() const {
	return _key;
}

std::string Channel::getStrUserList() const {
	std::string list = "";

	if (_channelOperator) {
		list = "@" + _channelOperator->getNickname() + " ";
	}
	for (ClientMap::const_iterator it = _userList.begin(); it != _userList.end(); it++)
		if (it->second != _channelOperator)
			list += it->second->getNickname() + " ";
	if (list[list.size() - 1] == ' ')
		list = list.substr(0, list.size() - 1);
	return list;
}
