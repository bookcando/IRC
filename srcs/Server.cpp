#include "../includes/Server.hpp"
#include "../includes/Validator.hpp"
#include "../includes/Buffer.hpp"
#include "../includes/Message.hpp"
#include "../includes/Command.hpp"
#include "../includes/utils/utils.hpp"
#include "../includes/Lists.hpp"

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
    // 서버 호스트 패스워드, 이름
    _opName = "admin";
    _opPassword = "admin";
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
	Lists::clearClientList();
	Lists::clearChannelList();
    close(_socketFd);
    close(_kqueueFd);
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
    pushEvents(_newEventFdList, _socketFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

    // 서버 가동 플래그 설정
    _isRunning = true;

    // 서버 시작 시간 설정
    _startTime = time(NULL);

    // --- 임시 타임아웃 설정 (추후 삭제) ---
    _timeout.tv_sec = 0;
    _timeout.tv_nsec = 500000000;
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
    struct timespec time;

    memset(&time, 0, sizeof(time));
    std::cout << "Server loop started" << std::endl;
    while (_isRunning) {
        // std::cout << "Waiting for events ..." << std::endl;
        /*
            kevent 역할: 이벤트 변경 또는 감지
            등록할 이벤트 목록(_newEventFdList)과 발생한 이벤트를 저장할 배열(_kEventList)을 인자로 받음. 이를 통해 이벤트를 kqueue에 추가 및 변경, 발생한 이벤트 감지
            이후 발생한 이벤트 수를 반환하며, 이를 통해 프로그램은 어떤 이벤트가 발생했는지 파악 가능
        */
        eventCount = kevent(_kqueueFd, NULL, 0, _kEventList, 100, &_timeout); // NULL: 블로킹 모드로 설정 (이벤트 발생까지 대기)

        // std::cout << "eventCount: " << eventCount << std::endl;
        if (eventCount == -1)
            throw std::runtime_error("ERROR: Kevent error"); // 이벤트 발생 실패 시 예외 발생 (이벤트 발생 실패 시는 없을 것 같음)
        // 이미 kqueue에 이벤트가 등록되었으므로, 이벤트 목록 비우기
        _newEventFdList.clear();

        // 발생한 이벤트 수 만큼 루프
        for (int i = 0; i < eventCount; i++) {
            // _kEventList는 발생한 이벤트를 저장한 배열 (위에서 kevent 함수를 통해 이벤트를 감지하고 저장한 배열)
            // cur: 현재 이벤트
            struct kevent cur = _kEventList[i];
            if (cur.flags & EV_ERROR) { // 이벤트에 오류 플래그가 설정되어 있는지 확인
                if (isServerEvent(cur.ident)) { // 오류 이벤트가 서버 소켓과 관련된 것인지 확인
                    std::cout << "Server error" << std::endl;
                    _isRunning = false; // 서버 이벤트인 경우 서버 자체의 오류이므로 서버 종료
                    break; // 루프 종료
                }
                else {
                    std::cout << "Client error" << std::endl;
                    deleteClient(cur.ident); // 오류 이벤트가 클라이언트와 관련된 것이면 해당 클라이언트 삭제
                    break ; // 루프 종료
                }
            }
            else if (cur.flags & EVFILT_READ) { // 읽기 이벤트인지 확인
                
                std::cout << "EVFILT_READ" << std::endl;
                if (isServerEvent(cur.ident)) { // 읽기 이벤트가 서버 소켓과 관련된 것인지 확인
                    // std::cout << "New client" << std::endl;
					addClient(cur.ident); // 새 클라이언트 연결 요청 처리
				}
				else if (containsCurrentEvent(cur.ident)) { // 현재 이벤트가 처리 목록에 있는지 확인
                    // std::cout << "New read event" << std::endl;
					handleReadEvent(cur.ident, cur.data, _host); // 클라이언트로부터의 데이터 읽기 처리
				} // cur.ident: 이벤트가 발생한 파일 디스크립터, cur.data: 읽어온 데이터 크기, _host: 서버의 호스트 이름
            }
            else if (cur.flags & EVFILT_WRITE) { // 쓰기 이벤트인지 확인 (-> cur.ident에서 수정함: 확인 필요)
				std::cout << "EVFILT_WRITE" << std::endl;
				if (containsCurrentEvent(cur.ident)) { // 현재 이벤트가 처리 목록에 있는지 확인
					handleWriteEvent(cur.ident); // 클라이언트에 데이터 쓰기 처리
				}
			}
        }
    	// 모든 새 이벤트에 대한 처리가 끝난 후, 연결이 끊긴 클라이언트를 처리
        // handleDisconnectedClients();
    }
}

