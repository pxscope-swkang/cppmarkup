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

#include <functional>

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
namespace impl {
    class element_base;
}

/**
 * Array access ����ü. Object �迭�� ��� vector<ActualObjectType>���� ǥ��������, ���÷����� �����ϴ�
 *�ڵ忡���� ���� Ÿ���� �� �� �����ϴ�. vector<object>�� ���� ������ ǥ���ϴ� �͵� �Ұ��ϹǷ�, �ϱ��ϴ�
 *�ݹ� �Լ��� ���� �޸��� manipulator�� �����մϴ�.
 * ��, �Ʒ� �Լ��� �ݵ�� ��� �޸𸮰� object array ������ ������ ���Ǿ�� �մϴ�.
 */
struct object_vector_manip {
    virtual ~object_vector_manip() = default;

    virtual size_t size(void const*) const                    = 0;
    virtual object const* at(void const*, size_t index) const = 0;
    virtual object* at(void*, size_t index) const             = 0;
    virtual size_t append(void*, object** out) const          = 0;
    virtual void clear(void*) const                           = 0;
    virtual void swap(void*, size_t a, size_t b) const        = 0;
    virtual void pop_back(void*) const                        = 0;
    virtual size_t erase(void*, size_t from, size_t to) const = 0;
};

/**
 * Map access ����ü. Array access ����ü�� ���� ������ �ϸ�, ���� �����մϴ�.
 * TODO: �������̽� ����
 */
struct object_map_manip {
    virtual ~object_map_manip() = default;

    virtual size_t size(void*) const                              = 0;
    virtual object* at(void*, u8string_view) const                = 0;
    virtual bool insert(void*, u8string_view, object** out) const = 0; // returns is_newly_inserted
    virtual bool contains(void*, u8string_view) const             = 0;

    virtual void for_each(void*, std::function<void(u8string_view, object*)>) const = 0;
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
        size_t elem_offset;

        // /** ��Ʈ ������Ʈ�� ���� ���� ������ */
        // size_t global_offset; // TODO �ʿ伺 ����

        /** ��� ��ü ũ�� */
        size_t total_element_size;
    } memory;

    /** ���� ��Ʈ����Ʈ ǥ�� */
    struct attribute_representation {
        /** ��Ʈ����Ʈ Ÿ�� */
        element_type type;

        /** ��Ʈ����Ʈ �̸� */
        u8string_view name;

        /** ��� �ּҿ� ���� ������ */
        size_t offset;

        /** ũ�� */
        size_t size;

        /** ��Ʈ����Ʈ �⺻ �ʱ�ȭ��*/
        void (*pinitializer)(void*);

        /** ȹ���� */
        [[nodiscard]] void const* get(void const* elem) const { return (char*)elem + offset; }
        [[nodiscard]] void* get(void* elem) const { return (char*)elem + offset; }

        template <typename Ty_>
        [[nodiscard]] Ty_ const* as(void const* m) const;
        template <typename Ty_>
        [[nodiscard]] Ty_* as(void* m) const { return as<Ty_>(static_cast<object const*>(m)); }
    };

    /** ��Ʈ����Ʈ ��� */
    std::vector<attribute_representation> attributes;

    /** ��� �Ŵ�ǽ������ ȹ�� */
    auto as_array() const { return _vector_manip; }

    /** �� �Ŵ�ǽ������ ȹ�� */
    auto as_map() const { return _map_manip; }

    /** ������Ƽ �ּ� ȹ�� */
    [[nodiscard]] void const* get(object const* m) const { return (char*)m + memory.elem_offset + memory.value_offset; }
    [[nodiscard]] void* get(object* m) const { return const_cast<void*>(get(static_cast<object const*>(m))); }

    /** ������Ƽ ����ȯ ȹ��. �ݵ�� ������ ��ġ�ؾ� �մϴ�. */
    template <typename Ty_>
    [[nodiscard]] Ty_ const* as(object const* m) const;
    template <typename Ty_>
    [[nodiscard]] Ty_* as(object* m) const { return as<Ty_>(static_cast<object const*>(m)); }

private:
    friend class impl::element_base;

    /** ���� �Ŵ�ǽ������ */
    object_vector_manip const* _vector_manip;

    /** �� �Ŵ�ǽ������ */
    object_map_manip const* _map_manip;
};

