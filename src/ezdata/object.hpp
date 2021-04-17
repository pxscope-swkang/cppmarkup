#pragma once
#include <functional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>
#include <pugixml.hpp>

#include <cassert>

/**
 * string -> raw_memory(offset, size)
 * raw_memory -> string
 *
 * ������Ʈ�� ���ο� ���Ե� �ν��Ͻ��� �Ľ� �� ����ȭ�� �����ϱ� ���� ���� Ŭ�����Դϴ�.
 * ������Ʈ ��ü�� �����͸� ���� ������, �Ľ� �� ����ȭ ������ ��� CRTP �ν��Ͻ��� static inline
 *vector�� ����˴ϴ�.
 * ������ ������ Ÿ���� �Ľ��ϱ� ���� ������Ʈ�� ���̴� ��� ������ ������ bool(std::span<std::byte>,
 *std::string_view)�� �����ؾ� �մϴ�. ����ȭ�� ���ؼ� void(std::string, std::span<const std::byte>)
 *�Լ��� �����Ǿ�� �մϴ�.
 *
 * �⺻������, ezdata::marshal ���ӽ����̽��� parse �� serialize �Լ��� ã���ϴ�.
 *
 * �Ľ̿� ������ ��� ���ܸ� �����ų�, �ܼ��� ���� ����ü�� ��ȯ�� �� �ֽ��ϴ�. (optional)
 *
 * Json �� XML ������Ʈ Ʈ�� ��ü�� �Ľ� �� ����ȭ�ϴ� �������̽��� �����մϴ�. 
 */
namespace ezdata::impl {

template <class source_type>
using parse_fn_t = std::function<bool(void *, size_t, source_type const &)>;
template <class source_type>
using dump_fn_t = std::function<bool(source_type &, void const *, size_t)>;

/**
 * ������Ʈ ���� embed �� �ϳ��� ��忡 ���� �Ľ�/���� ������Ƽ
 */
struct node_property {
    std::u8string_view tag;
    size_t offset;
    size_t next_offset;
    size_t size;                    // ������� padding���� ���� ����Ȯ�� ���� .. ��� �뵵�θ� ���.
    std::u8string_view description; // ������ �׻� ���ڿ� ���ͷ� ...
    struct {
        parse_fn_t<pugi::xml_node> parser;
        dump_fn_t<pugi::xml_node> dumper;
    } xml;
    std::vector<std::pair<std::u8string_view, std::u8string_view>> attrib_default;

    std::u8string *get_attrib(void *element_base, size_t index) const
    {
        auto str_base = reinterpret_cast<std::u8string *>(size + (char *)element_base);
        assert(index <= (next_offset - offset) / sizeof(std::u8string));
        assert((next_offset - offset) % sizeof(std::u8string) == 0);
        return str_base + index;
    }
};

/**
 * ������Ʈ ���ø� ���� �� ���� �����ϴ� ��� Ŭ������ ���̽� Ÿ��.
 * 
 */
class object_base
{
public:
};

/**
 * ������Ʈ ���ø��� ������ ����ϴ� Ŭ�����Դϴ�.
 * node_list�� �˻� �����ϰ� ��
 * ������ object_instance ��ü�� �������� node_property static inline member�� �����Ƿ�,
 *������ ���·� �ش� static inline ����� ����ŵ�ϴ�.
 */
template <typename Tmplt_>
struct traits : object_base {
    static inline std::vector<node_property> INTERNAL_EZ_node_list = {};
    static inline const char8_t *INTERNAL_EZ_description_str       = {};
};

/**
 * EZDATA_ADD�� �����Ǵ� ������Ʈ �ν��Ͻ��� ����մϴ�.
 */
template <class Crtp_, typename Val_, size_t Align_>
struct object_inst_init {
    object_inst_init(std::vector<node_property> &nodes, std::u8string_view tag, size_t size, const char8_t **descr, parse_fn_t<pugi::xml_node> &&f)
    {
        size_t offset = nodes.empty() ? 0 : nodes.back().next_offset;
        size += Align_ - 1;
        size -= size % Align_;

        auto &n = nodes.emplace_back();
        n.tag   = tag;
        n.size  = size;

        n.offset      = offset; // size�� 8����Ʈ�� align���Ѽ� ������ ����.
        n.next_offset = offset + size;

        n.description = *descr ? std::u8string_view(*descr) : std::u8string_view{};
        if (*descr) { *descr = nullptr; }

        n.xml.parser = std::move(f);
    }
};

template <class UniqueClass_, size_t Align_>
struct object_inst_attr_init {
    object_inst_attr_init(std::vector<node_property> &nodes, std::u8string_view attr_name, std::u8string_view default_value)
    {
        auto &n = nodes.back();
        n.attrib_default.emplace_back(attr_name, default_value);

        // u8string ũ�⸸ŭ �������� �ڷ� �� ... ���� ������Ʈ�� ���������� ��ġ�ϰ�
        auto size = sizeof(std::u8string);
        size += Align_ - 1;
        size -= size % Align_;

        n.next_offset += size;
        assert(n.next_offset - n.offset / sizeof(std::u8string) == n.attrib_default.size());
    }
};

} // namespace ezdata::impl