bool Server::isServerEvent(uintptr_t ident) {
	return (ident == (size_t)_socketFd); // size_t 형변환은 필요한지 확인 필요
}

void Server::deleteClient(int fd) {
    // 만약 현재 작업 중인 클라이언트가 삭제될 클라이언트와 같다면, 작업 중인 클라이언트 참조를 NULL로 설정
    // if (_op == _clientList[fd])
        // _op = NULL;
    if (_op == &Lists::findClient(fd))
        _op = NULL;

    // 클라이언트 객체 삭제
    // delete _clientList[fd];
    // delete &Lists::findClient(fd);

    // 해당 클라이언트의 읽기 및 쓰기 버퍼를 버퍼 관리 객체에서 제거
    Buffer::eraseReadBuffer(fd);
    Buffer::eraseWriteBuffer(fd);

    // 클라이언트 목록에서 해당 클라이언트 제거
    // _clientList.erase(fd);
    Lists::deleteClientList(fd);

    // 연결 해제된 클라이언트에 대한 정보를 로그로 출력
    //Print::PrintComplexLineWithColor("[" + getStringTime(time(NULL)) + "] " + "Disconnected Client : ", fd, RED);
}

bool Server::containsCurrentEvent(uintptr_t ident) {
	// return (_clientList.find(ident) != _clientList.end());
    return (Lists::hasClient(ident));
}

// Server 클래스의 메서드: 읽기 이벤트 처리
void Server::handleReadEvent(int fd, intptr_t data, std::string host) {
    std::string buffer;
    std::string message;
    size_t size = 0;
    int cut;

    // 메시지의 유효성을 확인합니다. 유효하지 않은 경우 클라이언트를 삭제합니다.
    if (Validator::validateMessage(fd, data) == false) { 
        std::cout << "validateMessage:: invalid" << std::endl;
        deleteClient(fd); // 클라이언트 삭제
        return; // 함수 종료
    }

    buffer = Buffer::getReadBuffer(fd); // 클라이언트로부터 읽은 데이터를 가져옵니다.
    Buffer::resetReadBuffer(fd); // 읽기 버퍼를 초기화합니다.

    // 무한 루프를 통해 버퍼 내의 모든 메시지를 처리합니다.
    while (1) {
        // 줄바꿈 문자("\r\n", "\r", "\n")를 찾아 메시지를 구분합니다.
        if ((size = buffer.find("\r\n")) != std::string::npos) {
            cut = size + 2; // "\r\n"을 포함하여 자릅니다.
        } else if ((size = buffer.find("\r")) != std::string::npos || (size = buffer.find("\n")) != std::string::npos) {
            cut = size + 1; // "\r" 또는 "\n"만 포함하여 자릅니다.
        } else {
            break; // 줄바꿈 문자가 없으면 루프를 종료합니다.
        }
        /*
        메시지 추출 예시: "메시지1\n메시지2\n메시지3\n" 입력 시,
        
        message = "메시지1\n"
        buffer  = "메시지2\n메시지3\n"
        */

        message = buffer.substr(0, cut);
        buffer = buffer.substr(cut, buffer.size()); // 나머지 버퍼를 업데이트합니다.

        // 메시지 길이 제한 검사 (512 바이트)
        if (message.size() > 512) {
            Buffer::sendMessage(fd, Error::ERR_INPUTTOOLONG(host)); // 메시지가 너무 길면 오류 메시지를 전송합니다.
            continue;
        }

        // 메시지를 파싱하고, 유효한 경우 명령을 실행합니다.
        if (Message::parseMessage(message)) {
            executeCommand(fd); // 명령 실행 
        }
    }
    // 남은 버퍼를 다시 설정합니다.
    Buffer::setReadBuffer(std::make_pair(fd, buffer));
}

