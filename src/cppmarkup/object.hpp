#pragma once
#include <functional>
#include <list>
#include <string>
#include <type_traits>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "hash.hpp"
#include <atomic>
#include <cassert>

/**
 * Json 및 XML 오브젝트 트리 전체 또는 부분을 파싱 및 직렬화하는 인터페이스를 제공합니다.
 */
namespace cppmarkup {
// clang-format off
struct tag_duplication_exception : std::exception {};
struct attribute_duplication_exception : std::exception {};
struct undefined_marshaller_exception : std::exception {};
// clang-format on
} // namespace cppmarkup

namespace pugi {
class xml_node;
} // namespace pugi

namespace cppmarkup {
/**
 *
 */
enum class node_type {
    object,
    integral_number,
    real_number,
    string,
    array,
    table,
    null,
    none,
    MAX_NODE_TYPE
};

/**
 * 빠르게 cppmarkup 오브젝트를 바이너리로 직렬화하는 인터페이스를 제공합니다.
 * 호환성은 없는 자체 포맷으로, 직렬 통신에서 빠르게 cppmarkup 오브젝트를
 *동기시킬 필요가 있을 때 사용합니다. (BSON의 커스텀 구현)
 */
enum compact_byte : uint8_t;
using compact_binary = std::vector<compact_byte>;

/**
 * 타입에 대한 파싱, 덤프 로직을 제공하는 마셜링 클래스입니다.
 * 각 타입에 대한 파싱 로직은 marshaller 내의 함수에 정의되어야 합니다.
 */
class marshaller_base
{
public:
    virtual ~marshaller_base() = default;

    virtual bool parse(void*, size_t, pugi::xml_node const&) const { throw undefined_marshaller_exception{}; }
    virtual bool parse(void*, size_t, nlohmann::json const&) const { throw undefined_marshaller_exception{}; }
    virtual bool parse(void*, size_t, compact_binary const&) const { throw undefined_marshaller_exception{}; }

    virtual bool dump(pugi::xml_node const&, void*, size_t) const { throw undefined_marshaller_exception{}; }
    virtual bool dump(nlohmann::json const&, void*, size_t) const { throw undefined_marshaller_exception{}; }
    virtual bool dump(compact_binary const&, void*, size_t) const { throw undefined_marshaller_exception{}; }
};

/**
 * 인스턴스화되는 마샬러 클래스입니다.
 * marshaller_base에 정의된 로직 중 필요한 부분을 overrride하고, 반드시 marshaller_base를 상속해야 합니다.
 */
template <typename Ty_>
class marshaller : public marshaller_base
{
};
} // namespace cppmarkup

namespace cppmarkup::impl {

template <class source_type>
using parse_fn_t = bool (*)(void*, size_t, source_type const&);
template <class source_type>
using dump_fn_t = bool (*)(source_type&, void const*, size_t);

/**
 * 오브젝트 내에 embed 된 하나의 노드에 대한 파싱/덤프 프로퍼티
 */
struct node_property {
    // 노드 해쉬. 전체 오브젝트의 일부분을 직렬화할 경우, 해쉬를 통해 일부 데이터의
    //정확한 루트 노드를 찾는 데 사용합니다. 태그로부터 고정된 알고리즘을 사용해
    //구조가 동일한 경우 반드시 같은 해쉬 반환해야 합니다.
    uint64_t hash;
    node_type type;

    std::u8string_view tag;
    std::u8string_view description; // 설명은 항상 문자열 리터럴 ...

    size_t offset;
    size_t next_offset;
    size_t value_size; // 사이즈는 padding으로 인해 부정확한 정보 .. 어서션 용도로만 사용.
    size_t total_size;

    std::vector<std::pair<std::u8string_view, std::u8string_view>> attrib_default;
    std::u8string* get_attrib(void* element_base, size_t index) const;

    marshaller_base const* pmarshal;
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

    // TODO: 여기에 DUMP 및 PARSE 함수성 추가
    // TODO: 파싱, 덤프 시 노드의 일부분만 갱신하는 기능
    // How it works ...
    // object t; t.t0.t1의 서브 오브젝트를 가질 때, t.dump(&t.t0.t1, markup_node)와 같이
    //멤버 엘리먼트에 대해 오버로드 된
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
    object_inst_init(std::atomic_size_t& new_node_idx, std::vector<node_property>& nodes, std::u8string_view tag, size_t size, const char8_t** descr, marshaller_base const* marshal, cppmarkup::node_type type)
    {
        size += Align_ - 1;
        size -= size % Align_;
        printf("aligned_size %llu\n", size);

        for (auto& pn : nodes)
        {
            if (pn.tag == tag) { throw cppmarkup::tag_duplication_exception{}; }
        }

        auto seed    = nodes.empty() ? 0 : nodes.back().hash;
        new_node_idx = nodes.size();

        auto& n = nodes.emplace_back();
        n.hash  = get_hash(tag, seed);

        n.tag        = tag;
        n.type       = type;
        n.value_size = size;
        n.total_size = size;
        n.pmarshal   = marshal;

        n.description = *descr ? std::u8string_view(*descr) : std::u8string_view{};
        if (*descr) { *descr = nullptr; }
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
using object = impl::object_base;

namespace impl {
    // https://stackoverflow.com/questions/31762958/check-if-class-is-a-template-specialization
    template <class T, template <class...> class Template>
    struct is_specialization : std::false_type {
    };

    template <template <class...> class Template, class... Args>
    struct is_specialization<Template<Args...>, Template> : std::true_type {
    };

    // TODO: Map support, map/unordered_map<std::u8string, Ty_> ... only u8string key!
    // XML  <MapTag> <KeyA>Value</KeyA><KeyB><Value><KeyB> </MapTag>
    // JSON { "MapTag" : { "KeyA": Value, "KeyB": Value } }
} // namespace impl

template <typename Ty_>
constexpr node_type get_node_type()
{
    if constexpr (std::is_base_of_v<impl::object_base, Ty_>) { return node_type::object; }
    if constexpr (std::is_integral_v<Ty_>) { return node_type::integral_number; }
    if constexpr (std::is_floating_point_v<Ty_>) { return node_type::real_number; }
    if constexpr (impl::is_specialization<Ty_, std::basic_string>::value) { return node_type::string; }
    if constexpr (impl::is_specialization<Ty_, std::vector>::value) { return node_type::array; }
    if constexpr (std::is_same_v<Ty_, nullptr_t>) { return node_type::null; }
    return node_type::none;
}
} // namespace cppmarkup

namespace cppmarkup::marshal {
// TODO: 템플릿 함수, integral은 int64_t 읽어오는 함수로, 부동 소수점은 double로, 문자열은 u8string으로,
template <typename Data_>
bool parse(Data_& d, pugi::xml_node const& s) { return false; }
bool parse(cppmarkup::impl::object_base& d, pugi::xml_node const& s);

// TODO: 완전히 동일한 CPPMarkup 구조 사이에 데이터를 주고받기 위한 고속 바이너리 인터페이스. 보통 통신 전용
inline bool parse(cppmarkup::impl::object_base& d, compact_binary const& s) { return true; }
} // namespace cppmarkup::marshal

static constexpr size_t INTERNAL_EZ_depth = 0;
