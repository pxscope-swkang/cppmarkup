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
 * Json �� XML ������Ʈ Ʈ�� ��ü �Ǵ� �κ��� �Ľ� �� ����ȭ�ϴ� �������̽��� �����մϴ�.
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
 * ������ cppmarkup ������Ʈ�� ���̳ʸ��� ����ȭ�ϴ� �������̽��� �����մϴ�.
 * ȣȯ���� ���� ��ü ��������, ���� ��ſ��� ������ cppmarkup ������Ʈ��
 *�����ų �ʿ䰡 ���� �� ����մϴ�. (BSON�� Ŀ���� ����)
 */
enum compact_byte : uint8_t;
using compact_binary = std::vector<compact_byte>;

/**
 * Ÿ�Կ� ���� �Ľ�, ���� ������ �����ϴ� ���ȸ� Ŭ�����Դϴ�.
 * �� Ÿ�Կ� ���� �Ľ� ������ marshaller ���� �Լ��� ���ǵǾ�� �մϴ�.
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
 * �ν��Ͻ�ȭ�Ǵ� ������ Ŭ�����Դϴ�.
 * marshaller_base�� ���ǵ� ���� �� �ʿ��� �κ��� overrride�ϰ�, �ݵ�� marshaller_base�� ����ؾ� �մϴ�.
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
 * ������Ʈ ���� embed �� �ϳ��� ��忡 ���� �Ľ�/���� ������Ƽ
 */
struct node_property {
    // ��� �ؽ�. ��ü ������Ʈ�� �Ϻκ��� ����ȭ�� ���, �ؽ��� ���� �Ϻ� ��������
    //��Ȯ�� ��Ʈ ��带 ã�� �� ����մϴ�. �±׷κ��� ������ �˰����� �����
    //������ ������ ��� �ݵ�� ���� �ؽ� ��ȯ�ؾ� �մϴ�.
    uint64_t hash;
    node_type type;

    std::u8string_view tag;
    std::u8string_view description; // ������ �׻� ���ڿ� ���ͷ� ...

    size_t offset;
    size_t next_offset;
    size_t value_size; // ������� padding���� ���� ����Ȯ�� ���� .. ��� �뵵�θ� ���.
    size_t total_size;

    std::vector<std::pair<std::u8string_view, std::u8string_view>> attrib_default;
    std::u8string* get_attrib(void* element_base, size_t index) const;

    marshaller_base const* pmarshal;
};

/**
 * ������Ʈ ���ø� ���� �� ���� �����ϴ� ��� Ŭ������ ���̽� Ÿ��.
 * 
 */
class object_base
{
public:
    virtual ~object_base() = default;

    virtual std::vector<node_property> const& properties() const = 0;
    virtual size_t depth() const                                 = 0;

    // TODO: ���⿡ DUMP �� PARSE �Լ��� �߰�
    // TODO: �Ľ�, ���� �� ����� �Ϻκи� �����ϴ� ���
    // How it works ...
    // object t; t.t0.t1�� ���� ������Ʈ�� ���� ��, t.dump(&t.t0.t1, markup_node)�� ����
    //��� ������Ʈ�� ���� �����ε� ��
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
    static inline const char8_t* INTERNAL_EZ_description_str       = {};

    struct INTERNAL_EZ_description_replacer {
        INTERNAL_EZ_description_replacer(const char8_t* str) { INTERNAL_EZ_description_str = str; }
    };

    std::vector<node_property> const& properties() const override { return INTERNAL_EZ_node_list; }
};

/**
 * CPPMARKUP_ADD�� �����Ǵ� ������Ʈ �ν��Ͻ��� ����մϴ�.
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
// TODO: ���ø� �Լ�, integral�� int64_t �о���� �Լ���, �ε� �Ҽ����� double��, ���ڿ��� u8string����,
template <typename Data_>
bool parse(Data_& d, pugi::xml_node const& s) { return false; }
bool parse(cppmarkup::impl::object_base& d, pugi::xml_node const& s);

// TODO: ������ ������ CPPMarkup ���� ���̿� �����͸� �ְ�ޱ� ���� ��� ���̳ʸ� �������̽�. ���� ��� ����
inline bool parse(cppmarkup::impl::object_base& d, compact_binary const& s) { return true; }
} // namespace cppmarkup::marshal

static constexpr size_t INTERNAL_EZ_depth = 0;
