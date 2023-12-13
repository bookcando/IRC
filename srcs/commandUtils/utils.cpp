#include "../../includes/utils/utils.hpp"

time_t getCurTime() {
	return time(NULL); // 현재 시간을 반환합니다.
}

std::string getStringTime(time_t const& time) {
	char buf[50]; // 시간 문자열을 저장할 버퍼
	struct tm* tm; // 시간 구조체

	memset(buf, 0, sizeof(buf)); // 버퍼를 0으로 초기화합니다.
	tm = localtime(&time); // time_t 구조체를 tm 구조체로 변환합니다.
	strftime(buf, 50, "%c", tm); // 시간을 문자열로 변환합니다.
	return buf; // 변환된 문자열을 반환합니다.
}

bool chkForbiddenChar(std::string const& str, std::string const& forbidden_set) {
	for (size_t i = 0; i < str.size(); i++) { // 문자열을 순회하며 금지 문자가 있는지 확인합니다.
		for (size_t j = 0; j < forbidden_set.size(); j++) // 금지 문자 집합을 순회하며 금지 문자가 있는지 확인합니다.
			if (str[i] == forbidden_set[j]) // 금지 문자가 있다면
				return true;           // true를 반환합니다.
	}
	return false; // 금지 문자가 없다면 false를 반환합니다.
}