namespace ezdata::marshal {
bool parse(int32_t &d, pugi::xml_node const &s);
bool parse(int64_t &d, pugi::xml_node const &s);
bool parse(int16_t &d, pugi::xml_node const &s);
bool parse(int8_t &d, pugi::xml_node const &s);
bool parse(float &d, pugi::xml_node const &s);
bool parse(double &d, pugi::xml_node const &s);
bool parse(std::u8string &d, pugi::xml_node const &s);
bool parse(ezdata::impl::object_base &d, pugi::xml_node const &s);

} // namespace ezdata::marshal

#define INTERNAL_EZDATA_OBJECT_TEMPLATE(template_type_name)                     \
    struct INTERNAL_EZ_SUPER_##template_type_name                               \
        : public ezdata::impl::traits<INTERNAL_EZ_SUPER_##template_type_name> { \
        using INTERNAL_EZ_super = INTERNAL_EZ_SUPER_##template_type_name;       \
    };                                                                          \
    struct template_type_name : public INTERNAL_EZ_SUPER_##template_type_name

#ifndef EZDATA_ALIGN
#define EZDATA_ALIGN 8
#endif

#define EZDATA_ADD(varname, tag, default_value, ...)                                                       \
    struct INTERNAL_EZ_INSTANCE_##varname {                                                                \
        using value_type                        = decltype(default_value);                                 \
        alignas(EZDATA_ALIGN) value_type _value = default_value;                                           \
                                                                                                           \
        static inline ezdata::impl::object_inst_init<                                                      \
            INTERNAL_EZ_INSTANCE_##varname, value_type, EZDATA_ALIGN>                                      \
            _init{                                                                                         \
                INTERNAL_EZ_node_list, tag, sizeof _value, &INTERNAL_EZ_description_str,                   \
                [](void *r, size_t size, pugi::xml_node const &s) -> bool {                                \
                    assert(sizeof(value_type) == size);                                                    \
                    return ezdata::marshal::parse(*(value_type *)r, s);                                    \
                }};                                                                                        \
                                                                                                           \
        operator value_type() const                                                                        \
        {                                                                                                  \
            return _value;                                                                                 \
        }                                                                                                  \
                                                                                                           \
        INTERNAL_EZ_INSTANCE_##varname(const INTERNAL_EZ_INSTANCE_##varname &r) = default;                 \
        INTERNAL_EZ_INSTANCE_##varname(INTERNAL_EZ_INSTANCE_##varname &&r)      = default;                 \
        INTERNAL_EZ_INSTANCE_##varname(value_type const &r = value_type{}) : _value(r) {}                  \
        INTERNAL_EZ_INSTANCE_##varname(value_type &&r) : _value(std::move(r)) {}                           \
        INTERNAL_EZ_INSTANCE_##varname &operator=(const INTERNAL_EZ_INSTANCE_##varname &r) = default;      \
        INTERNAL_EZ_INSTANCE_##varname &operator=(INTERNAL_EZ_INSTANCE_##varname &&r) = default;           \
                                                                                                           \
        INTERNAL_EZ_INSTANCE_##varname &operator=(value_type const &r) { return _value = r, *this; }       \
        INTERNAL_EZ_INSTANCE_##varname &operator=(value_type &&r) { return _value = std::move(r), *this; } \
        auto &operator()() { return _value; }                                                              \
        auto &operator()() const { return _value; }                                                        \
                                                                                                           \
        __VA_ARGS__                                                                                        \
    } varname;

