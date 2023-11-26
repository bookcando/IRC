#include "../../includes/Command.hpp"
#include "../../includes/utils/Containers.hpp"
#include "../../includes/Lists.hpp"
#include "../../includes/Buffer.hpp"
#include "../../includes/utils/reply.hpp"
#include "../../includes/Message.hpp"
#include "../../includes/Client.hpp"

static bool duplicate_nick(std::string const& nick) {
	ClientMap& clientList = Lists::getClientList();

	for (ClientMap::iterator it = clientList.begin(); it != clientList.end(); it++) {
		if (clientList[it->first]->getNickname() == nick)
			return true;
	}
	return false;
}

static void successNickChange(Client& client, std::string changeNick) {
	ClientMap& clientList = Lists::getClientList();

	for (ClientMap::iterator it = clientList.begin(); it != clientList.end(); it++) {
		if (it->second != &client)
			Buffer::sendMessage(it->second->getClientFd(), reply::RPL_SUCCESSNICK(client.getNickname(), client.getUsername(), client.getHost(), changeNick));
	}
}

void Command::motd(Client& client, std::string const& serverHost) {
	Buffer::sendMessage(client.getClientFd(), reply::RPL_MOTDSTART(serverHost, client.getNickname()));
	Buffer::sendMessage(client.getClientFd(), reply::RPL_MOTD(serverHost, client.getNickname(), "Hello! This is FT_IRC!"));
	Buffer::sendMessage(client.getClientFd(), reply::RPL_ENDOFMOTD(serverHost, client.getNickname()));
}

void Command::pass(Client& client, std::string const& password, std::string const& serverHost) {
	messageVector const& message = Message::getMessage();

	if (message.size() != 2) {
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "PASS"));
	} else if (client.getPassConnect() & IS_PASS) {
		Buffer::sendMessage(client.getClientFd(), Error::ERR_ALREADYREGISTERED(serverHost));
	} else if (message[1] != password) {
		Buffer::sendMessage(client.getClientFd(), Error::ERR_PASSWDMISMATCH(serverHost));
	} else {
		client.setPassConnect(IS_PASS);
	}
}

void Command::nick(Client& client, std::string const& serverHost) {
	messageVector const& message = Message::getMessage();

	// Nickname 충돌 오류는 어차피 서버 간 통신은 신경 쓰지 않아도 되기에 구현 안 함
	if (message.size() != 2)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NONICKNAMEGIVEN(serverHost));
	else if (duplicate_nick(message[1]))
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NICKNAMEINUSE(serverHost, message[1]));
	else if (chkForbiddenChar(message[1], "#:") || std::isdigit(message[1][0]) || message[1] == "" || message[1].size() > 9)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_ERRONEUSNICKNAME(serverHost, message[1]));
	else {
		client.setPassConnect(IS_NICK);
		if (client.getNickname() != "") {
			successNickChange(client, message[1]);
		}
		client.setNickname(message[1]);
	}
}

void Command::user(Client& client, std::string const& serverHost, std::string const& serverIp, time_t const& serverStartTime) {
	messageVector const& message = Message::getMessage();

	if (message.size() != 5)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "USER"));
	else if (client.getPassConnect() & IS_USER)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_ALREADYREGISTERED(serverHost));
	else if (!(client.getPassConnect() & (IS_PASS | IS_NICK)))
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NOTREGISTERED(serverHost, "You input pass, before It enroll User"));
	else {
		client.setPassConnect(IS_USER);
		client.setUsername(message[1]);
		client.setHost(inet_ntoa(client.getAddr()));
		client.setServer(serverIp);
		if (message[4][0] == ':')
			client.setRealname(message[4].substr(1, message[4].size()));
		else
			client.setRealname(message[4]);
		if ((client.getPassConnect() & IS_LOGIN) == IS_LOGIN) {
			time_t serv_time = serverStartTime;
			Buffer::sendMessage(client.getClientFd(), reply::RPL_WELCOME(serverHost, client.getNickname(), client.getUsername(), client.getHost()));
			Buffer::sendMessage(client.getClientFd(), reply::RPL_YOURHOST(serverHost, client.getNickname(), "1.0"));
			Buffer::sendMessage(client.getClientFd(), reply::RPL_CREATED(serverHost, client.getNickname(), getStringTime(serv_time)));
			Buffer::sendMessage(client.getClientFd(), reply::RPL_MYINFO(serverHost, client.getNickname(), "ircserv 1.0", "x", "itkol"));
			Buffer::sendMessage(client.getClientFd(), reply::RPL_ISUPPORT(serverHost, client.getNickname()));
			Command::motd(client, serverHost);
		}
	}
}
