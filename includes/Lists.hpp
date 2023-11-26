#ifndef LISTS_HPP
# define LISTS_HPP

# include "utils.hpp"
# include "../includes/utils/Containers.hpp"
# include <sys/socket.h>

class List {
private:
	static ClientMap cltList;
	static ChannelMap chlList;
public:
	static bool addClientList(int fd, struct in_addr info);
	static bool addChannelList(std::string name, Client* creator);
	static bool deleteClientList(int fd);
	static bool deleteChannelList(std::string name);
	static void clearClientList();
	static void clearChannelList();
	static Client& findClient(int fd);
	static Channel& findChannel(std::string name);
	static bool hasClient(int fd);
	static bool hasChannel(std::string name);
	static ClientMap& getClientList();
	static ChannelMap& getChannelList();
};

#endif
