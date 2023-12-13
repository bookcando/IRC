#include "../includes/Buffer.hpp"
#include "../includes/utils/Containers.hpp"
#include "../includes/Lists.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"

BufferMap Buffer::_bufferForRead;
BufferMap Buffer::_bufferForWrite;

void Buffer::resetReadBuffer(int fd) {
    if (_bufferForRead.find(fd) != _bufferForRead.end()) {
        _bufferForRead[fd] = "";
    } else {
        _bufferForRead.insert(std::make_pair(fd, ""));
    }
}

void Buffer::resetWriteBuffer(int fd) {
    if (_bufferForWrite.find(fd) != _bufferForWrite.end()) {
        _bufferForWrite[fd] = "";
    } else {
        _bufferForWrite.insert(std::make_pair(fd, ""));
    }
}

void Buffer::eraseReadBuffer(int fd) {
    if (_bufferForRead.find(fd) != _bufferForRead.end()) {
        _bufferForRead.erase(fd);
    }
}

void Buffer::eraseWriteBuffer(int fd) {
    if (_bufferForWrite.find(fd) != _bufferForWrite.end()) {
        _bufferForWrite.erase(fd);
    }
}

// Buffer 클래스의 메서드: 지정된 파일 기술자(fd)에 대한 읽기 버퍼를 반환합니다.
std::string Buffer::getReadBuffer(int fd) {
    // _bufferForRead 맵에서 해당 fd를 찾습니다.
    if (_bufferForRead.find(fd) != _bufferForRead.end()) {
        // fd가 맵에 있으면, 해당 fd의 버퍼 내용을 반환합니다.
        return _bufferForRead[fd];
    } else {
        // fd가 맵에 없으면, 빈 문자열을 반환합니다.
        return "";
    }
}

std::string Buffer::getWriteBuffer(int fd) {
    if (_bufferForWrite.find(fd) != _bufferForWrite.end()) {
        return _bufferForWrite[fd];
    } else {
        return "";
    }
}

void Buffer::setReadBuffer(std::pair<int, std::string> val) {
    if (_bufferForRead.find(val.first) != _bufferForRead.end()) {
        _bufferForRead[val.first] += val.second;
    } else {
        _bufferForRead.insert(val);
    }
}

void Buffer::setWriteBuffer(std::pair<int, std::string> val) {
    if (_bufferForWrite.find(val.first) != _bufferForWrite.end()) {
        _bufferForWrite[val.first] += val.second;
    } else {
        _bufferForWrite.insert(val);
    }
}

// Buffer 클래스의 메서드: 소켓에서 메시지 읽기
int Buffer::readMessage(int fd, intptr_t data) {
    char buffer[data + 1]; // 버퍼를 동적으로 할당
    //minsulee : 이거 Variable Length Array인데 C++98에서 괜찮은지 모르겠어요.
    int byte;

    memset(buffer, 0, sizeof(buffer)); // 버퍼를 0으로 초기화
    byte = recv(fd, buffer, data, 0); // 소켓에서 데이터를 읽어옴

    if (byte <= 0) {
        return byte; // 읽기 실패 또는 연결 종료의 경우, 읽은 바이트 수(또는 오류 코드) 반환
    }
    _bufferForRead[fd] += buffer; // 읽은 데이터를 관련 fd의 읽기 버퍼에 추가
    return byte; // 읽은 바이트 수 반환
}

//Server::run의 WRITE_EVENT에서만 사용하는 sendMessage
int Buffer::sendMessage(int fd) {
    int byte = 0;

    std::string message = _bufferForWrite[fd];
    byte = send(fd, message.c_str(), message.length(), 0);
    if (byte == -1) {
        // Server::pushEvents(nullptr, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
        // Lists::findClient(fd).getServerPtr()->pushevents(0, fd, EVFILT_WRITE, EV_ADD | EV d_ENABLE | EV_ONESHOT, 0, 0, NULL);
        Lists::findClient(fd).getServerPtr()->pushEvents(fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
        // getServerPtr()->pushEvents(nullptr, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
        //EVENT SETTER
        return -1;
    }
    // ServerRun에서는 한 번 보내본다
    // 1. 성공하면 그대로 그 부분 
    // 보내보고 안보내지면 다시
    // 이벤트 생성하고(WRITE_EVENT를 잡기 위함)
    // 이벤트 등록되면 이벤트 핸들러에서 보내기

    if (static_cast<size_t>(byte) < message.length()) {
        _bufferForWrite[fd] = _bufferForWrite[fd].substr(byte);
        //EVENT SETTER
        Lists::findClient(fd).getServerPtr()->pushEvents(fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
    }
    else
        _bufferForWrite[fd] = "";

    #ifdef DEBUG
    std::cout << "send: " << fd << " " << message;
    #endif
    return byte;
}

//Command들에서만 사용하는 sendMessage -> 여기서 send 안하고 그냥 _bufferForWrite에 넣어두고 이벤트만 잡으려 하자고
int Buffer::sendMessage(int fd, std::string message) {
    // int byte = 0;
    //EVENT SETTER
    Lists::findClient(fd).getServerPtr()->pushEvents(fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
    _bufferForWrite[fd] += message;
    // #ifdef DEBUG
    // std::cout << "send: " << fd << " " << message;
    // #endif
    return 0;
    // return byte;
}