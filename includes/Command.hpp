#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "./utils/Headers.hpp"
#include "./utils/Containers.hpp"
#include "./utils/Error.hpp"

namespace Command {
	int checkCommand();
	void motd(Client& client, std::string const& serverHost);
	void pass(Client& client, std::string const& _password, std::string const& serverHost);
	void nick(Client& client, std::string const& serverHost);
	void user(Client& client, std::string const& serverHost, std::string const& serverIp, time_t const& serverStartTime);
	void quit(Client& client);
	void ping(Client& client, std::string const& serverHost);
	void pong(Client& client, std::string const& serverHost, std::string const& token);
	void mode(Client& client, std::string const& serverHost);
	void privmsg(Client& client, std::string const& serverHost);
	void notice(Client& client);
	void part(Client& client, std::string const& serverHost);
	void join(Client& client, std::string const& serverHost);
	void kick(Client& client, std::string const& serverHost);
	void topic(Client& client, std::string const& serverHost);
	void invite(Client& client, std::string const& serverHost);
	bool oper(Client& client, std::string const& opName, std::string const& opPass, std::string const& serverHost, bool oper);
	int kill(Client& client, std::string const& serverHost, bool op);
}

#endif
