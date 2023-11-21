#ifndef CONTAINERS_HPP
#define CONTAINERS_HPP

#include "Headers.hpp"
class Client;

typedef std::map<int, Client*> ClientMap;
typedef std::vector<struct kevent> EventList;

#endif