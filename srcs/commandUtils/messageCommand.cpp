#include "../../includes/Command.hpp"
#include "../../includes/Buffer.hpp"
#include "../../includes/Message.hpp"
#include "../../includes/utils/Error.hpp"
#include "../../includes/utils/reply.hpp"
#include "../../includes/Lists.hpp"
#include "../../includes/Client.hpp"
#include <iostream>
#include <sstream>

static int findNick(ClientMap cltList, std::string const& tgt) {
	for (ClientMap::const_iterator it = cltList.begin(); it != cltList.end(); it++) {
		if (it->second->getNickname() == tgt)
			return it->first;
	}
	return 0;
}

void Command::privmsg(Client& client, std::string const& serverHost) {
	messageVector const& message = Message::getMessage();
	int fd;
	std::string tgtStr;
	std::string zipPrefix;
	std::istringstream str;
	size_t chanPrefix;
	Channel* chan;

	if (message.size() == 1)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NORECIPIENT(serverHost, client.getNickname(), "PRIVMSG"));
	else if (message.size() == 2)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NOTEXTTOSEND(serverHost, client.getNickname()));
	else {
		str.str(message[1]);
		while (std::getline(str, tgtStr, ',')) {
			zipPrefix = "";
			chanPrefix = 0;
			if ((chanPrefix = tgtStr.find('#')) != std::string::npos) {
				zipPrefix = tgtStr.substr(0, chanPrefix);
				if (zipPrefix.size() == 1 && zipPrefix[0] == '@')
					tgtStr.substr(1, tgtStr.size());
				if (!Lists::hasChannel(tgtStr))
					Buffer::sendMessage(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), tgtStr));
				else if (zipPrefix[0] == '@') {
					if ((chan = &Lists::findChannel(tgtStr))->getChannelOperator() == NULL
						|| chan->getChannelOperator()->getNickname() != tgtStr)
						Buffer::sendMessage(client.getClientFd(), Error::ERR_NOSUCHNICK(serverHost, client.getNickname()));
					else if (chan->getUserList().find(client.getClientFd()) == chan->getUserList().end())
						Buffer::sendMessage(client.getClientFd(), Error::ERR_CANNOTSENDTOCHAN(serverHost, client.getNickname(), chan->getChannelName()));
					else
						Buffer::sendMessage(chan->getChannelOperator()->getClientFd(), message[2]);
				}
				else {
					chan = &Lists::findChannel(tgtStr);
					ClientMap::const_iterator iter;
					ClientMap const& userList = chan->getUserList();
				
					if (!findNick(chan->getUserList(), client.getNickname()))
						Buffer::sendMessage(client.getClientFd(), Error::ERR_CANNOTSENDTOCHAN(serverHost, client.getNickname(), chan->getChannelName()));
					else {
						for (iter = userList.begin(); iter != userList.end(); iter++)
							if (iter->second->getNickname() != client.getNickname())
								Buffer::sendMessage(iter->second->getClientFd(), reply::RPL_SUCCESSPRIVMSG(client.getNickname(), client.getUsername(), client.getHost(), tgtStr, message[2]));
					}
				}
			}
			else {
				if (!(fd = findNick(Lists::getClientList(), tgtStr)))
					Error::ERR_NOSUCHNICK(serverHost, tgtStr);
				else
					Buffer::sendMessage(fd, reply::RPL_SUCCESSPRIVMSG(client.getNickname(), client.getUsername(), client.getHost(), tgtStr, message[2]));
			}
		}
	}
}

void Command::notice(Client& client) {
	messageVector const& message = Message::getMessage();
	int fd;
	std::string tgtStr;
	std::string zipPrefix;
	std::istringstream str;
	size_t chanPrefix;
	Channel *chan;

	if (message.size() > 2) {
		str.str(message[1]);
		while (std::getline(str, tgtStr, ',')) {
			zipPrefix = "";
			chanPrefix = 0;
			if ((chanPrefix = tgtStr.find('#')) != std::string::npos) {
				zipPrefix = tgtStr.substr(0, chanPrefix);
				if (zipPrefix.size() == 1 && zipPrefix[0] == '@')
					tgtStr.substr(1, tgtStr.size());
				if (!Lists::hasChannel(tgtStr))
					;
				else if (zipPrefix[0] == '@') {
					if ((chan = &Lists::findChannel(tgtStr))->getChannelOperator() == NULL
						|| chan->getChannelOperator()->getNickname() != tgtStr
						|| chan->getUserList().find(client.getClientFd()) == chan->getUserList().end())
						;
					else
						Buffer::sendMessage(chan->getChannelOperator()->getClientFd(), message[2]);
				}
				else {
					chan = &Lists::findChannel(tgtStr);
					ClientMap::const_iterator iter;
					ClientMap const& userList = chan->getUserList();
				
					if (!findNick(chan->getUserList(), client.getNickname()))
						;
					else {
						for (iter = userList.begin(); iter != userList.end(); iter++)
							if (iter->second->getNickname() != client.getNickname())
								Buffer::sendMessage(iter->second->getClientFd(), reply::RPL_SUCCESSNOTICE(client.getNickname(), client.getUsername(), client.getHost(), tgtStr, message[2]));
					}
				}
			}
			else {
				if (!(fd = findNick(Lists::getClientList(), tgtStr)))
					;
				else
					Buffer::sendMessage(fd, reply::RPL_SUCCESSNOTICE(client.getNickname(), client.getUsername(), client.getHost(), tgtStr, message[2]));
			}
		}
	}
}
