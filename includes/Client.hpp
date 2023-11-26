#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "./utils/Headers.hpp"
#include "./utils/Containers.hpp"
#include "./Lists.hpp"

// IRC 서버의 클라이언트. 여러 정보를 저장할 예정
class Client {
private :
    int _passConnect; // PASS, NICK, USER 전부를 거쳤는 지 검증. 비트마스킹.
    bool _operator;
    time_t _finalTime;
    int _clientFd;
    in_addr _info;
    std::string _host;
    std::string _nickname;
    std::string _realname;
    std::string _username;
    std::string _server;
    ChannelMap _joinList;
    std::string _recvBuffer;
    std::string _sendBuffer;
public :
    Client(int clientFd, in_addr info);
    ~Client();

    void resetReadBuffer();
    void resetWriteBuffer();

    void setNickname(std::string nickname);
    void setRealname(std::string realname);
    void setHost(std::string host);
    void setClientFd(int clientFd);
    void setUsername(std::string username);
    void setServer(std::string server);
    void setTime(std::string time);
    void setRecvBuffer(std::string recvBuffer);
    void setSendBuffer(std::string sendBuffer);

	void addJoinList(Channel* channel);
	void deleteJoinList(Channel* channel);
	int joinChannel(Channel* channel, std::string const& key);

	int getPassConnect() const;
	int getClientFd() const;
	bool getOperator() const;
	std::string const& getHostname() const;
	std::string const& getNickname() const;
	std::string const& getRealname() const;
	std::string const& getUsername() const;
	std::string const& getServer() const;
	time_t const& getTime() const;
	in_addr const& getAddr() const;

};
// 클라이언트의 정보 : 소켓, 포트, fd, 주소
// 클라이언트 유저 정보 : 닉네임, 유저네임, 리얼네임, 모드, (접속한) 채널

#endif



// int Clinet::getUserStatus() {
//     return _status;
// }