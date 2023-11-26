#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "./utils/Headers.hpp"
#include "./utils/Containers.hpp"
#include "Client.hpp"


class Channel {
private:
	std::string _channelName;	// 채널 이름
	Client* _channelOperator;	// 채널 운영자
	ClientMap _userList;	// 채널 가입자 명단
	int _userLimit;	// 가입자수 상한
	int _mode;	// 채널 모드(비트 마스킹)
	std::string _topic;	// 채널 주제
	std::string _password;	// 채널 비밀번호
	time_t _creationTime;	// 채널 생성 시간
	std::string _key;	// 채널 암호
	ClientMap _inviteList;	// 초대자 명단
public:
	Channel(std::string channelName, Client* client);
	~Channel();

	// getter
	Client const* getChannelOperator() const;
	ClientMap const& getUserList() const;
	int getUserLimit() const;
	std::string getChannelName() const;
	std::string getTopic() const;
	int getMode() const;
	time_t getTime() const;
	std::string getKey() const;
	std::string getStrUserList() const;

	// setter
	void setChannelName(std::string& name);
	void setChannelOperator(Client* client);
	void setUserLimit(int userLimit);
	void setTopic(std::string topic);
	void setPassword(std::string password);
	void setMode(int mode, bool flag);
	void setKey(std::string key);

	// add
	void addInviteList(Client* client);
	void addClientList(Client* client);

	// delete
	void deleteInviteList(Client* client);
	void deleteClientList(Client* client);

	// checker
	bool isClientInvite(Client* client);
};

#endif 