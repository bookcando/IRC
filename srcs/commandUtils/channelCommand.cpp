#include "../../includes/Command.hpp"
#include "../../includes/Message.hpp"
#include "../../includes/Buffer.hpp"
#include "../../includes/utils/reply.hpp"
#include "../../includes/utils/Error.hpp"
#include "../../includes/Lists.hpp"
#include "../../includes/utils/Headers.hpp"
#include "../../includes/Client.hpp"

static bool isChanName(std::string const& chanName) {
	if (chanName.size() < 2)
		return false;
	if (chanName[0] == '#')
		return true;
	return false;
}

static int findNick(ClientMap const& clientList, std::string const& tgt) {
	for (ClientMap::const_iterator it = clientList.begin(); it !=  clientList.end(); it++) {
		if (it->second->getNickname() == tgt)
			return it->first;
	}
	return 0;
}

void Command::join(Client& client, std::string const& serverHost) {
	std::istringstream chan;
	std::istringstream key;
	std::string chanStr = "";
	std::string keyStr = "";
	messageVector const& message = Message::getMessage();
	Channel* channel;

	if (message.size() < 2 || message.size() > 3)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "JOIN"));
	else {
		chan.str(message[1]);
		if (message.size() == 3)
			key.str(message[2]);
		while (std::getline(chan, chanStr, ',')) {
			if (message.size() == 3)
				std::getline(key, keyStr, ',');
			if (!isChanName(chanStr) || chanStr.size() > 200) {
				Buffer::sendMessage(client.getClientFd(), Error::ERR_BADCHANMASK(serverHost, client.getNickname(), chanStr));
				continue;
			}
			if (!Lists::hasChannel(chanStr))
				Lists::addChannelList(chanStr, &client);
			else {
				if (Lists::findChannel(chanStr).getUserList().size() == 0) {
					Lists::deleteChannelList(chanStr);
					Lists::addChannelList(chanStr, &client);
				}
			}
			channel = &Lists::findChannel(chanStr);
			switch (client.joinChannel(&Lists::findChannel(chanStr), keyStr)) {
				case TOOMANYCHANNELS:
					Buffer::sendMessage(client.getClientFd(), Error::ERR_TOOMANYCHANNELS(serverHost, client.getNickname(), chanStr));
					break;
				case CHANNELISFULL:
					Buffer::sendMessage(client.getClientFd(), Error::ERR_CHANNELISFULL(serverHost, client.getNickname(), chanStr));
					break;
				case INVITEONLYCHAN:
					Buffer::sendMessage(client.getClientFd(), Error::ERR_INVITEONLYCHAN(serverHost, client.getNickname(), chanStr));
					break;
				case BADCHANNELKEY:
					Buffer::sendMessage(client.getClientFd(), Error::ERR_BADCHANNELKEY(serverHost, client.getNickname(), chanStr));
					break;
				case IS_SUCCESS:
					channel->deleteInviteList(&client);
					Buffer::sendMessage(client.getClientFd(), reply::RPL_SUCCESSJOIN(client.getNickname(), client.getUsername(), client.getHost(), chanStr));
					if (channel->getTopic() != "")
						Buffer::sendMessage(client.getClientFd(), reply::RPL_TOPIC(serverHost, client.getNickname(), chanStr, channel->getTopic()));
					Buffer::sendMessage(client.getClientFd(), reply::RPL_NAMREPLY(serverHost, client.getNickname(), chanStr, channel->getStrUserList()));
					Buffer::sendMessage(client.getClientFd(), reply::RPL_ENDOFNAMES(serverHost, client.getNickname(), chanStr));
					for (ClientMap::const_iterator iter = channel->getUserList().begin(); iter != channel->getUserList().end(); iter++)
						if (iter->second != &client)
							Buffer::sendMessage(iter->second->getClientFd(), reply::RPL_SUCCESSJOIN(client.getNickname(), client.getUsername(), client.getHost(), chanStr));
					break;
			}
			chanStr = "";
			keyStr = "";
		}
	}
}

void Command::part(Client& client, std::string const& serverHost) {
	messageVector const& message = Message::getMessage();
	std::string chanStr = "";
	std::string reason = "";
	std::istringstream str;
	ClientMap userList;
	Channel* chan;
	ClientMap::const_iterator it;

	if (message.size() < 2 || message.size() > 3)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "PART"));
	else {
		str.str(message[1]);
		while (std::getline(str, chanStr, ',')) {
			if (Lists::hasChannel(chanStr))
				chan = &Lists::findChannel(chanStr);
			else
				chan = NULL;
			if (chan == NULL)
				Buffer::sendMessage(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), chanStr));
			else if (chan->getUserList().find(client.getClientFd()) == chan->getUserList().end())
				Buffer::sendMessage(client.getClientFd(), Error::ERR_NOTONCHANNEL(serverHost, client.getNickname(), chan->getChannelName()));
			else {
				userList = chan->getUserList();
				if (chan->getChannelOperator() != NULL && chan->getChannelOperator()->getClientFd() == client.getClientFd())
					chan->setChannelOperator(NULL);
				for (it = userList.begin(); it != userList.end(); it++) {
					if (message.size() == 3)
						Buffer::sendMessage(it->second->getClientFd(), reply::RPL_SUCCESSPART(client.getNickname(), client.getUsername(), client.getHost(), chan->getChannelName(), message[2]));
					else
						Buffer::sendMessage(it->second->getClientFd(), reply::RPL_SUCCESSPART(client.getNickname(), client.getUsername(), client.getHost(), chan->getChannelName()));
				}
				chan->deleteClientList(&client);
				if (chan->getUserList().size() == 0)
					Lists::deleteChannelList(chan->getChannelName());
			}
		}
	}
}

