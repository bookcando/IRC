#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

class Server {
private:
    int _socketFd;
    int _kqueueFd;
    struct sockaddr_in _serverAddr;
    int _port;
    std::string _pass;
    std::string _host;
    std::string _ip;


    // int kevent(int kqueueFd, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);
    struct kevent *_changeList;
    int _nChanges;
    struct kevent *_eventList;
    int _nEvents;
    const struct timespec *_timeout;

public:
    Server(std::string port, std::string pass);
    ~Server();

    void settingHostIp();
    void initializeServer();
    void addClient();
    void removeClient(Client &client);
    void receiveMessage();
    void sendMessage();
    void runServer();

    int getPort() const;

    static 

};

#endif