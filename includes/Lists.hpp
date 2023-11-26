#ifndef LISTS_HPP
# define LISTS_HPP

# include "utils.hpp"
# include "../includes/utils/Containers.hpp"
# include <sys/socket.h>

class List {
private:
	static ClientMap clientList;
	static ChannelMap channelList;
public:
	static bool addClientList(int fd, struct in_addr info);
	static bool deleteClientList(int fd);
	static void clearClientList();
	static Client& findClient(int fd);
	static bool hasClient(int fd);
	static ClientMap& getClientList();
	
	static bool addChannelList(std::string name, Client* creator);
	static bool deleteChannelList(std::string name);
	static void clearChannelList();
	static Channel& findChannel(std::string name);
	static bool hasChannel(std::string name);
	static ChannelMap& getChannelList();
};

#endif
