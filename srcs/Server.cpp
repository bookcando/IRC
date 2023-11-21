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
    std::cout << "Check 1.1" << std::endl;
    // pushEvents
    pushEvents(_newEventFdList, _socketFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    // 2. 소켓 옵션 설정
    int opt = 1;
    setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // 3. 소켓 바인드(IP, Port)
    if (bind(_socketFd, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) == -1) {
        throw std::runtime_error("ERROR: Socket bind failed");
    }
    std::cout << "Check 1.2" << std::endl;
    // 4. 소켓 리슨 -> 이제 받기 시작하도록
    if (listen(_socketFd, 100) == -1)
        throw std::runtime_error("ERROR: Socket listen failed");
    std::cout << "Check 1.3" << std::endl;
    _kqueueFd = kqueue();
    std::cout << "Check 1.4" << std::endl;
    // std::memset(&_changeList, 0, sizeof(_changeList));
    std::memset(&_kEventList, 0, sizeof(_kEventList));
    std::cout << "Check 1.5" << std::endl;
    // EV_SET(&_changeList[0], _socketFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    // EV_SET(&_changeList[0], _socketFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    // EV_SET(&_newEventFdList[0], _socketFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    //EV_SET arguments : struct kevent *kevp, uintptr_t ident, short filter, u_short flags, u_int fflags, intptr_t data, void *udata
    //
    std::cout << "Check 1.6" << std::endl;

    _timeout->tv_sec = 3;
    _timeout->tv_nsec = 0;
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



    //struct timespec {
    //    time_t tv_sec; /* seconds */
    //    long   tv_nsec; /* nanoseconds */
    //};


    // EV_SET(&_ev, _socketFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    int flag = 1; // flag를 
    int newEvent;
    while (flag)
    {
        std::cout << "Check 2.1" << std::endl;
        std::cout << "_newEventFdList.size() : " << _newEventFdList.size() << std::endl;
        newEvent = kevent(_kqueueFd, &_newEventFdList[0], _newEventFdList.size(), _kEventList, 15, _timeout);
        _newEventFdList.clear();
        // 여기서는 kevent를 사용하여 이벤트를 대기.

        // changeList : 이벤트를 등록하거나 삭제할 때 사용하는 구조체 : EV_SET을 통해 등록
        // EV_SET 으로 계속 들고다니면서 여기에 등록을 함.
        // pollfds의 event

        // nChanges : changeList에 등록된 이벤트의 개수
        // eventList : 이벤트가 발생했을 때 정보를 담는 구조체

        // nEvents : eventList에 등록된 이벤트의 개수
        // timeout : 이벤트가 발생할 때까지 대기하는 시간
        // 이벤트가 발생하면 발생한 이벤트의 개수를 반환하고, timeout이 지나면 0을 반환한다.
        // 이벤트가 발생하지 않으면 -1을 반환하고 errno에 오류 코드를 저장한다.

        if (newEvent == -1) {
            throw std::runtime_error("ERROR: Kqueue event error");
        }
        for (int i = 0; i < newEvent; i++) {
            // 현재 이벤트의 정보
            std::cout << "i : " << i << std::endl;
            std::cout << "main socket fd : " << _socketFd << std::endl;
            std::cout << "fd : " << _kEventList[i].ident << std::endl;
            std::cout << "filter : " << _kEventList[i].filter << std::endl;
            std::cout << "flags : " << _kEventList[i].flags << std::endl;

            if (_kEventList[i].flags & EV_ERROR) {
                std::cout << "ERROR: Master socket error" << std::endl;
            }
            else if (_kEventList[i].flags & EVFILT_READ) {
                std::cout << "Check 2.3.0" << std::endl;
                if (_kEventList[i].ident == static_cast<uintptr_t>(_socketFd)) {
                    std::cout << "Check 2.3.1" << std::endl;
                    addClient();
                    std::cout << "Check 2.3.2" << std::endl;
                }
                else if (_clientList.find(_kEventList[i].ident) != _clientList.end()) {
                    std::cout << "Check 2.3.-1" << std::endl;
                    receiveMessage(_kEventList[i].ident);
                    std::cout << "Check 2.3.-2" << std::endl;
                }
                else {
                    std::cout << "ERROR: Unknown socket error" << std::endl;
                }
            }
            else if (_kEventList[i].flags & EVFILT_WRITE) {
                if (_clientList.find(_kEventList[i].ident) != _clientList.end()) {
                    sendMessage(*_clientList[_kEventList[i].ident]);
                }
                else {
                    std::cout << "ERROR: Unknown socket error" << std::endl;
                }
            }
            else if (_kEventList[i].flags & EVFILT_WRITE) {
                std::cout << "Check 2.3.8" << std::endl;

                sendMessage(*_clientList[_kEventList[i].ident]); // Server에서 eventList의 fd를 보고
                std::cout << "Check 2.3.9" << std::endl;
            }
            std::cout << "Check 2.3.10" << std::endl;
        }
        std::cout << "Check 2.5" << std::endl;
        sleep(2); 
        std::cout << "end of while" << std::endl;
    }
}

