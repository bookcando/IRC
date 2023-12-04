#include "../includes/Lists.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Message.hpp"
#include "../includes/Command.hpp"
#include <iostream>

ChannelMap Lists::channelList; // 채널 정보를 저장하는 정적 맵을 선언합니다.
ClientMap Lists::clientList; // 클라이언트 정보를 저장하는 정적 맵을 선언합니다.

// -> 클라이언트를 저장하는 클라이언트리스트 라는 GLOBAL 변수가 이미 존재함 -> 서버에 만들어서는 안됨.

// Lists::Lists() {} // List 클래스의 생성자입니다. 특별한 초기화 작업이 없습니다.
// Lists::~Lists() {} // List 클래스의 소멸자입니다. 특별한 정리 작업이 없습니다.

bool Lists::addClientList(int fd, in_addr info) {
    // 클라이언트 목록에 주어진 파일 기술자(fd)가 없으면 새 클라이언트를 추가합니다.
    if (clientList.find(fd) == clientList.end()) {
        clientList.insert(std::make_pair(fd, new Client(fd, info))); // 새 클라이언트 객체를 생성하고 목록에 추가합니다.
        return true; // 성공적으로 추가된 경우 true를 반환합니다.
    }
    return false; // 이미 존재하는 경우 false를 반환합니다.
}

bool Lists::deleteClientList(int fd) {
    // 클라이언트 목록에 주어진 파일 기술자(fd)가 있으면 해당 클라이언트를 삭제합니다.
    if (clientList.find(fd) != clientList.end()) {
        delete clientList[fd]; // 클라이언트 객체를 메모리에서 해제합니다.
        clientList[fd] = nullptr;
        clientList.erase(fd); // 맵에서 해당 엔트리를 제거합니다 (댕글링 포인터 방지)
        return true; // 성공적으로 삭제된 경우 true를 반환합니다.
    }
    return false; // 존재하지 않는 경우 false를 반환합니다.
}

void Lists::clearClientList() {
    // 클라이언트 목록의 모든 엔트리를 순회하며 삭제합니다.
    ClientMap::iterator iter = clientList.begin(); // 클라이언트 목록을 순회하기 위한 반복자를 초기화합니다.
    for (iter = clientList.begin(); iter != clientList.end(); ++iter) {
        delete iter->second; // 각 클라이언트 객체를 메모리에서 해제합니다.
        iter->second = nullptr;
    }
    clientList.clear(); // 맵을 비웁니다.
}

Client& Lists::findClient(int fd) {
    // 주어진 파일 기술자(fd)에 해당하는 클라이언트를 찾아 반환합니다.
    if (clientList.find(fd) == clientList.end()) // 만약 주어진 파일 기술자에 해당하는 클라이언트가 없으면
        throw std::out_of_range("Client not found"); // 예외를 발생시킵니다. (이는 서버의 오류입니다.
    return *clientList.find(fd)->second;
}

bool Lists::hasClient(int fd) {
    // 클라이언트 목록에 주어진 파일 기술자(fd)가 있는지 확인합니다.
    return clientList.find(fd) != clientList.end();
}

ClientMap& Lists::getClientList() {
    // 클라이언트 목록 전체를 반환합니다.
    return Lists::clientList;
}

bool Lists::addChannelList(std::string name, Client* creator) {
    // 채널 목록에 주어진 이름(name)이 없으면 새 채널을 추가합니다.
    if (channelList.find(name) == channelList.end()) {
        channelList.insert(std::make_pair(name, new Channel(name, creator))); // 새 채널 객체를 생성하고 목록에 추가합니다.
        return true; // 성공적으로 추가된 경우 true를 반환합니다.
    }
    return false; // 이미 존재하는 경우 false를 반환합니다.
}

bool Lists::deleteChannelList(std::string name) {
    // 채널 목록에 주어진 이름(name)이 있으면 해당 채널을 삭제합니다.
    if (channelList.find(name) != channelList.end()) {
        delete channelList[name]; // 채널 객체를 메모리에서 해제합니다.
        channelList[name] = nullptr;
        channelList.erase(name); // 맵에서 해당 엔트리를 제거합니다.
        return true; // 성공적으로 삭제된 경우 true를 반환합니다.
    }
    return false; // 존재하지 않는 경우 false를 반환합니다.
}

void Lists::clearChannelList() {
    // 채널 목록의 모든 엔트리를 순회하며 삭제합니다.
    ChannelMap::iterator iter = channelList.begin(); // 채널 목록을 순회하기 위한 반복자를 초기화합니다.
    for (iter = channelList.begin(); iter != channelList.end(); ++iter) {
        delete iter->second; // 각 채널 객체를 메모리에서 해제합니다.
        iter->second = nullptr;
    }
    channelList.clear(); // 맵을 비웁니다.
}

Channel& Lists::findChannel(std::string name) {
    // 주어진 이름(name)에 해당하는 채널을 찾아 반환합니다.
    return *channelList.find(name)->second;
}

bool Lists::hasChannel(std::string name) {
    // 채널 목록에 주어진 이름(name)이 있는지 확인합니다.
    return channelList.find(name) != channelList.end();
}

ChannelMap& Lists::getChannelList() {
    // 채널 목록 전체를 반환합니다.
    return Lists::channelList;
}
