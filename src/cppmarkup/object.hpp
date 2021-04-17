#pragma once
#include <functional>
#include <list>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include <atomic>
#include <cassert>

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
 * 기본적으로, cppmarkup::marshal 네임스페이스의 parse 및 serialize 함수를 찾습니다.
 *
 * 파싱에 실패할 경우 예외를 던지거나, 단순히 실패 구조체를 반환할 수 있습니다. (optional)
 *
 * Json 및 XML 오브젝트 트리 전체를 파싱 및 직렬화하는 인터페이스를 제공합니다. 
 */
namespace cppmarkup {
// clang-format off
struct tag_duplication_exception : std::exception {};
struct attribute_duplication_exception : std::exception {};
// clang-format on
} // namespace cppmarkup

namespace pugi {
class xml_node;
} // namespace pugi

namespace cppmarkup::impl {

template <class source_type>
using parse_fn_t = std::function<bool(void*, size_t, source_type const&)>;
template <class source_type>
using dump_fn_t = std::function<bool(source_type&, void const*, size_t)>;

/**
 * 오브젝트 내에 embed 된 하나의 노드에 대한 파싱/덤프 프로퍼티
 */
struct node_property {
    std::u8string_view tag;
    std::u8string_view description; // 설명은 항상 문자열 리터럴 ...

    size_t offset;
    size_t next_offset;
    size_t value_size; // 사이즈는 padding으로 인해 부정확한 정보 .. 어서션 용도로만 사용.
    size_t total_size;

    std::vector<std::pair<std::u8string_view, std::u8string_view>> attrib_default;

    struct {
        parse_fn_t<pugi::xml_node> parser;
        dump_fn_t<pugi::xml_node> dumper;
    } xml;

    std::u8string* get_attrib(void* element_base, size_t index) const;
};

/**
 * 오브젝트 템플릿 정의 시 새로 정의하는 모든 클래스의 베이스 타입.
 * 
 */
class object_base
{
public:
    virtual ~object_base() = default;

    virtual std::vector<node_property> const& properties() const = 0;
    virtual size_t depth() const                                 = 0;

    // TODO: 여기에 DUMP 및 PARSE 함수성 추가.
};

/**
 * 오브젝트 템플릿이 실제로 상속하는 클래스입니다.
 * node_list를 검색 가능하게 함
 * 각각의 object_instance 객체가 실질적인 node_property static inline member를 가지므로,
 *포인터 형태로 해당 static inline 멤버를 가리킵니다.
 */
template <typename Tmplt_>
struct traits : object_base {
    static inline std::vector<node_property> INTERNAL_EZ_node_list = {};
    static inline const char8_t* INTERNAL_EZ_description_str       = {};

    struct INTERNAL_EZ_description_replacer {
        INTERNAL_EZ_description_replacer(const char8_t* str) { INTERNAL_EZ_description_str = str; }
    };

    std::vector<node_property> const& properties() const override { return INTERNAL_EZ_node_list; }
};

/**
 * CPPMARKUP_ADD로 생성되는 오브젝트 인스턴스가 상속합니다.
 */
template <class Crtp_, typename Val_, size_t Align_>
struct object_inst_init {
    object_inst_init(std::atomic_size_t& pnewnode, std::vector<node_property>& nodes, std::u8string_view tag, size_t size, const char8_t** descr, parse_fn_t<pugi::xml_node>&& f)
    {
        size += Align_ - 1;
        size -= size % Align_;
        printf("aligned_size %llu\n", size);

        for (auto& pn : nodes)
        {
            if (pn.tag == tag) { throw cppmarkup::tag_duplication_exception{}; }
        }

        pnewnode     = nodes.size();
        auto& n      = nodes.emplace_back();
        n.tag        = tag;
        n.value_size = size;
        n.total_size = size;

        n.description = *descr ? std::u8string_view(*descr) : std::u8string_view{};
        if (*descr) { *descr = nullptr; }

        n.xml.parser = std::move(f);
    }
};

template <class UniqueClass_, size_t Align_>
struct object_inst_attr_init {
    object_inst_attr_init(std::vector<node_property>& nodes, std::u8string_view attr_name, std::u8string_view default_value)
    {
        auto& n = nodes.back();

        for (auto& attr : n.attrib_default)
        {
            if (attr.first == attr_name) { throw attribute_duplication_exception{}; }
        }

        auto size = sizeof(std::u8string);
        size += Align_ - 1;
        size -= size % Align_;

        n.attrib_default.emplace_back(attr_name, default_value);
        n.total_size += size;
    }
};

} // namespace cppmarkup::impl

namespace cppmarkup {
enum compact_byte : uint8_t;
using compact_binary = std::vector<compact_byte>;
}

namespace cppmarkup::marshal {
// TODO: 템플릿 함수, integral은 int64_t 읽어오는 함수로, 부동 소수점은 double로, 문자열은 u8string으로,
template<typename Data_>
bool parse(Data_& d, pugi::xml_node const& s) { return false; }
bool parse(cppmarkup::impl::object_base& d, pugi::xml_node const& s);

// TODO: 완전히 동일한 CPPMarkup 구조 사이에 데이터를 주고받기 위한 고속 바이너리 인터페이스. 보통 통신 전용
inline bool parse(cppmarkup::impl::object_base& d, compact_binary const& s) { return true; }
} // namespace cppmarkup::marshal

static constexpr size_t INTERNAL_EZ_depth = 0;