void Server::pushEvents(EventList &eventFdList, uintptr_t fd, short filter, u_short flags, u_int fflags, intptr_t data, void *udata) {
    struct kevent event;

    EV_SET(&event, fd, filter, flags, fflags, data, udata);
    eventFdList.push_back(event);
}

void Server::addClient() {
    int clientFd;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen;

    std::memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddrLen = sizeof(clientAddr);

    // 새 클라이언트 연결 수락
    if ((clientFd = accept(_socketFd, (struct sockaddr*)&clientAddr, &clientAddrLen)) == -1)
        throw std::runtime_error("ERROR: Client accept error");
    // 클라이언트 정보 저장
    _clientList.insert(std::make_pair(clientFd, new Client(clientFd)));
    // 읽기 및 쓰기 이벤트 등록
    pushEvents(_newEventFdList, clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    pushEvents(_newEventFdList, clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
    // 버퍼 초기화
    _clientList[clientFd]->resetReadBuffer();
    _clientList[clientFd]->resetWriteBuffer();
    // 소켓 설정
    fcntl(clientFd, F_SETFL, O_NONBLOCK);
    send (clientFd, "hello\n", 6, 0);
}



void Server::removeClient(int clientFd) {
    Client *temp = _clientList[clientFd];
    _clientList.erase(clientFd);
    delete (temp);
    close (clientFd);

    // 소멸자에 넣을 수 있는 것은 다 거기에 몰아 넣자 :D
    // ㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋ

    // 2. Client 객체를 Server 클래스 안에서 삭제
    // iterator 사용하여 _clientList 에서 Client 객체를 찾아서 삭제

    // (Client 객체를 어떤 구조에 저장해 둘 것인지?)
    // 3. Client 객체를 삭제

    // Client 클래스가 생각할 것
    // Client 객체 삭제할 때 Client 객체가 가지고 있던 socket을 close
}




void Server::receiveMessage(int clientFd) {
// 클라이언트 한테서 읽을 게 있다! socket에 있는 버퍼에 읽을 게 있으니까 
// IRC 명세에 맞는 메세지가 들어왔으니 실행을 해 달라!
    // Client *currentClient = _clientList[clientFd];

// 1. Client 객체가 가지고 있는 socket에서 메세지를 받아서
// 2. Client 객체가 가지고 있는 recv buffer + 현재 받은 메세지 -> Server의 recv buffer에 넣기
// 3. Server의 recv buffer에서 메세지를 하나씩 빼서(CRLF, CR, LF 기준으로 자른다. 다른 whitespace는 구분자로 사용되어서는 안됨)
// 4. Server의 command buffer에 넣기
// 5. Server의 command parser에 넣고 돌리기 : 물론 커맨드에도 클라이언트 정보 필요하니까
// 클라이언트 정보도 같이 보내기 (클라이언트가 방장이 된다든지, 채널에 참여한다든지 등)
// 6. command 실행 중 -> send가 실패하면 send buffer에 넣기
// 그리고 Event에서 정보 전송 가능 event 감지 후 (한번만 감지하는 옵션) send buffer에서 메세지를 빼서 보내기
// 7. command 실행에서 send 되면 다음 command 실행 -> 3번으로 가세요.

    char recvBuffer[1024] = {0,};
    // -> recv 가 char 만 받아서 string 못받음!
    int recvLength = recv(clientFd, recvBuffer, 1020, 0); // recvBuffer는 Server의 recv buffer
    recvBuffer[recvLength] = '\0';
    std::cout << "recvBuffer : " << recvBuffer << std::endl;
    send(clientFd, recvBuffer, recvLength, 0);
    if (recvLength == -1) {
        // throw std::runtime_error("ERROR: Receive error");
        // 리시브가 실패하는 경우 : 다시 읽으려 노력해볼것인가? 이를 생각해야 함.

        // **
        // 중요한 오류처리 부분 : 함께 고민해봐요~
        // **
        /*


        recv의 return이 -1인 경우
        1. recv 에서 읽을 것이 없어서 -1 인 경우(신호는 들어왔지만 아직 메세지가 들어오지는 않았음) : 다시 읽기를 바람?
        2. recv 자체의 오류 : 다시 실행?
        이를 어떻게 구분하여야 하며, 다시 실행해야 하는가?
        recv에서 오류가 나면 지금까지 읽어온 부분은 어떻게 해야 하는가?

        */

    }
    else if (recvLength == 0) {
        // 접속 종료
        // removeClient(_clientList[clientFd]);
        // removeClient(_clientList[clientFd]);
    
    }
    // std::string totalMessage = _clientList[clientFd].getRecvBuffer() + recvBuffer;
    recvBuffer[recvLength] = '\0';
    std::string totalMessage;
    if (_clientList[clientFd]->getRecvBuffer().size() > 0) {
        totalMessage = _clientList[clientFd]->getRecvBuffer() + recvBuffer;
        _clientList[clientFd]->setRecvBuffer("");
    }
    else {
        totalMessage = recvBuffer;
        return ;
    }


    // 이제 totalMessage에서 메세지를 하나씩 빼서 command buffer에 넣기
    // 이 때, CRLF, CR, LF를 기준으로 자르기
    // 다른 whitespace는 구분자로 사용되어서는 안됨
    // std::size_t crlfPos = totalMessage.find("\r\n"); // CRLF 가 있으면 CR이나 LF는 무조건 있음
    std::size_t crPos = totalMessage.find("\r");
    std::size_t lfPos = totalMessage.find("\n");
    std::size_t pos = 0;
    std::size_t denominatorLength = 0;
    std::string command;

    if (crPos == std::string::npos && lfPos == std::string::npos) {
        // CR, LF가 없는 경우 (그럼 당연히 CRLF는 없다)
        // 그냥 totalMessage를 command buffer에 넣고 끝낸다. 다음 recv/accept를 하러 가 보자구~
        _clientList[clientFd]->setRecvBuffer(totalMessage);
    }
    else {
        // CRLF, CR, LF가 있는 경우
        // CRLF, CR, LF 중 가장 먼저 나오는 것을 찾아서 그 전까지를 command buffer에 넣고
        // 나머지는 다시 totalMessage에 넣고 다시 찾기

        // 그래서 하나라도 npos가 아닌 경우,
        // crlfPos 는 cr이 없거나 lf가 없으면 당연히 없음
        while (crPos != std::string::npos || lfPos != std::string::npos) {

            if (crPos != std::string::npos && lfPos != std::string::npos) {
                if (lfPos + 1 == crPos) { // CRLF인 경우
                    denominatorLength = 2;
                    pos = crPos;
                }
                else { // CRLF가 아닌경우 : CR 또는 LF만 있음
                    denominatorLength = 1;
                    pos = std::min(crPos, lfPos);
                }
                pos = std::min(crPos, lfPos);
            } // crlf cr lf
            else if (crPos != std::string::npos) {
                denominatorLength = 1;
                pos = crPos;
            } // cr
            else if (lfPos != std::string::npos) {
                denominatorLength = 1;
                pos = lfPos;
            } // lf
            //CRLF가 있는 경우에는 CR 과 LF 모두 당연히 있음

            // pos는 CRLF, CR, LF 중 가장 먼저 나오는 것의 위치
            // pos 전까지를 command buffer에 넣고
            // pos + denominatorLength 부터 totalMessage 끝까지를 다시 totalMessage에 넣고 나서...

            //그럼 다음 커맨드를 위한 준비도 완료 + 커맨드 파싱해서 실행하면 되겠다!

            _clientList[clientFd]->setRecvBuffer(totalMessage.substr(pos + denominatorLength));
            command = totalMessage.substr(0, pos);
            // 이 커맨드 파싱하고 실행시키면 됨.
            // runCommand(command, clientObject);
            //++PARSING;
            //++RUN_COMMAND;
            //파싱안해~
            // dkham이 다 하자~
            //echo 부분
            int sendLength;
            sendLength = send(clientFd, (command + "/r/n").c_str(), (command + "/r/n").size(), 0);
            if (sendLength < 0) {
                std::cout << "ERROR: send error in recvMessage" << std::endl;
            }
            //여기서 오류나서 send가 안되면 send 버퍼에 다 남아있는데 그거 나중에 언젠가는 비워줘야하니까!


        }
    }
}
//1. crlf가 있음 -> cr 도 lf 도 검출됨
//2-1. crlf가 없음 -> cr만 검출
//2-2. crlf가 없음 -> lf만 검출
//3. cr, lf가 없음 -> 메세지 버퍼에 집어넣기

// 서버가 하나의 클라이언트에게 메세지를 보내는 함수?
// 서버가 전체 클라이언트에게 같은 메세지를 보낼 때의 함수?
// 
void Server::sendMessage(Client &client) {
   // Client 객체가 가지고 있는 socket에 메세지를 보내는 함수 

    // 어차피 client에 보낼 메세지들은 이미 명령어 실행하면서 보냈겠지만
    // 만약 send 오류로 그 명령들이 client의 send buffer에 남아있는 경우가 있을 수 있음
    // -> 그 때에만 사용될 함수임. : send buffer에 남아있는 메세지를 보내고 send buffer를 비워주는 함수.

    int sendLength = 0;
    // send buffer에 남아있는 메세지가 있는지 확인
    if (client.getSendBuffer().size()) {
        sendLength = send(client.getClientFd(), client.getSendBuffer().c_str(), client.getSendBuffer().size(), 0);
        if (sendLength == -1) {
            //send에서 다시 오류가 난 경우...
        }
        else {
            //send에서 sendLength만큼 잘 전달한 경우
            // 근데 sendLength와 sendBufferLength를 비교해서 전부 전달 되었는지 확인해야 하나??
            client.setSendBuffer("");
        }
        
    }
    else {
        // 아무것도 하지 말자. 한 번 더 기다려!
        // write event가 없는데 쓸 수 있어서 뭐하니?
    }
}
// void Server::sendMessage(std::string message) {
    
// }

int Server::getPort() const {
    return _port;
}

