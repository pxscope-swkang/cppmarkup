#pragma once
#include "typedefs.hpp"

/**
 * 고속 파싱을 위해 각 json 문자를 빠르게 순회 ...
 *
 * 프로퍼티 스택이 존재 ... Json 트리에서 현재 자신의 위치를 나타냄(문맥)
 *
 * 태그 파싱 후, 현재 프로퍼티 스택의 탑 프로퍼티(오브젝트 또는 맵)의 프로퍼티 리스트에서 대응되는 태그를 검색
 *
 * 태그 검색 시 프로퍼티 스택에 해당하는 프로퍼티 푸시 후, value 파싱 ...
 *
 * 오브젝트면 한 단계 들어감(푸시다운 오토마타), 값이면 파싱하고 빠져나옴
 *
 * 해당 태그의 모든 엘리먼트 파싱 마치면 스택에서 팝 ... 부모 문맥으로 이동
 *
 * 어트리뷰트는 "엘리먼트 이름~@@ATTR@@" 형태, 별도의 맵
 */

namespace kangsw::markup {

struct json_parse {
    u8string_view source;
};

struct json_dump {
    u8string& buff;

    // 아래로 포맷팅 옵션 ...
    int indent         = -1; // 0 이상의 값 지정 시 개행 문자 삽입됨
    int initial_indent = 0;
};

marshalerr_t parse(object& to, json_parse from);
marshalerr_t dump(json_dump to, object const& from);

} // namespace kangsw::markup