/**
 * ������ �⺻ ���ø� �������̽�.
 * Ư��ȭ���� ���� �Լ��� ����� �׻� �����մϴ�.
 */
template <typename Markup_>
marshalerr_t parse(object& to, Markup_ const& from) { static_assert(false); }

template <typename Markup_>
marshalerr_t dump(Markup_& to, object const& from) { static_assert(false); }

/**
 * XML, �Ǵ� JSON ������Ʈ�� ��Ÿ���ϴ�.
 * �ٸ� ������Ʈ �Ǵ� �������� �����̳��Դϴ�.
 */
class object {
public:
    virtual ~object() noexcept = default;

    virtual std::vector<property> const& props() const = 0;

    virtual uint64_t structure_hash() const = 0;

    void reset();

    property const* find_property(u8string_view name);

    // TODO: ���� �ؽø� ���� compact martialing method ... Key ���ȸ� ���� ������� value�� ���� ����

private: // ���� ���� ���� //
    friend class impl::element_base;

    virtual std::vector<property>& _props() = 0;
    virtual uint64_t& _structure_hash()     = 0;
};

/**
 * ������Ʈ Ÿ���� ��ȯ
 */
template <typename Ty_>
constexpr element_type get_element_type()
{
    if constexpr (std::is_base_of_v<object, Ty_>) {
        return element_type::object;
    }
    else if constexpr (std::is_same_v<Ty_, binary_chunk>) {
        return element_type::binary;
    }
    else if constexpr (std::is_same_v<Ty_, bool>) {
        return element_type::boolean;
    }
    else if constexpr (std::is_same_v<Ty_, int64_t>) {
        return element_type::integer | element_type::number;
    }
    else if constexpr (std::is_same_v<Ty_, double>) {
        return element_type::floating_point | element_type::number;
    }
    else if constexpr (std::is_same_v<nullptr_t, Ty_>) {
        return element_type::null;
    }
    else if constexpr (std::is_same_v<Ty_, u8string>) {
        return element_type::string;
    }
    else if constexpr (templates::is_specialization<Ty_, std::vector>::value) {
        return get_element_type<typename Ty_::value_type>() | element_type::array;
    }
    else if constexpr (templates::is_specialization<Ty_, std::map>::value) {
        return get_element_type<typename Ty_::mapped_type>() | element_type::map;
    }
    else {
        static_assert(false, "Not a valid element type");
        return element_type::null; // warning suppression
    }
}

/** internals ���� */
namespace impl {
    /**
     * ��ũ�� ���� �� �ӽ� ������Ʈ�� ����ϴ� �⺻ Ŭ�����Դϴ�.
     * ���� ���������� ���� Ŭ���� ������Ƽ�� ��� ����Ʈ�� ���̰� �մϴ�.
     */
    template <typename ObjClass_>
    struct object_base : public object {
        static inline bool INTERNAL_is_first_entry     = true;
        static inline uint64_t INTERNAL_structure_hash = 0;

        static auto& INTERNAL_next_description()
        {
            static u8string _value;
            return _value;
        };

        object_base() noexcept// ���� ���� ȣ�� ����
        {
        }

        ~object_base() noexcept // ���� �ʰ� ȣ�� ����
        {
            INTERNAL_is_first_entry = false;
        }

        static ObjClass_ get_default()
        {
            ObjClass_ obj;
            obj.reset();
            return obj;
        }

    private:
        std::vector<property>& _props() override { return (std::vector<property>&)props(); }
        uint64_t& _structure_hash() override { return INTERNAL_structure_hash; }

