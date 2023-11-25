#include "../includes/Server.hpp"
#include "../includes/Validator.hpp"

/*
    Server 클래스 생성자
    1. 포트 검증 및 등록
    2. 패스워드 검증 및 등록
    3. 호스트 이름 및 IP 주소 설정
    4. kqueue 이벤트 알림 시스템 초기화 (kqueue 인스턴스 생성 및 인스턴스에 대한 파일 디스크립터 반환)
*/
Server::Server(std::string port, std::string pass) {
    long long validatedPort;

    // 포트 검증
    validatedPort = Validator::validatePort(port);
    _port = static_cast<int>(validatedPort);
    // 패스워드 검증 및 등록
    Validator::validatePassword(pass);
    _pass = pass;
    // 호스트 이름 및 IP 주소 설정
    settingHostIp();
    // kqueue 이벤트 알림 시스템 초기화 (kqueue 인스턴스 생성 및및 인스턴스에 대한 파일 디스크립터 반환)
    if ((_kqueueFd = kqueue()) == -1)
        throw std::runtime_error("ERROR: Kqueue creation failed");
}

/*
    Server 클래스 소멸자
    1. 클라이언트 리스트에 있는 클라이언트 객체 삭제
    2. kqueue 인스턴스 삭제
    3. 서버 소켓 파일 디스크립터 삭제
*/
Server::~Server() {
}

/*
    서버의 호스트 이름 및 IP 주소 설정
    1. 호스트 이름 가져오기
    2. 호스트 이름에 맞는 호스트 정보 조회
    3. 호스트 정보에서 호스트 주소를 가져와서 IP 주소로 변환 (IPv4)
*/
void Server::settingHostIp() {
    char hostName[1024];
    struct hostent *hostStruct;

    // 호스트 이름 가져오기 (예. c4r6s5.42seoul.kr)
    if (gethostname(hostName, sizeof(hostName)) == -1)
        throw std::runtime_error("ERROR: Hostname error");
    else {
        // 호스트 이름(hostName)에 맞는 호스트 정보 조회
        // 반환되는 hostent 구조체(hostStruct)는 호스트 이름, 별칭, 주소 유형, 주소 길이, 주소 목록 등 정보 포함
        if (!(hostStruct = gethostbyname(hostName)))
            throw std::runtime_error("ERROR: Hostname error");
        else
            // 호스트 정보에서 호스트 주소(hostStruct->h_addr_list[0])를 가져와서 IP 주소로 변환 (IPv4)
            _ip = inet_ntoa(*(struct in_addr*)hostStruct->h_addr_list[0]);
    }
    _host = "irc.localHost.net"; // 추후 확인 필요
}

/*
    서버 초기화
    1. 서버 소켓 생성 및 각종 설정
    2. 새로운 클라이언트 연결을 위한 이벤트 등록
*/
void Server::initializeServer() {
    // 서버 소켓 생성 (AF_INET: IPv4, SOCK_STREAM: TCP, 0: 기본 프로토콜(이 경우 TCP))
    _socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketFd == -1) {
        throw std::runtime_error("ERROR: Socket creation failed");
    }
    // 서버 소켓 주소 설정
    memset(&_serverAddr, 0, sizeof(_serverAddr));
    _serverAddr.sin_family = AF_INET; // 주소체계 IPv4로 설정
    _serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY로 연결된 모든 네트워크 인터페이스에서 연결 수락
    _serverAddr.sin_port = htons(_port); // 서버 포트 _port로 설정
    
    /*
        서버 소켓 옵션 설정
        SOL_SOCKET: 소켓 옵션 레벨
        SO_REUSEADDR: 소켓 재사용 옵션
        &isReuseAddr: 소켓 재사용 옵션 활성 여부
        sizeof(isReuseAddr): 소켓 재사용 옵션 값의 크기
    */
    int isReuseAddr = 1;
    setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &isReuseAddr, sizeof(isReuseAddr));
    
    // 서버 소켓에 주소 설정
    if (bind(_socketFd, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) == -1) {
        throw std::runtime_error("ERROR: Socket bind failed");
    }
    // 서버 소켓을 passive 모드로 설정: 연결 요청 대기
    if (listen(_socketFd, 100) == -1) {
        throw std::runtime_error("ERROR: Socket listen failed");
    }
    // 서버 소켓 논블로킹 모드로 설정: 
    // 만약 요청된 작업을 즉시 완료할 수 없는 경우, 시스템 호출은 대기하지 않고 즉시 에러 코드를 반환: 다른 작업을 수행하거나, 나중에 다시 시도할 수 있음
    fcntl(_socketFd, F_SETFL, O_NONBLOCK);

    /*
        새로운 클라이언트 연결을 위한 이벤트 등록:
        EVFILT_READ: 읽기 가능한 데이터가 있는 경우 (새로운 연결 요청 감지)
        EV_ADD: 이벤트 추가
        EV_ENABLE: 이벤트 활성화
    */
    pushEvents(_newEventFdList, _socketFd, EVFILT_READ, EV_ADD | EV_ENABLE);

    // 서버 가동 플래그 설정
    _isRunning = true;

    // 서버 시작 시간 설정
    _startTime = time(NULL);

    // --- 임시 타임아웃 설정 (추후 삭제) ---
    // _timeout->tv_sec = 3;
    // _timeout->tv_nsec = 0;
}

