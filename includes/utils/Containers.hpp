#ifndef CONTAINERS_HPP
#define CONTAINERS_HPP

#include "Headers.hpp"
class Client;

typedef std::map<int, Client*> ClientMap;
typedef std::map<std::string, Channel*> ChannelMap;
typedef std::vector<struct kevent> EventList;
typedef std::map<int, std::string> BufferMap;
typedef std::vector<std::string> messageVector;

#endif