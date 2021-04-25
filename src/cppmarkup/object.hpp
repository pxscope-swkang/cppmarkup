#pragma once
#include <atomic>
#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <typeinfo>
#include <vector>

#include "hash.hpp"
#include "marshal_defaults.hpp"
#include "template_utils.hxx"
#include "typedefs.hpp"

/*
- References - 

    CPPMARKUP_OBJECT_TEMPLATE(template_type_name)
    CPPMARKUP_NESTED_OBJECT(variable_name, display_name, attributes...)
    CPPMARKUP_ADD(variable_name, display_name, default_value, attributes...)
    CPPMARKUP_ATTR(attr_name, display_name, attr_default_value)
    CPPMARKUP_DESCRIPTION_BELOW(descr);
    CPPMARKUP_ALIGNMENT

    template<>
    struct marshal<int>
    {
        static bool parse(int&d, pugi::xml_node const&) {}
        static bool parse(pugi::xml_node&, int const&) {}
    }

*/

/*
 - Notes -
    ���ȸ�
    template<typename Mashal_>
    struct marshal_instance : base_marshaller
    {
        virtual bool parse(void* r, size_t t, pugi::xml_node const& s)
        {
            assert(t == Marshal_::element_size);
            using trait = marshaller_traits<Marshal_>;
            return Marshal_::parse(*(typename trait::pointer)r, s);
        }
    }

    ������Ƽ Ʈ�� ����
    ���� �ν��Ͻ� ���� �� ����, �ڿ������� ���� ������ ������Ʈ���� ������ (��� �ʱ�ȭ��)
 */

namespace kangsw::markup {
/**
 * ����� ������ ��Ÿ���ϴ�. ������Ʈ ~ �� ������Ʈ, ��̸� �����ϴ� �뵵�ո� ���.
 */
enum class element_type {
    null,
    object,
    array,
    data,
};

/**
 * ������Ʈ, �Ǵ� �������� ��Ÿ������ �ϳ��� ��Ÿ���� ������Ƽ�Դϴ�.
 * ������ markup object�� ���� �� ���� ����˴ϴ�.
 */
struct property {
    /** */
    element_type type;

    /**
     * ������ markup object�� �����Ϳ� �����ϰ� �ο��Ǵ� ID�Դϴ�.
     * ����ü�� �Ϻθ��� type_check �ɼǰ� �Բ� dump�� ��, �ش� �����͸� �ٽ� parse�� �� ����մϴ�.
     *
     * offset_hash�� �ſ� ������ �ؽ���, ���� ������Ʈ �ν��Ͻ��� ���� ��ġ���� ���� �ߴ���
     *offset_hash�� �ٸ��� �Ľ��� �Ұ����մϴ�.
     */
    uint64_t offset_hash;

    /** �±�. �ݵ�� ���ڿ� literal�κ��� ����Ǿ�� �մϴ�. (code section�� ������) */
    u8string tag;

    /** �ش� ������Ʈ�� ���� ����. ��Ű ���� �����ϱ� ���� ����մϴ�. */
    u8string* description;

    /** ������Ʈ �⺻�� �ʱ�ȭ �Լ� ... ���ø����� ������ */
    void (*pinitializer)(void*);

    /** �޸� ǥ�� */
    struct memory_representation {
        /** Value�� ������Ʈ ������ ���� ������ */
        size_t value_offset;

        /** �� ��� ũ�� (��Ʈ����Ʈ ����) */
        size_t value_size;

        /** Owner ������Ʈ�� ���� ���� ������ */
        size_t local_offset;

        // /** ��Ʈ ������Ʈ�� ���� ���� ������ */
        // size_t global_offset; // TODO �ʿ伺 ����

        /** ��� ��ü ũ�� */
        size_t total_element_size;
    } memory;

    /** ��� ������ */
    impl::basic_marshaller const* pmarshal;

    /** ���� ��Ʈ����Ʈ ǥ�� */
    struct attribute_representation {
        /** ��Ʈ����Ʈ �̸� */
        u8string_view name;

        /** ��� �ּҿ� ���� ������ */
        size_t offset;

        /** ũ�� */
        size_t size;

