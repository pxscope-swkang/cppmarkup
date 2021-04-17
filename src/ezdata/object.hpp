#pragma once
#include <functional>
#include <list>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>
#include <pugixml.hpp>

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
 * 기본적으로, ezdata::marshal 네임스페이스의 parse 및 serialize 함수를 찾습니다.
 *
 * 파싱에 실패할 경우 예외를 던지거나, 단순히 실패 구조체를 반환할 수 있습니다. (optional)
 *
 * Json 및 XML 오브젝트 트리 전체를 파싱 및 직렬화하는 인터페이스를 제공합니다. 
 */
namespace ezdata {
// clang-format off
struct tag_duplication_exception : std::exception {};
struct attribute_duplication_exception : std::exception {};
// clang-format on
} // namespace ezdata

namespace ezdata::impl {

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
    virtual std::list<node_property> const& properties() const = 0;
    virtual ~object_base()                                     = default;
};

/**
 * 오브젝트 템플릿이 실제로 상속하는 클래스입니다.
 * node_list를 검색 가능하게 함
 * 각각의 object_instance 객체가 실질적인 node_property static inline member를 가지므로,
 *포인터 형태로 해당 static inline 멤버를 가리킵니다.
 */
template <typename Tmplt_>
struct traits : object_base {
    static inline std::list<node_property> INTERNAL_EZ_node_list = {};
    static inline const char8_t* INTERNAL_EZ_description_str     = {};

    struct INTERNAL_EZ_description_replacer {
        INTERNAL_EZ_description_replacer(const char8_t* str) { INTERNAL_EZ_description_str = str; }
    };

    std::list<node_property> const& properties() const override
    {
        return INTERNAL_EZ_node_list;
    }
};

/**
 * EZDATA_ADD로 생성되는 오브젝트 인스턴스가 상속합니다.
 */
template <class Crtp_, typename Val_, size_t Align_>
struct object_inst_init {
    object_inst_init(std::atomic<node_property*>& pnewnode, std::list<node_property>& nodes, std::u8string_view tag, size_t size, const char8_t** descr, parse_fn_t<pugi::xml_node>&& f)
    {
        size += Align_ - 1;
        size -= size % Align_;
        printf("aligned_size %llu\n", size);

        for (auto& pn : nodes)
        {
            if (pn.tag == tag) { throw ezdata::tag_duplication_exception{}; }
        }

        auto& n      = nodes.emplace_back();
        pnewnode     = &n;
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
    object_inst_attr_init(std::list<node_property>& nodes, std::u8string_view attr_name, std::u8string_view default_value)
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

} // namespace ezdata::impl

namespace ezdata::marshal {

bool parse(int32_t& d, pugi::xml_node const& s) { return true; }
bool parse(int64_t& d, pugi::xml_node const& s) { return true; }
bool parse(int16_t& d, pugi::xml_node const& s) { return true; }
bool parse(int8_t& d, pugi::xml_node const& s) { return true; }
bool parse(float& d, pugi::xml_node const& s) { return true; }
bool parse(double& d, pugi::xml_node const& s) { return true; }
bool parse(std::u8string& d, pugi::xml_node const& s) { return true; }
bool parse(ezdata::impl::object_base& d, pugi::xml_node const& s) { return true; }

} // namespace ezdata::marshal

#define EZDATA_OBJECT_TEMPLATE(template_type_name)                                \
    struct INTERNAL_EZ_SUPER_##template_type_name                                 \
        : public ::ezdata::impl::traits<INTERNAL_EZ_SUPER_##template_type_name> { \
        using INTERNAL_EZ_super = INTERNAL_EZ_SUPER_##template_type_name;         \
    };                                                                            \
    struct template_type_name : public INTERNAL_EZ_SUPER_##template_type_name

#ifndef EZDATA_ALIGN
#define EZDATA_ALIGN 8
#endif

