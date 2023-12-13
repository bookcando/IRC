#include "../includes/Command.hpp"
#include "../includes/utils/Headers.hpp"
#include "../includes/utils/utils.hpp"
#include "../includes/Message.hpp"

int Command::checkCommand() { // 명령어를 확인하는 함수
	messageVector const& message = Message::getMessage();

	if (!message.size())
		return IS_NOT_ORDER;
	if (message[0] == "PASS")
		return IS_PASS;
	if (message[0] == "NICK")
		return IS_NICK;
	if (message[0] == "USER")
		return IS_USER;
	if (message[0] == "PING")
		return IS_PING;
	if (message[0] == "PONG")
		return IS_PONG;
	if (message[0] == "MODE")
		return IS_MODE;
	if (message[0] == "JOIN")
		return IS_JOIN;
	if (message[0] == "QUIT")
		return IS_QUIT;
	if (message[0] == "PART")
		return IS_PART;
	if (message[0] == "PRIVMSG")
		return IS_PRIVMSG;
	if (message[0] == "PART")
		return IS_PART;
	if (message[0] == "KICK")
		return IS_KICK;
	if (message[0] == "INVITE")
		return IS_INVITE;
	if (message[0] == "TOPIC")
		return IS_TOPIC;
	return IS_NOT_ORDER;
}