    public:
        std::vector<property> const& props() const override
        {
            static std::vector<property> INTERNAL_props;
            return INTERNAL_props;
        }
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
        void INTERNAL_elembase_init(
            element_type type,
            object* base,
            u8string_view tag,
            u8string& description,
            size_t value_offset,
            size_t value_size,
            size_t total_size,
            void (*pinit)(void*),
            std::vector<property::attribute_representation>& attrs,
            object_vector_manip const* vmanip,
            object_map_manip const* mmanip)
        {
            // printf("Initializing %s ...\n", tag.data());
            // struct hash ���.
            auto seed        = base->_props().empty() ? 0 : base->_props().back().offset_hash;
            auto offset_hash = get_hash(tag, seed);

            auto& structure_hash = base->_structure_hash();
            structure_hash       = get_hash(tag, structure_hash + value_offset + (value_size << 32));

            auto& array = base->_props();
            auto& n     = array.emplace_back();
            n.tag       = tag;
            n.type      = type;

            n.memory.elem_offset        = (intptr_t)this - (intptr_t)base;
            n.memory.value_offset       = value_offset;
            n.memory.total_element_size = total_size;
            n.memory.value_size         = value_size;
            n.offset_hash               = offset_hash;
            n.description               = &description;
            n.pinitializer              = pinit;

            n._vector_manip = vmanip;
            n._map_manip    = mmanip;

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
        void INTERNAL_attrbase_init(element_base* base, u8string_view name, element_type type, std::vector<property::attribute_representation>& attrs, size_t attr_size, void (*pinit)(void*))
        {
            auto& n        = attrs.emplace_back();
            n.type         = type;
            n.offset       = (intptr_t)this - (intptr_t)base;
            n.size         = attr_size;
            n.name         = name;
            n.pinitializer = pinit;
        }
    };

    /** �� ������Ʈ �ν��Ͻ��� ���� ������ ����ƽ ������ �Ҵ��ϱ� ���� Ŭ���� */
    template <typename TempClassId_>
    class element_template_base : protected element_base {
    protected:
        static inline u8string _description;
    };

    /** Ty_�� �����̳� Ÿ���̰�, element_type�� object���� Ȯ�� */
    enum class container_type {
        other,
        object_array,
        object_map,
    };

    template <typename Ty_>
    constexpr container_type check_object_container()
    {
        if constexpr (templates::is_specialization<Ty_, std::vector>::value) {
            return std::is_base_of_v<object, typename Ty_::value_type>
                       ? container_type::object_array
                       : container_type::other;
        }
        else if constexpr (templates::is_specialization<Ty_, std::map>::value) {
            return std::is_base_of_v<object, typename Ty_::mapped_type>
                       ? container_type::object_map
                       : container_type::other;
        }
        else {
            return container_type::other;
        }
    }

    /** value type�� ������Ʈ ����� ���� ��ȿ�� �Ŵ�ǽ������ ���۷����� ��ȯ */
    template <typename Ty_> struct object_array_instance {
        struct manip : object_vector_manip {
            using container_type = Ty_;
            static_assert(std::is_base_of_v<object, typename container_type::value_type>);

            size_t size(void const* m) const override { return ((container_type*)m)->size(); }
            object* at(void* m, size_t index) const override { return &((container_type*)m)->operator[](index); }
            object const* at(void const* m, size_t index) const override { return &((container_type*)m)->operator[](index); }
            void clear(void* m) const override { return ((container_type*)m)->clear(); }
            void pop_back(void* m) const override { return ((container_type*)m)->pop_back(); }
            size_t erase(void* m, size_t from, size_t to) const override
            {
                auto& v = *(container_type*)m;
                auto it = v.erase(v.begin() + from, v.begin() + to);
                return it - v.begin();
            }
            void swap(void* m, size_t a, size_t b) const override
            {
                auto& v = *(container_type*)m;
                std::swap(v[a], v[b]);
            }
            size_t append(void* m, object** out) const override
            {
                auto& v  = *(container_type*)m;
                auto idx = v.size();
                if (out) { *out = &v.emplace_back(); }
                return idx;
            }
        };

        static object_vector_manip const* get()
        {
            if constexpr (check_object_container<Ty_>() != container_type::object_array) {
                return nullptr;
            }
            else {
                static manip m;
                return &m;
            }
        }
    };

    template <typename Ty_> struct object_map_instance {
        struct manip : object_map_manip {};

        static object_map_manip const* get()
        {
            if constexpr (check_object_container<Ty_>() != container_type::object_array) {
                return nullptr;
            }
            else {
                //static manip m;
                // return &m; TODO: map_manip ����
                return nullptr;
            }
        }
    };

} // namespace impl

template <typename Ty_> Ty_ const* property::attribute_representation::as(void const* m) const
{
    if (get_element_type<Ty_>() != this->type) { return nullptr; }
    return static_cast<Ty_ const*>(get(m));
}

template <typename Ty_> Ty_ const* property::as(object const* m) const
{
    if (get_element_type<Ty_>() != this->type) { return nullptr; }
    return static_cast<Ty_ const*>(get(m));
}

} // namespace kangsw::markup
