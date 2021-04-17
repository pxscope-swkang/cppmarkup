#pragma once
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json_fwd.hpp>
#include <pugixml.hpp>

/**
 * string -> raw_memory(offset, size)
 * raw_memory -> string
 *
 * 오브젝트는 내부에 포함된 인스턴스의 파싱 및 직렬화를 수행하기 위한 기저 클래스입니다.
 * 오브젝트 자체는 데이터를 담지 않으며, 파싱 및 직렬화 순서는 모두 CRTP 인스턴스의 static inline
 *vector에 저장됩니다.
 * 각각의 데이터 타입을 파싱하기 위해 오브젝트에 쓰이는 모든 데이터 형식은 bool(std::span<std::byte>,
 *std::string_view)을 제공해야 합니다. 직렬화를 위해선 void(std::string, std::span<const std::byte>)
 *함수가 제공되어야 합니다.
 *
 * 기본적으로, ezdata::marshal 네임스페이스의 parse 및 serialize 함수를 찾습니다.
 *
 * 파싱에 실패할 경우 예외를 던지거나, 단순히 실패 구조체를 반환할 수 있습니다. (optional)
 *
 * Json 및 XML 오브젝트 트리 전체를 파싱 및 직렬화하는 인터페이스를 제공합니다. 
 */
namespace ezdata
{

}