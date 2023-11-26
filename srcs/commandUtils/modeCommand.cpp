#include "../../includes/Command.hpp"
#include "../../includes/Buffer.hpp"
#include "../../includes/Message.hpp"
#include "../../includes/utils/Error.hpp"
#include "../../includes/utils/reply.hpp"
#include "../../includes/Lists.hpp"
#include "../../includes/Client.hpp"
#include <sstream>

static bool chkNum(std::string const& string) {
	for (size_t i = 0; i < string.size(); i++)
		if (string[i] < '0' || string[i] > '9')
			return false;
	return true;
}

static int findNick(ClientMap const& clientList, std::string const& tgt) {
	for (ClientMap::const_iterator it = clientList.begin(); it !=  clientList.end(); it++) {
		if (it->second->getNickname() == tgt)
			return it->first;
	}
	return 0;
}

static void createSetMode(int mode, Channel& channel, std::string& outputMode, std::string& outputValue) {
	std::ostringstream oss;

	outputMode += "+";
	for (int i = 0; i < 5; i++) {
		if (mode & (1 << i)) {
			switch (1 << i) {
				case USER_LIMIT_PER_CHANNEL:
					oss << channel.getUserLimit();
					outputMode += "l";
					outputValue += oss.str() + " ";
					oss.clear();
					break;
				case INVITE_CHANNEL:
					outputMode += "i";
					break;
				case KEY_CHANNEL:
					outputMode += "k";
					outputValue += channel.getKey() + " ";
					break;
				case SAFE_TOPIC:
					outputMode += "t";
					break;
				case SET_CHANOP:
					outputMode += "o";
					outputValue += channel.getChannelOperator()->getNickname() + " ";
					break;
			}
		}
	}
	if (outputValue != "" && outputValue[outputValue.size() - 1] == ' ') {
		outputValue = outputValue.substr(0, outputValue.size() - 1);
	}
}

void Command::mode(Client& client, std::string const& serverHost) {
	messageVector const& message = Message::getMessage();
	ChannelMap::iterator it;
	std::string successMode = "";
	std::string successValue = "";
	std::string supportMode = "itkol";
	std::string last = "";
	std::ostringstream oss;
	bool flag = true;
	int set[5];
	size_t val = 3;
	int fd;
	ChannelMap& chlList = Lists::getChannelList();

	if (message.size() == 2 && (it = chlList.find(message[1])) != chlList.end()) {
		oss << it->second->getTime();
		createSetMode(it->second->getMode(), *it->second, successMode, successValue);
		Buffer::sendMessage(client.getClientFd(), reply::RPL_CHANNELMODEIS(serverHost, client.getNickname(), message[1], successMode, successValue));
		Buffer::sendMessage(client.getClientFd(), reply::RPL_CREATIONTIME(serverHost, client.getNickname(), message[1], oss.str()));
	}
	else if (message.size() < 3)
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NEEDMOREPARAMS(serverHost, "MODE"));
	else if ((it = chlList.find(message[1])) == chlList.end())
		Buffer::sendMessage(client.getClientFd(), Error::ERR_NOSUCHCHANNEL(serverHost, client.getNickname(), message[1]));
	else if (chlList[message[1]]->getChannelOperator()->getClientFd() != client.getClientFd())
		Buffer::sendMessage(client.getClientFd(), Error::ERR_CHANOPRIVSNEEDED(serverHost, client.getNickname(), message[1]));
	else {
		for (size_t i = 0; i < message[2].size(); i++) {
			if (val == message.size() - 1)
				last = ":";
			if (message[2][i] == '+' || message[2][i] == '-') {
				if (message[2][i] == '+') {
					successMode += "+";
					flag = true;
					for (int j = 0; j < 5; j++)
						set[j] = (1 << j);
				} else {
					successMode += "-";
					flag = false;
					for (int k = 0; k < 5; k++)
						set[k] = ~(1 << k);
				}
				continue;
			}
			if (supportMode.find(message[2][i]) != std::string::npos) {
				switch (message[2][i]) {
					case 'l':
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
					case 'i':
						successMode += "i";
						it->second->setMode(set[1], flag);
						break;
					case 'k':
						if (val >= message.size() || message[val] == "")
							Buffer::sendMessage(client.getClientFd(),
								Error::ERR_INVALIDMODEPARAM(serverHost, client.getNickname(), message[1], message[2][i], "You must specify a parameter. Syntax: <key>"));
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
					case 't':
						successMode += "t";
						it->second->setMode(set[3], flag);
						break;
					case 'o':
						if (flag == true && (val >= message.size() || message[val] == ""))
							Buffer::sendMessage(client.getClientFd(),
								Error::ERR_INVALIDMODEPARAM(serverHost, client.getNickname(), message[1], message[2][i], "You must specify a parameter. Syntax: <nick>"));
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
				Buffer::sendMessage(client.getClientFd(), Error::ERR_UNKNOWNMODE(serverHost, client.getNickname(), message[2][i]));
			}
		}
	}
	if (message.size() != 2 && successMode != "") {
		ClientMap const& userList = it->second->getUserList();
		if (successValue != "" && successValue[successValue.size() - 1] == ' ')
			successValue = successValue.substr(0, successValue.size() - 1);
		for (ClientMap::const_iterator iter = userList.begin(); iter != userList.end(); iter++)
			Buffer::sendMessage(iter->second->getClientFd(), reply::RPL_SUCCESSMODE(client.getNickname(), client.getUsername(), client.getHost(), message[1], successMode, successValue));
	}
}
