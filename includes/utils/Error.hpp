#ifndef _ERROR_HPP_
#define _ERROR_HPP_

#include "Headers.hpp"

namespace Error {
	std::string const ERR_INPUTTOOLONG(std::string const& serverHost);
	std::string const ERR_NEEDMOREPARAMS(std::string const& serverHost, std::string const& command);
	std::string const ERR_ALREADYREGISTERED(std::string const& serverHost);
	std::string const ERR_PASSWDMISMATCH(std::string const& serverHost);
	std::string const ERR_NONICKNAMEGIVEN(std::string const& serverHost);
	std::string const ERR_NICKNAMEINUSE(std::string const& serverHost, std::string const& nick);
	std::string const ERR_ERRONEUSNICKNAME(std::string const& serverHost, std::string const& nick);
	std::string const ERR_NOTREGISTERED(std::string const& serverHost, std::string const& reason);
	std::string const ERR_UNKNOWNCOMMAND(std::string const& serverHost, std::string const& command);
	std::string const ERR_NOORIGIN(std::string const& serverHost, std::string const& nick);
	std::string const ERR_INVALIDMODEPARAM(std::string const& serverHost, std::string const& nick, std::string const& chName, char const& mode, std::string const& reason);
	std::string const ERR_UNKNOWNMODE(std::string const& serverHost, std::string const& nick, char const& mode);
	std::string const ERR_NOSUCHCHANNEL(std::string const& serverHost, std::string const& nick, std::string const& chName);
	std::string const ERR_CHANOPRIVSNEEDED(std::string const& serverHost, std::string const& nick, std::string const& chName);
	std::string const ERR_BADCHANMASK(std::string const& serverHost, std::string const& nick, std::string const& chName);
	std::string const ERR_TOOMANYCHANNELS(std::string const& serverHost, std::string const& nick, std::string const& chName);
	std::string const ERR_CHANNELISFULL(std::string const& serverHost, std::string const& nick, std::string const& chName);
	std::string const ERR_INVITEONLYCHAN(std::string const& serverHost, std::string const& nick, std::string const& chName);
	std::string const ERR_BADCHANNELKEY(std::string const& serverHost, std::string const& nick, std::string const& chName);
	std::string const ERR_NORECIPIENT(std::string const& serverHost, std::string const& nick, std::string const& command);
	std::string const ERR_NOTEXTTOSEND(std::string const& serverHost, std::string const& nick);
	std::string const ERR_NOSUCHNICK(std::string const& serverHost, std::string const& nick);
	std::string const ERR_CANNOTSENDTOCHAN(std::string const& serverHost, std::string const& nick, std::string const& chName);
	std::string const ERR_NOTONCHANNEL(std::string const& serverHost, std::string const& nick, std::string const& chName);
	std::string const ERR_USERNOTINCHANNEL(std::string const& serverHost, std::string const& nick, std::string const& tgt, std::string const& chName);
	std::string const ERR_USERONCHANNEL(std::string const& serverHost, std::string const& nick, std::string const& tgt, std::string const& chName);
	std::string const ERR_NOOPERHOST(std::string const& serverHost, std::string const& nick, std::string const& reason);
	std::string const ERR_ALREADYOTHEROPER(std::string const& serverHost, std::string const& nick);
	std::string const ERR_NOOPIVILEGES(std::string const& serverHost, std::string const& nick);
}

#endif