#define EZDATA_ADD(varname, tag, default_value, ...)                                                       \
    struct INTERNAL_EZ_INSTANCE_##varname {                                                                \
        using value_type = decltype(default_value);                                                        \
                                                                                                           \
    private:                                                                                               \
        alignas(EZDATA_ALIGN) value_type _value                             = default_value;               \
        static inline std::atomic<::ezdata::impl::node_property*> _node_ref = nullptr;                     \
                                                                                                           \
    public:                                                                                                \
        static bool parse(void* r, size_t size, pugi::xml_node const& s)                                   \
        {                                                                                                  \
            assert(sizeof(value_type) == size);                                                            \
            return ::ezdata::marshal::parse(*(value_type*)r, s);                                           \
        }                                                                                                  \
                                                                                                           \
        static inline ::ezdata::impl::object_inst_init<                                                    \
            INTERNAL_EZ_INSTANCE_##varname, value_type, EZDATA_ALIGN>                                      \
            _init{                                                                                         \
                _node_ref,                                                                                 \
                INTERNAL_EZ_node_list,                                                                     \
                tag,                                                                                       \
                sizeof _value,                                                                             \
                &INTERNAL_EZ_description_str,                                                              \
                &parse};                                                                                   \
                                                                                                           \
        operator value_type() const                                                                        \
        {                                                                                                  \
            return _value;                                                                                 \
        }                                                                                                  \
                                                                                                           \
        INTERNAL_EZ_INSTANCE_##varname(void* owner_base)                                                   \
        { /* Accurate offset is recalculated here. */                                                      \
            if (auto ptr = _node_ref.exchange(nullptr))                                                    \
            {                                                                                              \
                printf("ptr: %p ", ptr);                                                                   \
                ptr->offset      = (intptr_t)this - (intptr_t)owner_base;                                  \
                ptr->next_offset = ptr->offset + sizeof *this;                                             \
                printf("offset: %llu ~ size: %llu\n", ptr->offset, ptr->total_size);                       \
            }                                                                                              \
        }                                                                                                  \
                                                                                                           \
        INTERNAL_EZ_INSTANCE_##varname(const INTERNAL_EZ_INSTANCE_##varname& r) = default;                 \
        INTERNAL_EZ_INSTANCE_##varname(INTERNAL_EZ_INSTANCE_##varname&& r)      = default;                 \
        INTERNAL_EZ_INSTANCE_##varname(value_type const& r = value_type{}) : _value(r) {}                  \
        INTERNAL_EZ_INSTANCE_##varname(value_type&& r) : _value(std::move(r)) {}                           \
        INTERNAL_EZ_INSTANCE_##varname& operator=(const INTERNAL_EZ_INSTANCE_##varname& r) = default;      \
        INTERNAL_EZ_INSTANCE_##varname& operator=(INTERNAL_EZ_INSTANCE_##varname&& r) = default;           \
                                                                                                           \
        INTERNAL_EZ_INSTANCE_##varname& operator=(value_type const& r) { return _value = r, *this; }       \
        INTERNAL_EZ_INSTANCE_##varname& operator=(value_type&& r) { return _value = std::move(r), *this; } \
        auto& operator()() { return _value; }                                                              \
        auto& operator()() const { return _value; }                                                        \
                                                                                                           \
        __VA_ARGS__                                                                                        \
    } varname{this};

#define EZDATA_ATTR(attr_name, default_value)                             \
    alignas(EZDATA_ALIGN) std::u8string attr_name;                        \
    struct INTERNAL_EZ_ATTR_##attr_name {                                 \
        static inline ::ezdata::impl::object_inst_attr_init<              \
            INTERNAL_EZ_ATTR_##attr_name, EZDATA_ALIGN>                   \
            _init{INTERNAL_EZ_node_list, u8## #attr_name, default_value}; \
    };

#define EZDATA_ADD_ARRAY(template_type_name)
#define EZDATA_DESCRIPTION_BELOW(description) \
    static inline INTERNAL_EZ_description_replacer DESCRIPTION_##__LINE__{description};

#define EZDATA_NESTED_OBJECT(varname, tag, ...)               \
    EZDATA_OBJECT_TEMPLATE(INTERNAL_EZ_##varname##_TEMPLATE){ \
        __VA_ARGS__};                                         \
    EZDATA_ADD(varname, tag, INTERNAL_EZ_##varname##_TEMPLATE{})