void Server::handleWriteEvent(int fd) {
    Buffer::sendMessage(fd);
}

void Server::handleDisconnectedClients() {
    time_t cur = time(NULL); // 현재 시간을 받아옵니다.
    std::vector<int> deleteList; // 연결 종료할 클라이언트의 파일 기술자를 저장할 벡터를 선언합니다.
    std::string timeout = "Quit :timeout\r\n"; // 타임아웃 메시지를 정의합니다.
    ClientMap& cltList = Lists::getClientList(); // 클라이언트 목록을 가져옵니다.

    Message::parseMessage(timeout); // 타임아웃 메시지를 파싱합니다.
    // 모든 클라이언트를 순회하며 타임아웃 여부를 검사합니다.
    for (ClientMap::const_iterator it = cltList.begin(); it != cltList.end(); it++) {
        // 클라이언트의 마지막 활동 시간으로부터 현재까지의 시간 차이가 120초(2분)를 초과하는 경우
        if (cur - it->second->getTime() > 60)
            deleteList.push_back(it->second->getClientFd()); // 해당 클라이언트를 삭제 목록에 추가합니다.
    }
    // 삭제 목록의 각 클라이언트에 대해 연결 종료 처리를 수행합니다.
    for (size_t i = 0; i < deleteList.size(); i++) {
        Command::quit(*cltList[deleteList[i]]); // 클라이언트의 연결을 종료합니다.
        deleteClient(deleteList[i]); // 클라이언트를 서버 목록에서 제거합니다.
    }
}

void Server::pushEvents(EventList &eventFdList, uintptr_t fd, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata) {
    struct kevent event;

    EV_SET(&event, fd, filter, flags, fflags, data, udata);
    kevent(_kqueueFd, &event, 1, NULL, 0, NULL);
    eventFdList.push_back(event);
}