/*
    (여기부터 코드 채워넣기 필요. 일단 에코서버 기준으로 되어있던 코드는 주석처리.
    handleReadEvent 등 내부에서 사용되는 함수들 실제 구현 필요)
    서버 동작부:
    1. 새로운 이벤트가 발생할 때까지 이벤트 대기
    2. 새로운 이벤트가 발생한 경우, 이벤트 처리
    3. 새로운 이벤트 처리 끝난 후, 연결이 끊긴 클라이언트 처리
*/
void Server::runServer() {
    int eventCount = 0;

    std::cout << "Server loop started" << std::endl;
    while (_isRunning) {
        std::cout << "Waiting for events ..." << std::endl;
        //memset(&_kEventList, 0, sizeof(_kEventList)); // 필요한지 모르겠음

        /*
            kevent 역할: 이벤트 변경 또는 감지
            등록할 이벤트 목록(_newEventFdList)과 발생한 이벤트를 저장할 배열(_kEventList)을 인자로 받음. 이를 통해 이벤트를 kqueue에 추가 및 변경, 발생한 이벤트 감지
            이후 발생한 이벤트 수를 반환하며, 이를 통해 프로그램은 어떤 이벤트가 발생했는지 파악 가능
        */
        eventCount = kevent(_kqueueFd, &_newEventFdList[0], _newEventFdList.size(), _kEventList, 100, NULL); // NULL: 블로킹 모드로 설정 (이벤트 발생까지 대기)
        // 이미 kqueue에 이벤트가 등록되었으므로, 이벤트 목록 비우기
        _newEventFdList.clear();

        // 발생한 이벤트 수 만큼 루프
        for (int i = 0; i < eventCount; i++) {
            // _kEventList는 발생한 이벤트를 저장한 배열 (위에서 kevent 함수를 통해 이벤트를 감지하고 저장한 배열)
            // cur: 현재 이벤트
            struct kevent cur = _kEventList[i];
            if (cur.flags & EV_ERROR) { // 이벤트에 오류 플래그가 설정되어 있는지 확인
                if (isServerEvent(cur.ident)) { // 오류 이벤트가 서버 소켓과 관련된 것인지 확인
                    _isRunning = false; // 서버 이벤트인 경우 서버 자체의 오류이므로 서버 종료
                    break; // 루프 종료
                }
                else {
                    deleteClient(cur.ident); // 오류 이벤트가 클라이언트와 관련된 것이면 해당 클라이언트 삭제
                }
            }
            if (cur.flags & EVFILT_READ) { // 읽기 이벤트인지 확인
                if (isServerEvent(cur.ident)) { // 읽기 이벤트가 서버 소켓과 관련된 것인지 확인
					addClient(); // 새 클라이언트 연결 요청 처리
				}
				if (this->containsCurrentEvent(cur.ident)) { // 현재 이벤트가 처리 목록에 있는지 확인
					handleReadEvent(cur.ident, cur.data); // 클라이언트로부터의 데이터 읽기 처리
				}
            }
            if (cur.ident & EVFILT_WRITE) { // 쓰기 이벤트인지 확인
				//std::cout << "EVFILT_WRITE" << std::endl;
				if (this->containsCurrentEvent(cur.ident)) { // 현재 이벤트가 처리 목록에 있는지 확인
					handleWriteEvent(cur.ident); // 클라이언트에 데이터 쓰기 처리
				}
			}
        }
    	// 모든 새 이벤트에 대한 처리가 끝난 후, 연결이 끊긴 클라이언트를 처리
        handleDisconnectedClients();
    }
}

bool Server::isServerEvent(uintptr_t ident) {
	return (ident == (size_t)_socketFd); // size_t 형변환은 필요한지 확인 필요
}

void Server::deleteClient(int fd) {
    // 만약 현재 작업 중인 클라이언트가 삭제될 클라이언트와 같다면, 작업 중인 클라이언트 참조를 NULL로 설정
    if (_op == _clientList[fd])
        _op = NULL;

    // 클라이언트 객체 삭제
    delete _clientList[fd];

    // 해당 클라이언트의 읽기 및 쓰기 버퍼를 버퍼 관리 객체에서 제거
    // hyojocho 가 만드는 중
    // Buffer::eraseReadBuf(fd);
    // Buffer::eraseSendBuf(fd);

    // 클라이언트 목록에서 해당 클라이언트 제거
    _clientList.erase(fd);

    // 연결 해제된 클라이언트에 대한 정보를 로그로 출력
    //Print::PrintComplexLineWithColor("[" + getStringTime(time(NULL)) + "] " + "Disconnected Client : ", fd, RED);
}

bool Server::containsCurrentEvent(uintptr_t ident) {
	return (_clientList.find(ident) != _clientList.end());
}

void Server::handleReadEvent(int fd, intptr_t data) {
    // 구현 추가
}

void Server::handleWriteEvent(int fd) {
    // 구현 추가
}

void Server::handleDisconnectedClients() {
    // 구현 추가
}

void Server::pushEvents(EventList &eventFdList, uintptr_t fd, short filter, u_short flags) {
    struct kevent event;

    EV_SET(&event, fd, filter, flags, 0, 0, this);
    // kevent(_kqueueFd, &event, 1, NULL, 0, NULL);
    eventFdList.push_back(event);
}

void Server::addClient() {
    int clientFd;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen;

    std::memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddrLen = sizeof(clientAddr);

    if ((clientFd = accept(_socketFd, (struct sockaddr*)&clientAddr, &clientAddrLen)) == -1)
        throw std::runtime_error("ERROR: Client accept error");
    _clientList.insert(std::make_pair(clientFd, new Client(clientFd)));
    pushEvents(_newEventFdList, clientFd, EVFILT_READ | EVFILT_WRITE,  EV_ADD | EV_ENABLE);
    _clientList[clientFd]->resetReadBuffer();
    _clientList[clientFd]->resetWriteBuffer();
    fcntl(clientFd, F_SETFL, O_NONBLOCK);
    std::cout << "Client is connected" << std::endl;
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
            //Message::parseMassages();
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

