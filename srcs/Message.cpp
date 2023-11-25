#include "../includes/Message.hpp"
#include "../includes/Buffer.hpp"
#include "../includes/utils/Error.hpp"
#include "../includes/utils/Headers.hpp"
#include "../includes/Validator.hpp"

Message::Message() {}

Message::~Message() {}

bool Message::parseMessage(std::string& originMessage) {
    _commandElements.clear(); // 명령 요소 목록 초기화

    bool first = true; // 첫 번째 요소를 추적하기 위한 플래그
    std::istringstream str; // 문자열 스트림
    std::string tmp = ""; // 임시 문자열

    // 메시지 끝의 줄바꿈 문자 처리
    if (originMessage[originMessage.size() - 1] == '\r' // 만약 마지막 문자가 '\r'이거나, '\n'이면
        || (originMessage[originMessage.size() - 2] != '\r' && originMessage[originMessage.size() - 1] == '\n'))
        str.str(originMessage.substr(0, originMessage.size() - 1)); // 마지막 문자 제거
    else                                                // 만약 /r/n이라면
        str.str(originMessage.substr(0, originMessage.size() - 2)); // 마지막 두 문자 제거

    // 스트림에서 문자열을 공백으로 구분하여 읽음 (예. QUIT :Gone to have lunch)
    while (std::getline(str, tmp, ' ')) {
        if (first) {
            // 첫 번째 요소 처리 (명령어: QUIT)
            if (tmp.empty() || Validator::checkForbiddenChar(tmp, "\r\n\0")) // 만약 빈 문자열이거나 금지된 문자가 있으면 false 반환
                return false;
            else
                _commandElements.push_back(tmp); // 그 외의 경우, 요소 추가
            first = false; // 첫 번째 요소 처리 완료
        }
        else {
            // 이후 요소 처리
            if (tmp.empty())
                return false; // 빈 문자열이면 false 반환
            else if (tmp[0] == ':') {
                // ':'로 시작하는 경우, ':'를 제외한 나머지를 추가 (예. :Gone)
                _commandElements.push_back(tmp.substr(1));
                break;
            }
            else {
                if (!Validator::checkForbiddenChar(tmp, "\r\n\0"))
                    _commandElements.push_back(tmp); // 금지된 문자가 없으면 추가
                else
                    return false; // 금지된 문자가 있으면 false 반환
            }
        }
    }

    // 남은 부분을 처리
    tmp = "";
    std::getline(str, tmp); // str에서 남은 부분을 읽어서 tmp에 저장
    if (tmp != "") // 남은 문자열이 있으면 마지막 요소에 추가
        _commandElements[_commandElements.size() - 1] += " " + tmp; // 공백은 기존 요소와 구분하기 위해 추가
    return true; // 성공적으로 파싱 완료

    /*
    QUIT :Gone to have lunch라는 입력 예시:

    입력 초기화 및 준비:
    _commandElements.clear()가 호출되어 _commandElements 리스트를 초기화합니다.
    str 스트림이 "QUIT :Gone to have lunch" 문자열로 설정됩니다.
    
    첫 번째 단어 처리 (QUIT):
    while 루프가 시작되고, std::getline(str, tmp, ' ')에 의해 첫 번째 공백 ' '까지의 문자열인 "QUIT"이 tmp에 저장됩니다.
    if (first) 조건이 참이므로, tmp가 빈 문자열이거나 금지된 문자를 포함하고 있는지 확인합니다. 여기서는 "QUIT"이므로 해당하지 않습니다.
    "QUIT"이 _commandElements에 추가되고, first는 false로 설정됩니다.
    
    두 번째 단어 처리 (:Gone):
    while 루프의 다음 반복에서, std::getline(str, tmp, ' ')는 다음 공백까지의 문자열인 ":Gone"을 tmp에 저장합니다.
    이제 if (first) 조건은 거짓이므로, else 블록이 실행됩니다.
    else if (tmp[0] == ':') 조건이 참이므로, ":Gone"에서 콜론을 제외한 "Gone"이 _commandElements에 추가됩니다.
    break; 문에 의해 while 루프를 빠져나갑니다.
    
    남은 부분 처리 (to have lunch):
    tmp = "";로 tmp를 초기화하고, std::getline(str, tmp)로 스트림의 남은 부분을 읽습니다. 이 경우, 남은 문자열은 "to have lunch"입니다.
    if (tmp != "") 조건이 참이므로, " to have lunch" (공백 포함)가 _commandElements의 마지막 요소 "Gone"에 추가됩니다.
    결과:

    return true;로 함수는 성공적으로 파싱을 완료하였음을 나타냅니다.
    최종적으로 _commandElements 리스트에는 ["QUIT", "Gone to have lunch"] 두 요소가 저장됩니다.
    */
}  

// Message 클래스의 메서드: 네트워크로부터 받은 데이터를 처리하고 메시지를 파싱합니다.
void Message::parseMessageAndExecute(int fd, std::string buffer, std::string host) {
    std::string message;
    size_t size = 0;
    int cut;

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
        if (Message::parseMessage(message))
            executeCommand(message);
    }

    // 남은 버퍼를 다시 설정합니다.
    Buffer::setReadBuffer(std::make_pair(fd, buffer));
}
