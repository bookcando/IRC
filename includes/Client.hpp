#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "./utils/Headers.hpp"

// IRC 서버의 클라이언트. 여러 정보를 저장할 예정
class Client {
private :
    int _clientFd;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _recvBuffer;
    // 리시브 버퍼가 필요한 이유 
    // 1. 클라이언트에서 명령어를 가져올 때, 한 번의 recv 안에 여러 명령어가 들어있을 수 있음 + 뒤에 조금의 명령어 붙어서 올 수 있음 -> 명령어를 보관 후 다음 명령어에 붙여야 함
    // 2. 클라이언트에서 명령어를 가져올 때, 한 번의 recv 안에 명령어가 끊겨서 있을 수 있음 -> 명령어를 보관 후 다음 명령어에 붙여야 함
    // 하여튼 이전에 recv에서 받았던 명령어가 끊겨있을 때 (CR, LF 또는 CRLF가 들어오지 않았을 때) 계속 이어줘야 함 (GNL에서 엔터 들어올 때 까지 계속 입력을 받았던 것 처럼)
    
    std::string _sendBuffer;
    // 센드 버퍼가 필요한 이유
    // 1. 클라이언트로 명령을 보낼 때 send 에러가 난 경우 -> 명령어를 보내지 않고 send가 가능할 때까지 기다렸다가 명령을 수행하도록 해야 할 수 있음
    // -> 그럼 send buffer에 저장한 후 이벤트 루프에서 send가 가능할 때까지 기다렸다가 보내도록 해야 함.

public :
    Client(int clientFd);
    ~Client();

    void resetReadBuffer();
    void resetWriteBuffer();

    void setClientFd(int clientFd);
    void setNickname(std::string nickname);
    void setUsername(std::string username);
    void setRealname(std::string realname);
    void setRecvBuffer(std::string recvBuffer);
    void setSendBuffer(std::string sendBuffer);

    int getClientFd();
    std::string getNickname();
    std::string getUsername();
    std::string getRealname();
    std::string getRecvBuffer();
    std::string getSendBuffer();
    
    int getUserStatus();
};
// 클라이언트의 정보 : 소켓, 포트, fd, 주소
// 클라이언트 유저 정보 : 닉네임, 유저네임, 리얼네임, 모드, (접속한) 채널

#endif



// int Clinet::getUserStatus() {
//     return _status;
// }