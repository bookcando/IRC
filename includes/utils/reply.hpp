#ifndef REPLY_HPP
#define REPLY_HPP

# include <string>

// numeric reply
namespace reply {
	std::string const RPL_WELCOME(std::string const& serverHost, std::string const& nick, std::string const& user, std::string const& serv);
	std::string const RPL_YOURHOST(std::string const& serverHost, std::string const& nick, std::string const& version);
	std::string const RPL_CREATED(std::string const& serverHost, std::string const& nick, std::string const& date);
	std::string const RPL_MYINFO(std::string const& serverHost, std::string const& nick, std::string const& version, std::string const& usermode, std::string const& chanmode);
	std::string const RPL_ISUPPORT(std::string const& serverHost, std::string const& nick);
	std::string const RPL_MOTDSTART(std::string const& serverHost, std::string const& nick);
	std::string const RPL_MOTD(std::string const& serverHost, std::string const& nick, std::string const& line);
	std::string const RPL_ENDOFMOTD(std::string const& serverHost, std::string const& nick);
	std::string const RPL_SUCCESSMODE(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName, std::string const& mode, std::string const& argument);
	std::string const RPL_CHANNELMODEIS(std::string const& serverHost, std::string const& nick, std::string const& chName, std::string const& mode, std::string const& argument);
	std::string const RPL_CREATIONTIME(std::string const& serverHost, std::string const& nick, std::string const& chName, std::string const& time);
	std::string const RPL_SUCCESSJOIN(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName);
	std::string const RPL_TOPIC(std::string const& serverHost, std::string const& nick, std::string const& chName, std::string const& topic);
	std::string const RPL_NAMREPLY(std::string const& serverHost, std::string const& nick, std::string const& chName, std::string const& userList);
	std::string const RPL_ENDOFNAMES(std::string const& serverHost, std::string const& nick, std::string const& chName);
	std::string const RPL_SUCCESSQUIT(std::string const& nick, std::string const& user, std::string const& serv, std::string const& reason);
	std::string const RPL_SUCCESSPRIVMSG(std::string const& nick, std::string const& user, std::string const& serv, std::string const& tgtName, std::string const& talk);
	std::string const RPL_SUCCESSNOTICE(std::string const& nick, std::string const& user, std::string const& serv, std::string const& tgtName, std::string const& talk);
	std::string const RPL_SUCCESSPART(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName);
	std::string const RPL_SUCCESSPART(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName, std::string const& reason);
	std::string const RPL_SUCCESSKICK(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName, std::string const& tgt);
	std::string const RPL_SUCCESSKICK(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName, std::string const& tgt, std::string const& reason);
	std::string const RPL_INVITING(std::string const& serverHost, std::string const& nick, std::string const& tgt, std::string const& chName);
	std::string const RPL_SUCCESSINVITING(std::string const& nick, std::string const& user, std::string const& serv, std::string const& tgt, std::string const& chName);
	std::string const RPL_NOTOPIC(std::string const& serverHost, std::string const& nick, std::string const& chName);
	std::string const RPL_SUCCESSTOPIC(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName, std::string const& topic);
	std::string const RPL_YOUREOPER(std::string const& serverHost, std::string const& nick);
	std::string const RPL_SUCCESSNICK(std::string const& nick, std::string const& user, std::string const& serv, std::string const& changeNick);
}

#endif
