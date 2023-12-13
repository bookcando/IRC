#ifndef SERVER_HPP
#define SERVER_HPP

#include "./utils/Headers.hpp"
#include "./utils/Containers.hpp"
#include "Client.hpp"
#include "Buffer.hpp"

class Server {
private:
    int _socketFd;
    int _kqueueFd;
    struct sockaddr_in _serverAddr;
    int _port;
    Client *_op;
    std::string _opName;
    std::string _opPassword;
    std::string _pass;
    std::string _host;
    std::string _ip;

    EventList _newEventFdList;

    int _nEvents;
    struct kevent _kEventList[100];
    struct timespec _timeout;

    bool _isRunning;
    time_t _startTime;

public:
    Server(std::string port, std::string pass);
    ~Server();

    void initializeServer();
    void settingHostIp();

    void runServer();
    
    void addClient(int fd);
    void deleteClient(int fd);
    void removeClient(int clientFd);

    void receiveMessage();
    // void sendMessage(Client &client, std::string message);
    void sendMessage(Client &client);


    void receiveMessage(int clientFd);
    void sendMessage(int cliendFd, std::string message);
    void sendMessage(int cliendFd);
    bool isServerEvent(uintptr_t ident);

    bool containsCurrentEvent(uintptr_t ident);
    void handleReadEvent(int fd, intptr_t data, std::string host);
    void handleWriteEvent(int fd);
    void handleDisconnectedClients();

    void executeCommand(int fd);

    void pushEvents(uintptr_t fd, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata);

    int getPort() const;
    std::string const& getHost() const;
};

#endif