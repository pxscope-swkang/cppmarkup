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
 * ������Ʈ�� ���ο� ���Ե� �ν��Ͻ��� �Ľ� �� ����ȭ�� �����ϱ� ���� ���� Ŭ�����Դϴ�.
 * ������Ʈ ��ü�� �����͸� ���� ������, �Ľ� �� ����ȭ ������ ��� CRTP �ν��Ͻ��� static inline
 *vector�� ����˴ϴ�.
 * ������ ������ Ÿ���� �Ľ��ϱ� ���� ������Ʈ�� ���̴� ��� ������ ������ bool(std::span<std::byte>,
 *std::string_view)�� �����ؾ� �մϴ�. ����ȭ�� ���ؼ� void(std::string, std::span<const std::byte>)
 *�Լ��� �����Ǿ�� �մϴ�.
 *
 * �⺻������, cppmarkup::marshal ���ӽ����̽��� parse �� serialize �Լ��� ã���ϴ�.
 *
 * �Ľ̿� ������ ��� ���ܸ� �����ų�, �ܼ��� ���� ����ü�� ��ȯ�� �� �ֽ��ϴ�. (optional)
 *
 * Json �� XML ������Ʈ Ʈ�� ��ü�� �Ľ� �� ����ȭ�ϴ� �������̽��� �����մϴ�. 
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
 * ������Ʈ ���� embed �� �ϳ��� ��忡 ���� �Ľ�/���� ������Ƽ
 */
struct node_property {
    std::u8string_view tag;
    std::u8string_view description; // ������ �׻� ���ڿ� ���ͷ� ...

    size_t offset;
    size_t next_offset;
    size_t value_size; // ������� padding���� ���� ����Ȯ�� ���� .. ��� �뵵�θ� ���.
    size_t total_size;

    std::vector<std::pair<std::u8string_view, std::u8string_view>> attrib_default;

    struct {
        parse_fn_t<pugi::xml_node> parser;
        dump_fn_t<pugi::xml_node> dumper;
    } xml;

    std::u8string* get_attrib(void* element_base, size_t index) const;
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

    // TODO: ���⿡ DUMP �� PARSE �Լ��� �߰�.
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
// TODO: ���ø� �Լ�, integral�� int64_t �о���� �Լ���, �ε� �Ҽ����� double��, ���ڿ��� u8string����,
template<typename Data_>
bool parse(Data_& d, pugi::xml_node const& s) { return false; }
bool parse(cppmarkup::impl::object_base& d, pugi::xml_node const& s);

// TODO: ������ ������ CPPMarkup ���� ���̿� �����͸� �ְ�ޱ� ���� ��� ���̳ʸ� �������̽�. ���� ��� ����
inline bool parse(cppmarkup::impl::object_base& d, compact_binary const& s) { return true; }
} // namespace cppmarkup::marshal

static constexpr size_t INTERNAL_EZ_depth = 0;
