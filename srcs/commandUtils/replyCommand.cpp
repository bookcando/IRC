#include "../../includes/utils/reply.hpp"

std::string const suffix = "\r\n";

std::string const reply::RPL_WELCOME(std::string const& serverHost, std::string const& nick, std::string const& user, std::string const& serv) {
	return ":" + serverHost + " 001 " + nick + " :Welcome to the Internet Relay Network " + nick + "!" + user + "@" + serv + suffix;
}

std::string const reply::RPL_YOURHOST(std::string const& serverHost, std::string const& nick, std::string const& version) {
	return ":" + serverHost + " 002 " + nick + " :Your host is " + serverHost + ", running version " + version + suffix;
}

std::string const reply::RPL_CREATED(std::string const& serverHost, std::string const& nick, std::string const& date) {
	return ":" + serverHost + " 003 " + nick + " :This server was created " + date + suffix;
}

std::string const reply::RPL_MYINFO(std::string const& serverHost, std::string const& nick, std::string const& version, std::string const& usermode, std::string const& chanmode) {
	return ":" + serverHost + " 004 " + nick + " :" + serverHost + " " + version + " " + usermode + " " + chanmode + suffix;
}

std::string const reply::RPL_ISUPPORT(std::string const& serverHost, std::string const& nick) {
	return ":" + serverHost + " 005 " + nick + " :CASEMAPPING=rfc1459 CHANMODES=i,t,k,o,l CHANTYPES=# CHARSET=ascii MASCHANNELS=10 MAXNICKLEN=9" + suffix; 
}

std::string const reply::RPL_MOTDSTART(std::string const& serverHost, std::string const& nick) {
	return ":" + serverHost + " 375 " + nick + " :- " + serverHost + " Message of the day - " + suffix;
}

std::string const reply::RPL_MOTD(std::string const& serverHost, std::string const& nick, std::string const& line) {
	return ":" + serverHost + " 372 " + nick + " :" + line + suffix;
}

std::string const reply::RPL_ENDOFMOTD(std::string const& serverHost, std::string const& nick) {
	return ":" + serverHost + " 376 " + nick + " :End of /MOTD command." + suffix;
}

std::string const reply::RPL_CHANNELMODEIS(std::string const& serverHost, std::string const& nick, std::string const& chName, std::string const& mode, std::string const& argument) {
	return ":" + serverHost + " 324 " + nick + " " + chName + " " + mode + " " + argument + suffix;
}

std::string const reply::RPL_CREATIONTIME(std::string const& serverHost, std::string const& nick, std::string const& chName, std::string const& time) {
	return ":" + serverHost + " 329 " + nick + " " + chName + " :" + time + suffix;
}

std::string const reply::RPL_SUCCESSMODE(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName, std::string const& mode, std::string const& argument) {
	return ":" + nick + "!" + user + "@" + serv + " MODE " + chName + " " + mode + " " + argument + suffix;
}

std::string const reply::RPL_SUCCESSJOIN(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName) {
	return ":" + nick + "!" + user + "@" + serv + " JOIN " + chName + " :" + chName + suffix;
}

std::string const reply::RPL_TOPIC(std::string const& serverHost, std::string const& nick, std::string const& chName, std::string const& topic) {
	return ":" + serverHost + " 332 " + nick + " " + chName + " :" + topic + suffix;
}

std::string const reply::RPL_NAMREPLY(std::string const& serverHost, std::string const& nick, std::string const& chName, std::string const& userList) {
	return ":" + serverHost + " 353 " + nick + " = " + chName + " :" + userList + suffix;
}

std::string const reply::RPL_ENDOFNAMES(std::string const& serverHost, std::string const& nick, std::string const& chName) {
	return ":" + serverHost + " 366 " + nick + " " + chName + " :End of /NAMES list." + suffix;
}

std::string const reply::RPL_SUCCESSQUIT(std::string const& nick, std::string const& user, std::string const& serv, std::string const& reason) {
	return ":" + nick + "!" + user + "@" + serv + " QUIT :Quit: " + reason + suffix;
}

std::string const reply::RPL_SUCCESSPRIVMSG(std::string const& nick, std::string const& user, std::string const& serv, std::string const& tgtName, std::string const& talk) {
	return ":" + nick + "!" + user + "@" + serv + " PRIVMSG " + tgtName + " :" + talk + suffix;
}

std::string const reply::RPL_SUCCESSPART(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName) {
	return ":" + nick + "!" + user + "@" + serv + " PART " + chName + suffix;
}

std::string const reply::RPL_SUCCESSPART(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName, std::string const& reason) {
	return ":" + nick + "!" + user + "@" + serv + " PART " + chName + " :" + reason + suffix;
}

std::string const reply::RPL_SUCCESSKICK(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName, std::string const& tgt) {
	return ":" + nick + "!" + user + "@" + serv + " KICK " + chName + " " + tgt + " :" + suffix;
}

std::string const reply::RPL_SUCCESSKICK(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName, std::string const& tgt, std::string const& reason) {
	return ":" + nick + "!" + user + "@" + serv + " KICK " + chName + " " + tgt + " :" + reason + suffix;
}

std::string const reply::RPL_INVITING(std::string const& serverHost, std::string const& nick, std::string const& tgt, std::string const& chName) {
	return ":" + serverHost + " 341 " + nick + " " + tgt + " :" + chName + suffix; 
}

std::string const reply::RPL_SUCCESSINVITING(std::string const& nick, std::string const& user, std::string const& serv, std::string const& tgt, std::string const& chName) {
	return ":" + nick + "!" + user + "@" + serv + " INVITE " + tgt + " :" + chName + suffix;
}

std::string const reply::RPL_NOTOPIC(std::string const& serverHost, std::string const& nick, std::string const& chName) {
	return ":" + serverHost + " 331 " + nick + " " + chName + " :No topic is set" + suffix;
}

std::string const reply::RPL_SUCCESSTOPIC(std::string const& nick, std::string const& user, std::string const& serv, std::string const& chName, std::string const& topic) {
	return ":" + nick + "!" + user + "@" + serv + " TOPIC " + chName + " :" + topic + suffix;
}

std::string const reply::RPL_YOUREOPER(std::string const& serverHost, std::string const& nick) {
	return ":" + serverHost + " 381 " + nick + " :You are now an IRC operator" + suffix;
}

std::string const reply::RPL_SUCCESSNICK(std::string const& nick, std::string const& user, std::string const& serv, std::string const& changeNick) {
	return ":" + nick + "!" + user + "@" + serv + " NICK :" + changeNick + suffix; 
}