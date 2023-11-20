#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <string>

// IRC 서버의 클라이언트. 여러 정보를 저장할 예정
class Client {
    // private :
    public :
        // Client();
        // ~Client();
        // Client(const Client &ref);
        // Client &operator=(const Client &ref);

        int     port;
        int     fd;
        char    *addr;

        std::string recvBuffer;
        std::string sendBuffer;
};
// 클라이언트의 정보 : 소켓, 포트, fd, 주소
// 클라이언트 유저 정보 : 닉네임, 유저네임, 리얼네임, 모드, (접속한) 채널
#endif