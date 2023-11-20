#include "../includes/Server.hpp"
#include "../includes/Validator.hpp"

// 각 Class 마다의 책임 + 해야 하는 일...

Server::Server(std::string port, std::string pass) : _pass(pass) {
    long long validatedPort;

    validatedPort = Validator::validatePort(port);
    _port = static_cast<int>(validatedPort);
    Validator::validatePassword(pass);
    settingHostIp();
    if ((_kqueueFd = kqueue()) == -1)
        throw std::runtime_error("ERROR: Kqueue creation failed");
}

Server::~Server() {

}

void Server::settingHostIp() {
    char hostName[1024];
    struct hostent *hostStruct;

    if (gethostname(hostName, sizeof(hostName)) == -1)
        throw std::runtime_error("ERROR: Hostname error");
    else {
        if (!(hostStruct = gethostbyname(hostName)))
            throw std::runtime_error("ERROR: Hostname error");
        else
            _ip = inet_ntoa(*(struct in_addr*)hostStruct->h_addr_list[0]);
    }
    _host = "irc.localHost.net";
}

void Server::initializeServer() {    
    // initializing...
    // 1. 소켓 만들기
    _socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketFd == -1) {
        throw std::runtime_error("ERROR: Socket creation failed");
    }
    memset(&_serverAddr, 0, sizeof(_serverAddr));
    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    _serverAddr.sin_port = htons(_port);

    // 2. 소켓 옵션 설정
    int opt = 1;
    setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // 3. 소켓 바인드(IP, Port)
    if (bind(_socketFd, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) == -1) {
        throw std::runtime_error("ERROR: Socket bind failed");
    }
    // 4. 소켓 리슨 -> 이제 받기 시작하도록
    if (listen(_socketFd, 100) == -1)
        throw std::runtime_error("ERROR: Socket listen failed");
    // 이거 밑으로는 루프로 갑시다~
    // 5. 소켓 억셉트 -> 이건 loop로 빼기. 이건 addClient?
    // 6. kqueue 만들기.
    // 7. 이벤트 등록 또한 loop로 빼기.
}

// loop에서 master socket에 이벤트 검출 -> 새로 접속할 클라이언트가 있음! -> accept

void Server::runServer() {
    // 1. while loop + 
    // server run flag 만들어서 flag가 0(오류가 발생되어 종료해야 하는 경우) 이면 while이 끝나도록?
    // 2. kqueue에서 이벤트 검출
    // 3. 이벤트 검출된 소켓이 master socket인지, client socket인지 구분
    // 4. master socket이면 새로운 클라이언트 접속 -> addClient
    // 5. client socket이면 메시지 수신 -> receiveMessage
    // client socket인데 recv = 0 이면 -> 접속 종료 -> removeClient
    // 이제 보낼 메세지 확인 : socket의 send buffer에 메세지가 있는지 확인 + 메세지를 보낼 수 있는지 확인.
    // 6. client socket이면 메시지 송신 -> sendMessage
    // 7. exception throw 된 거 찾았으면 server 종료??


    //이건 initialization으로 빼자.
    this->_kqueueFd = kqueue();
    std::memset(_changeList, 0, sizeof(_changeList));
    std::memset(_eventList, 0, sizeof(_eventList));
    EV_SET(&_changeList[0], _socketFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    _timeout = { 3, 0 };


    // EV_SET(&_ev, _socketFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    int flag = 1; // flag를 
    int newEvent;
    while (flag)
    {

        newEvent = kevent(_kqueueFd, _changeList, _nChanges, _eventList, _nEvents, _timeout);
        // blah blah
        // 여기서는 kevent를 사용하여 이벤트를 대기.
        // changeList : 이벤트를 등록하거나 삭제할 때 사용하는 구조체 : EV_SET을 통해 등록
        // nChanges : changeList에 등록된 이벤트의 개수
        // eventList : 이벤트가 발생했을 때 정보를 담는 구조체
        // nEvents : eventList에 등록된 이벤트의 개수
        // timeout : 이벤트가 발생할 때까지 대기하는 시간
        // 이벤트가 발생하면 발생한 이벤트의 개수를 반환하고, timeout이 지나면 0을 반환한다.
        // 이벤트가 발생하지 않으면 -1을 반환하고 errno에 오류 코드를 저장한다.


        if (newEvent == -1) {
            throw std::runtime_error("ERROR: Kqueue event error");
        }
        // 대충 나눔??
        // 네 진짜대충임미닷

        for (int i = 0; i < newEvent; i++) {
            if (_eventList[i].ident == _socketFd) {
                // master socket에 이벤트가 발생한 경우
                // 근데 master socket이 오류가 난 걸 수도 있음
                if (_eventList[i].flags & EV_ERROR) {
                    throw std::runtime_error("ERROR: Master socket error");
                    flag = 0;
                }
                else {
                    // 새로운 클라이언트 접속
                    addClient();
                }
            }
            else {
                // client socket에 이벤트가 발생한 경우
                if (_eventList[i].flags & EV_EOF) {
                    // 접속 종료
                    removeClient();
                }
                else {
                    // 메세지 수신
                    receiveMessage();
                }
            }
            //발생한 이벤트를 순회하며 확인하였음. 
        }
        // 
    }

}

void Server::addClient() {
    // 이미 master socket에서 이벤트가 검출되어서 함수가 호출됨
    // -> accept를 통해 새로운 클라이언트를 받아옴.


    struct sockaddr_in clientAddr;
    std::memset(&clientAddr, 0, sizeof(clientAddr));
    socklen_t clientAddrLen = sizeof(clientAddr);
    // **-> 이 정보들을 버릴것인지 가지고 다닐 것인지??**

    int clientFd = accept(_socketFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientFd == -1) {
        throw std::runtime_error("ERROR: Accept error");
    }



    // 새로운 클라이언트 -> 새로운 Client 객체 -> Client 객체를 kqueue에 등록
    // 그럼 클라이언트 정보가 들어 있을 구조체(vector 라던지)가 필요할 듯?

    // Client 객체...를 어디에 저장할까요??
    // 1. Client 객체 만들기 ( Client 객체 생성 방법 협의 )
    // 2. Client 객체 Server의 어딘가의 자료구조로 저장하기
    // 3. Client 객체를 kqueue에 등록하기
    

    

}
// 1. Client 객체를 만들어서 Server 클래스 안에서 저장하고 가지고 다닐 것인지?
// 2. Client 객체를 어떤 구조에 저장해 둘 것인지?


void Server::removeClient(Client &client) {
    // 1. kqueue 에서 이벤트를 빼고
    EV_DELETE(&_kq, &client._ev); // KQ fd를 가지고 다닐 서버의 변수 필요.


    // 2. Client 객체를 Server 클래스 안에서 삭제
    // (Client 객체를 어떤 구조에 저장해 둘 것인지?)
    // 3. Client 객체를 삭제


    // Client 클래스가 생각할 것
    // Client 객체 삭제할 때 Client 객체가 가지고 있던 socket을 close
    delete client;
}

void Server::receiveMessage() {
    
}

// 서버가 하나의 클라이언트에게 메세지를 보내는 함수?
// 서버가 전체 클라이언트에게 같은 메세지를 보낼 때의 함수?
// 
void Server::sendMessage(Client &client, std::string message) {
   // Client 객체가 가지고 있는 socket에 메세지를 보내는 함수 
   // Client의 send buffer를 신경쓸까?
   if (client.sendBuffer.length() > 0)
   {
    //먼저 
   }
}
void Server::sendMessage(std::string message) {
    
}

int Server::getPort() const {
    return _port;
}

