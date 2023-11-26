#include "../includes/Validator.hpp"
#include "../includes/Buffer.hpp"

long long Validator::validatePort(const std::string& port) {
    long long portNum = std::strtoll(port.c_str(), NULL, 10);
    if (portNum < 1024 || portNum > 65535) {
        throw std::runtime_error("ERROR: Invalid port number");
    }
    return portNum;
}

void Validator::validatePassword(const std::string& _password) {
    if (_password.length() < 8) {
        throw std::runtime_error("ERROR: Password must be at least 8 characters long");
    }
    if (_password.find(" ") != std::string::npos) {
        throw std::runtime_error("ERROR: Password cannot contain spaces");
    }
    if (_password.find("\r") != std::string::npos || _password.find("\n") != std::string::npos) {
        throw std::runtime_error("ERROR: Password cannot contain newlines");
    }
}

// Validator 클래스의 메서드: 메시지 유효성 검사
bool Validator::validateMessage(const int fd, const intptr_t data) {
    int byte = 0;

    byte = Buffer::readMessage(fd, data); // 소켓으로부터 메시지를 읽습니다.

    if (byte <= 0) {
        return false; // 읽은 데이터가 없거나 오류가 발생한 경우, 유효하지 않음
    }
    return true; // 그 외의 경우, 유효함
}

bool Validator::checkForbiddenChar(std::string const& str, std::string const& forbidden_set) {
    for (size_t i = 0; i < str.size(); i++) { // 문자열의 각 문자를 순회
        for (size_t j = 0; j < forbidden_set.size(); j++) { // 금지된 문자 세트의 각 문자를 순회
            if (str[i] == forbidden_set[j]) // 현재 문자열의 문자가 금지된 문자 중 하나와 일치하는지 확인
                return true; // 일치하는 경우 true 반환
        }
    }
    return false; // 금지된 문자가 문자열에 없는 경우 false 반환
}
