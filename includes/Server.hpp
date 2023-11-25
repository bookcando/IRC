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
    std::string _pass;
    std::string _host;
    std::string _ip;

    //server의 recv/send buffer
    std::string _recvBuffer;
    std::string _sendBuffer;

    ClientMap _clientList;

    EventList _newEventFdList;

    // int kevent(int kqueueFd, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);
    struct kevent _kEventList[100];
    int _nEvents;
    struct timespec *_timeout;

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
    void handleReadEvent(int fd, intptr_t data);
    void handleWriteEvent(int fd);
    void handleDisconnectedClients();

    void pushEvents(EventList &eventFdList, uintptr_t fd, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata);
    // struct kevent {
    //     uintptr_t       ident;          /*  identifier for this event */
    //     short           filter;         /*  filter for event */
    //     u_short         flags;          /*  general flags */
    //     u_int           fflags;         /*  filter-specific flags */
    //     intptr_t        data;           /*  filter-specific data */
    //     void            *udata;         /*  opaque user data identifier */
    // };
    // ident : 이벤트가 발생한 파일 디스크립터
    // filter : 이벤트를 검출할 때 사용할 필터
    // flags : 이벤트를 검출할 때 사용할 플래그
    // fflags : 필터에 따라 사용할 플래그
    // data : 필터에 따라 사용할 데이터
    // udata : 사용자 정의 데이터
    int getPort() const;
};

#endif