void Server::addClient(int fd) {
    int clientFd; // 새 클라이언트의 소켓
    struct sockaddr_in clientAddr; // 클라이언트 주소 정보를 저장할 구조체
    socklen_t clientSize; // 클라이언트 주소 정보의 크기

    clientSize = sizeof(clientAddr); // 클라이언트 주소 구조체의 크기를 설정
    // 새 클라이언트 연결 수락
    if ((clientFd = accept(fd, (struct sockaddr*)&clientAddr, &clientSize)) == -1)
        throw std::runtime_error("Error : accept!()"); // 연결 수락 실패 시 예외 발생

    // 클라이언트 소켓에 대한 읽기 이벤트를 이벤트 리스트에 추가
    pushEvents(_newEventFdList, clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    // 클라이언트 소켓에 대한 쓰기 이벤트를 이벤트 리스트에 추가
    pushEvents(_newEventFdList, clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
    // 쓰기 이벤트는 한 번만 검출되고 더 이상 검출되지 않음(DISPATCH = 삭제, EV_ONESHOT = 한 번만 검출)
    // -> 쓰기 이벤트가 필요하면 다시 추가해야 함
    
    // 클라이언트 목록에 새 클라이언트 추가

    // _clientList.insert(std::make_pair(clientFd, new Client(clientFd, clientAddr.sin_addr)));
    Lists::addClientList(clientFd, clientAddr.sin_addr);

    
    // 클라이언트 소켓의 읽기 및 쓰기 버퍼 초기화
    Buffer::resetReadBuffer(clientFd);
    Buffer::resetWriteBuffer(clientFd);

    // 클라이언트 소켓을 논블로킹 모드로 설정
    fcntl(clientFd, F_SETFL, O_NONBLOCK);

    // 새로 연결된 클라이언트에 대한 정보 출력
    std::cout << "client connected : " << clientFd << std::endl; //Print::PrintComplexLineWithColor("[" + getStringTime(time(NULL)) + "] " + "Connected Client : ", clientFd, GREEN);
}

void Server::removeClient(int clientFd) {
    Client *temp = &Lists::findClient(clientFd);
    Lists::deleteClientList(clientFd);
    delete (temp);
    temp = NULL;
    close (clientFd);
}

void Server::executeCommand(int fd) {
    bool killFlag; // 클라이언트 삭제 여부를 결정하는 플래그
    int killFd; // 삭제할 클라이언트의 파일 디스크립터

    // std::cout << "executeCommand" << std::endl;
    #ifdef DEBUG
    std::cout << "fd: " << fd << " ";
    std::cout << "Message: ";
    for (unsigned int i = 0; i < Message::getMessage().size(); i++) {
        std::cout << Message::getMessage()[i] << " ";
    }
    std::cout << std::endl;
    #endif

    switch (Command::checkCommand()) { // 받은 명령어를 확인
        case IS_PASS:
            Command::pass(Lists::findClient(fd), _pass, _host); // PASS 명령어 처리
            break;
        case IS_NICK:
            Command::nick(Lists::findClient(fd), _host); // NICK 명령어 처리
            break;
        case IS_USER:
            Command::user(Lists::findClient(fd), _host, _ip, _startTime); // USER 명령어 처리
            break;
        case IS_PING:
            Command::ping(Lists::findClient(fd), _host); // PING 명령어 처리
            break;
        case IS_PONG:
            Lists::findClient(fd).setTime(); // PONG 명령어 처리, 클라이언트 활동 시간 업데이트
            break;
        case IS_MODE:
            Command::mode(Lists::findClient(fd), _host); // MODE 명령어 처리
            break;
        case IS_JOIN:
            Command::join(Lists::findClient(fd), _host); // JOIN 명령어 처리
            break;
        case IS_PART:
            Command::part(Lists::findClient(fd), _host); // PART 명령어 처리
            break;
        case IS_KICK:
            Command::kick(Lists::findClient(fd), _host); // KICK 명령어 처리
            break;
        case IS_INVITE:
            Command::invite(Lists::findClient(fd), _host); // INVITE 명령어 처리
            break;
        case IS_TOPIC:
            Command::topic(Lists::findClient(fd), _host); // TOPIC 명령어 처리
            break;
        case IS_PRIVMSG:
            Command::privmsg(Lists::findClient(fd), _host); // PRIVMSG 명령어 처리
            break;
        case IS_NOTICE:
            Command::notice(Lists::findClient(fd)); // NOTICE 명령어 처리
            break;
        case IS_QUIT:
            Command::quit(Lists::findClient(fd)); // QUIT 명령어 처리
            deleteClient(fd); // 클라이언트 삭제
            break;
        case IS_OPER:
            // OPER 명령어 처리, 운영자 권한 부여
            if (Command::oper(Lists::findClient(fd), _opName, _opPassword, _host, _op != NULL ? true : false))
                _op = &Lists::findClient(fd);
            break;
        case IS_KILL:
            // KILL 명령어 처리
            if (_op == NULL || fd != _op->getClientFd())
                killFlag = false; // 운영자가 아니면 killFlag를 false로 설정
            else
                killFlag = true; // 운영자이면 killFlag를 true로 설정
            if ((killFd = Command::kill(Lists::findClient(fd), _host, killFlag))) {
                Command::quit(Lists::findClient(killFd)); // 특정 클라이언트 강제 종료
                deleteClient(killFd); // 클라이언트 삭제
            }
            break;
        case IS_NOT_ORDER:
            // 알려지지 않은 명령어 처리
            Buffer::sendMessage(fd, Error::ERR_UNKNOWNCOMMAND(_host, (Message::getMessage())[0]));
            break;
    };
}

std::string const& Server::getHost() const {
    return _host;
}