void Command::kick(Client& client, std::string const& serverHost) {
	messageVector const& message = Message::getMessage();
	std::string reason = "";
	std::string userStr = "";
	std::istringstream str;
	ClientMap::const_iterator it;
	int tgtFd = 0;
	ClientMap userList;
	Channel* chan;

	if (message.size() < 3 || message.size() > 4)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "KICK"));
	else if (!Lists::hasChannel(message[1]))
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), message[1]));
	else if ((chan = &Lists::findChannel(message[1]))->getChannelOperator() == NULL || chan->getChannelOperator()->getClientFd() != client.getClientFd())
		Buffer::sendMessage(client.getClientFd(), Error::ERR_CHANOPRIVSNEEDED(serverHost, client.getNickname(), message[1]));
	else {
		str.str(message[2]);

		while (std::getline(str, userStr, ',')) {
			userList = chan->getUserList();
			if ((tgtFd = findNick(userList, userStr)) == 0)
				Buffer::sendMessage(client.getClientFd(), Error::ERR_USERNOTINCHANNEL(serverHost, client.getNickname(), userStr, chan->getChannelName()));
			else {
				for (it = userList.begin(); it != userList.end(); it++) {
					if (message.size() == 4)
						Buffer::sendMessage(it->second->getClientFd(), reply::RPL_SUCCESSKICK(client.getNickname(), client.getUsername(), client.getHost(), message[1], userStr, message[3]));
					else
						Buffer::sendMessage(it->second->getClientFd(), reply::RPL_SUCCESSKICK(client.getNickname(), client.getUsername(), client.getHost(), message[1], userStr));
				}
					
				chan->deleteClientList(userList.find(tgtFd)->second);
			}
		}
	}
}

void Command::invite(Client& client, std::string const& serverHost) {
	messageVector const& message = Message::getMessage();
	ClientMap userList;
	Channel* chan;
	int tgtFd;

	if (message.size() != 3)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "INVITE"));
	else if (!Lists::hasChannel(message[2]))
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), message[2]));
	else if ((userList = (chan = &Lists::findChannel(message[2]))->getUserList()).find(client.getClientFd()) == userList.end())
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NOTONCHANNEL(serverHost, client.getNickname(), message[2]));
	else if (chan->getMode() & INVITE_CHANNEL
				&& (chan->getChannelOperator() == NULL || chan->getChannelOperator()->getClientFd() != client.getClientFd()))
		Buffer::sendMessage(client.getClientFd(), Error::ERR_CHANOPRIVSNEEDED(serverHost, client.getNickname(), message[2]));
	else if (findNick(userList, message[1]))
		Buffer::sendMessage(client.getClientFd(), Error::ERR_USERONCHANNEL(serverHost, client.getNickname(), message[1], message[2]));
	else {
		if (!(tgtFd = findNick(Lists::getClientList(), message[1])))
			return ;
		else {
			chan->addInviteList(&Lists::findClient(tgtFd));
			Buffer::sendMessage(client.getClientFd(), reply::RPL_INVITING(serverHost, client.getNickname(), message[2], message[1]));
			Buffer::sendMessage(tgtFd, reply::RPL_SUCCESSINVITING(client.getNickname(), client.getUsername(), client.getHost(), message[2], message[1]));
		}
	}
}

void Command::topic(Client& client, std::string const& serverHost) {
	messageVector const& message = Message::getMessage();
	ClientMap::const_iterator it;
	ClientMap userList;
	Channel *chan;

	if (message.size() < 2 || message.size() > 3)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "TOPIC"));
	else if (!Lists::hasChannel(message[1]))
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), message[1]));
	else if (message.size() == 2) {
		if ((chan = &Lists::findChannel(message[1]))->getTopic() == "")
			Buffer::sendMessage(client.getClientFd(), reply::RPL_NOTOPIC(serverHost, client.getNickname(), message[1]));
		else
			Buffer::sendMessage(client.getClientFd(), reply::RPL_TOPIC(serverHost, client.getNickname(), message[1], chan->getTopic()));
	}
	else if ((chan = &Lists::findChannel(message[1]))->getMode() & SAFE_TOPIC && (chan->getChannelOperator() == NULL || chan->getChannelOperator()->getClientFd() != client.getClientFd()))
		Buffer::sendMessage(client.getClientFd(), Error::ERR_CHANOPRIVSNEEDED(serverHost, client.getNickname(), message[1]));
	else {
		userList = chan->getUserList();
		chan->setTopic(message[2]);

		for (it = userList.begin(); it != userList.end(); it++)
			Buffer::sendMessage(it->second->getClientFd(), reply::RPL_SUCCESSTOPIC(client.getNickname(), client.getUsername(), client.getHost(), message[1], message[2]));
	}
}