        /** ��Ʈ����Ʈ Ÿ�� ������ */
        impl::basic_marshaller const* pmarshal;

        /** ��Ʈ����Ʈ �⺻ �ʱ�ȭ��*/
        void (*pinitializer)(void*);
    };

    /** ��Ʈ����Ʈ ��� */
    std::vector<attribute_representation> attributes;
};

using extended_parse_fn_t = bool (*)(void*, size_t, void const*);
using extended_dump_fn_t  = bool (*)(void const*, size_t, void*);

namespace impl {
    class basic_marshaller {
    public:
        virtual ~basic_marshaller()                                         = default;
        virtual bool parse(void* v, size_t n, u8string_view s) const        = 0;
        virtual bool parse(void* v, size_t n, compact_binary_view s) const  = 0;
        virtual bool dump(void const* v, size_t n, u8string& s) const       = 0;
        virtual bool dump(void const* v, size_t n, compact_binary& s) const = 0;

        template <typename Markup_>
        bool parse_by(void* v, size_t n, Markup_ const& s) const
        {
            return _parse_by(v, n, &s, typeid(Markup_).hash_code());
        }

        template <typename Markup_>
        bool dump_by(void const* v, size_t n, Markup_& s) const
        {
            return _dump_by(v, n, &s, typeid(Markup_).hash_code());
        }

    protected:
        virtual bool _parse_by(void* v, size_t n, void const* s, size_t markup_hash) const = 0;
        virtual bool _dump_by(void const* v, size_t n, void* s, size_t markup_hash) const  = 0;
    };

    template <typename Ty_>
    class marshaller_instance : public basic_marshaller {
    public:
        bool parse(void* v, size_t n, u8string_view s) const override { return assert(sizeof(Ty_) == n), ::kangsw::markup::parse(*(Ty_*)v, s); }
        bool parse(void* v, size_t n, compact_binary_view s) const override { return assert(sizeof(Ty_) == n), ::kangsw::markup::parse(*(Ty_*)v, s); }

        bool dump(void const* v, size_t n, u8string& s) const override { return assert(sizeof(Ty_) == n), ::kangsw::markup::dump(*(Ty_*)v, s); }
        bool dump(void const* v, size_t n, compact_binary& s) const override { return assert(sizeof(Ty_) == n), ::kangsw::markup::dump(*(Ty_*)v, s); }

        bool _parse_by(void* v, size_t n, void const* s, size_t markup_hash) const override
        {
            auto fit = _ext_marshal.find(markup_hash);
            if (fit == _ext_marshal.end()) { return false; }

            return fit->second.first(v, n, s);
        }

        bool _dump_by(void const* v, size_t n, void* s, size_t markup_hash) const override
        {
            auto fit = _ext_marshal.find(markup_hash);
            if (fit == _ext_marshal.end()) { return false; }

            return fit->second.second(v, n, s);
        };

    private:
        static inline std::map<
            size_t /*source type hash code*/,
            std::pair<extended_parse_fn_t, extended_dump_fn_t>>
            _ext_marshal;
    };
} // namespace impl

/**
 * XML, �Ǵ� JSON ������Ʈ�� ��Ÿ���ϴ�.
 * �ٸ� ������Ʈ �Ǵ� �������� �����̳��Դϴ�.
 */
class object {
public:
    virtual ~object()                                  = default;
    virtual std::vector<property> const& props() const = 0;
    virtual uint64_t structure_hash() const            = 0;

    // TODO: �κ��� marshalling, ����/���� Ÿ��üũ marshalling, �κ��� ����/���� Ÿ��üũ marshalling

private: // ���� ���� ���� //
    friend class impl::element_base;