#define EZDATA_ATTR(attr_name, default_value)                             \
    std::u8string attr_name;                                              \
    struct INTERNAL_EZ_ATTR_##attr_name {                                 \
        static inline ezdata::impl::object_inst_attr_init<                \
            INTERNAL_EZ_ATTR_##attr_name, EZDATA_ALIGN>                   \
            _init{INTERNAL_EZ_node_list, u8## #attr_name, default_value}; \
    };

#define INTERNAL_EZDATA_ADD_ARRAY(template_type_name)
#define INTERNAL_EZDATA_DESCRIPTION_BELOW(template_type_name)

INTERNAL_EZDATA_OBJECT_TEMPLATE(some_type)
{
    /*
    struct INTERNAL_EZ_INSTANCE_varname {
        using value_type                                 = decltype(TEMP_DEFAULTVAL);
        alignas(EZDATA_ALIGN) value_type _value = TEMP_DEFAULTVAL;

        static inline ezdata::impl::object_inst_init<INTERNAL_EZ_INSTANCE_varname, value_type, EZDATA_ALIGN> _init{
            INTERNAL_EZ_node_list, u8"TagName", sizeof _value, &INTERNAL_EZ_description_str,
            [](void *r, size_t size, pugi::xml_node const &s) -> bool {
                assert(sizeof(value_type) == size);
                return ezdata::marshal::parse(*(value_type *)r, s);
            }};

        operator value_type() const
        {
            return _value;
        }

        INTERNAL_EZ_INSTANCE_varname(const INTERNAL_EZ_INSTANCE_varname &r) = default;
        INTERNAL_EZ_INSTANCE_varname(INTERNAL_EZ_INSTANCE_varname &&r)      = default;
        INTERNAL_EZ_INSTANCE_varname(value_type const &r = value_type{}) : _value(r) {}
        INTERNAL_EZ_INSTANCE_varname(value_type &&r) : _value(std::move(r)) {}
        INTERNAL_EZ_INSTANCE_varname &operator=(const INTERNAL_EZ_INSTANCE_varname &r) = default;
        INTERNAL_EZ_INSTANCE_varname &operator=(INTERNAL_EZ_INSTANCE_varname &&r) = default;

        INTERNAL_EZ_INSTANCE_varname &operator=(value_type const &r) { return _value = r, *this; }
        INTERNAL_EZ_INSTANCE_varname &operator=(value_type &&r) { return _value = std::move(r), *this; }

        // attribute
        std::u8string Attr1;
        struct INTERNAL_EZ_ATTR_Attr1 {
            static inline ezdata::impl::object_inst_attr_init<INTERNAL_EZ_ATTR_Attr1, EZDATA_ALIGN>
                _init{
                    INTERNAL_EZ_node_list, u8"Attr1", u8"Attr1Default"};
        };
    } d;
    */

    EZDATA_ADD(fooa, u8"Fooa", 3.141, EZDATA_ATTR(Attr1, u8"Hell, world!"));
};

static void vo()
{
    some_type r;
}