    virtual std::vector<property>& _props() = 0;
    virtual uint64_t& _structure_hash()     = 0;
};

/** ������Ʈ ���ȸ� Ư��ȭ */
template <> bool parse<object>(object& o, u8string_view i);
template <> bool dump(object const& o, u8string& i);

/**
 * ������Ʈ Ÿ���� ��ȯ
 */
template <typename Ty_>
element_type get_element_type()
{
    if constexpr (std::is_base_of_v<object, Ty_>) { return element_type::object; }
    if constexpr (templates::is_specialization<Ty_, std::vector>::value) { return element_type::array; }
    if constexpr (std::is_same_v<nullptr_t, Ty_>) { return element_type::null; }
    return element_type::data;
}

/** internals ���� */
namespace impl {
    /**
     * ��ũ�� ���� �� �ӽ� ������Ʈ�� ����ϴ� �⺻ Ŭ�����Դϴ�.
     * ���� ���������� ���� Ŭ���� ������Ƽ�� ��� ����Ʈ�� ���̰� �մϴ�.
     */
    template <typename ObjClass_>
    struct object_base : object {
        static inline std::vector<property> INTERNAL_props;
        static inline u8string INTERNAL_next_description;
        static inline bool INTERNAL_is_first_entry     = true;
        static inline uint64_t INTERNAL_structure_hash = 0;

        object_base() // ���� ���� ȣ�� ����
        {
        }

        ~object_base() // ���� �ʰ� ȣ�� ����
        {
            INTERNAL_is_first_entry = false;
        }

    private:
        std::vector<property>& _props() override { return INTERNAL_props; }
        uint64_t& _structure_hash() override { return INTERNAL_structure_hash; }

    public:
        std::vector<property> const& props() const override { return INTERNAL_props; }
        uint64_t structure_hash() const override { return INTERNAL_structure_hash; }

    private:
        static inline struct _init_t {
            _init_t() { ObjClass_{}; }
        } _exec;
    };

    /**
     * ������Ʈ ������ ���� �ν��Ͻ̵Ǵ� ��� ������Ʈ�� ����ϴ� �⺻ Ŭ�����Դϴ�.
     * ���� ���� ��
     * @tparam TempClassId_ �ӽ� Ŭ���� �̸�����, CRTP �����Ͽ� �� ������Ʈ�� ���� �÷��� �ο�
     */
    class element_base {
    protected:
        void INTERNAL_elembase_init(element_type type, object* base, u8string_view tag, u8string& description, size_t value_offset, size_t value_size, size_t total_size, basic_marshaller* pmarshal, void (*pinit)(void*), std::vector<property::attribute_representation>& attrs)
        {
            // struct hash ���.
            auto seed        = base->_props().empty() ? 0 : base->_props().back().offset_hash;
            auto offset_hash = get_hash(tag, seed);

            auto& structure_hash = base->_structure_hash();
            structure_hash       = get_hash(tag, structure_hash + value_offset + (value_size << 32));

            auto& n = base->_props().emplace_back();
            n.tag   = tag;
            n.type  = type;

            n.memory.local_offset       = (intptr_t)this - (intptr_t)base;
            n.memory.value_offset       = value_offset;
            n.memory.total_element_size = total_size;
            n.memory.value_size         = value_size;
            n.offset_hash               = offset_hash;
            n.pmarshal                  = pmarshal;
            n.description               = &description;
            n.pinitializer              = pinit;

            n.attributes.assign(attrs.begin(), attrs.end());
            attrs.clear();
            attrs.shrink_to_fit();
        }
    };

    /**
     * �ν��Ͻ̵Ǵ� ��Ʈ����Ʈ�� ����ϴ� Ŭ����.
     */
    class attribute_base {
    protected:
        void INTERNAL_attrbase_init(element_base* base, u8string_view name, std::vector<property::attribute_representation>& attrs, size_t attr_size, basic_marshaller* marshaller, void (*pinit)(void*))
        {
            auto& n        = attrs.emplace_back();
            n.offset       = (intptr_t)this - (intptr_t)base;
            n.size         = attr_size;
            n.name         = name;
            n.pmarshal     = marshaller;
            n.pinitializer = pinit;
        }
    };

    /** �� ������Ʈ �ν��Ͻ��� ���� ������ ����ƽ ������ �Ҵ��ϱ� ���� Ŭ���� */
    template <typename TempClassId_>
    class element_template_base : protected element_base {
    protected:
        static inline u8string _description;
    };

    /** ������ ��Ʈ����Ʈ�� ����ϴ� ���̽� Ŭ�����Դϴ�. */

} // namespace impl

} // namespace kangsw::markup
