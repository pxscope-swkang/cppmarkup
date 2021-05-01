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
    마셜링
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

    프로퍼티 트리 구성
    최초 인스턴스 생성 시 수행, 자연스럽게 가장 내부의 오브젝트부터 구성됨 (멤버 초기화자)
 */

namespace kangsw::markup {
namespace impl {
    class element_base;
}

/**
 * Array access 구조체. Object 배열의 경우 vector<ActualObjectType>으로 표현되지만, 리플렉션을 수행하는
 *코드에서는 실제 타입을 알 수 없습니다. vector<object>와 같은 식으로 표현하는 것도 불가하므로, 하기하는
 *콜백 함수에 실제 메모리의 manipulator를 저장합니다.
 * 단, 아래 함수는 반드시 대상 메모리가 object array 형식일 때에만 사용되어야 합니다.
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
 * Map access 구조체. Array access 구조체와 같은 역할을 하며, 맵을 지정합니다.
 * TODO: 인터페이스 보강
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
 * 오브젝트, 또는 데이터의 메타데이터 하나를 나타내는 프로퍼티입니다.
 * 각각의 markup object에 대해 한 번만 실행됩니다.
 */
struct property {
    /** */
    element_type type;

    /**
     * 각각의 markup object와 데이터에 고유하게 부여되는 ID입니다.
     * 구조체의 일부만을 type_check 옵션과 함께 dump한 뒤, 해당 데이터를 다시 parse할 때 사용합니다.
     *
     * offset_hash는 매우 엄격한 해쉬로, 같은 오브젝트 인스턴스를 여러 위치에서 재사용 했더라도
     *offset_hash가 다르면 파싱이 불가능합니다.
     */
    uint64_t offset_hash;

    /** 태그. 반드시 문자열 literal로부터 빌드되어야 합니다. (code section을 포인터) */
    u8string tag;

    /** 해당 엘리먼트에 대한 설명. 위키 등을 빌드하기 위해 사용합니다. */
    u8string* description;

    /** 엘리먼트 기본값 초기화 함수 ... 템플릿으로 지정됨 */
    void (*pinitializer)(void*);

    /** 메모리 표현 */
    struct memory_representation {
        /** Value의 오브젝트 원점에 대한 오프셋 */
        size_t value_offset;

        /** 값 요소 크기 (어트리뷰트 제외) */
        size_t value_size;

        /** Owner 오브젝트에 대한 지역 오프셋 */
        size_t elem_offset;

        // /** 루트 오브젝트에 대한 전역 오프셋 */
        // size_t global_offset; // TODO 필요성 논의

        /** 요소 전체 크기 */
        size_t total_element_size;
    } memory;

    /** 단일 어트리뷰트 표현 */
    struct attribute_representation {
        /** 어트리뷰트 타입 */
        element_type type;

        /** 어트리뷰트 이름 */
        u8string_view name;

        /** 요소 주소에 대한 오프셋 */
        size_t offset;

        /** 크기 */
        size_t size;

        /** 어트리뷰트 기본 초기화자*/
        void (*pinitializer)(void*);

        /** 획득자 */
        [[nodiscard]] void const* get(void const* elem) const { return (char*)elem + offset; }
        [[nodiscard]] void* get(void* elem) const { return (char*)elem + offset; }

        template <typename Ty_>
        [[nodiscard]] Ty_ const* as(void const* m) const;
        template <typename Ty_>
        [[nodiscard]] Ty_* as(void* m) const { return as<Ty_>(static_cast<object const*>(m)); }
    };

    /** 어트리뷰트 목록 */
    std::vector<attribute_representation> attributes;

    /** 어레이 매니퓰레이터 획득 */
    auto as_array() const { return _vector_manip; }

    /** 맵 매니퓰레이터 획득 */
    auto as_map() const { return _map_manip; }

    /** 프로퍼티 주소 획득 */
    [[nodiscard]] void const* get(object const* m) const { return (char*)m + memory.elem_offset + memory.value_offset; }
    [[nodiscard]] void* get(object* m) const { return const_cast<void*>(get(static_cast<object const*>(m))); }

    /** 프로퍼티 형변환 획득. 반드시 형식이 일치해야 합니다. */
    template <typename Ty_>
    [[nodiscard]] Ty_ const* as(object const* m) const;
    template <typename Ty_>
    [[nodiscard]] Ty_* as(object* m) const { return as<Ty_>(static_cast<object const*>(m)); }

private:
    friend class impl::element_base;

    /** 벡터 매니퓰레이터 */
    object_vector_manip const* _vector_manip;

    /** 맵 매니퓰레이터 */
    object_map_manip const* _map_manip;
};

/**
 * 마샬러 기본 템플릿 인터페이스.
 * 특수화되지 않은 함수의 사용은 항상 실패합니다.
 */
template <typename Markup_>
marshalerr_t parse(object& to, Markup_ const& from) { static_assert(false); }

template <typename Markup_>
marshalerr_t dump(Markup_& to, object const& from) { static_assert(false); }

/**
 * XML, 또는 JSON 오브젝트를 나타냅니다.
 * 다른 오브젝트 또는 데이터의 컨테이너입니다.
 */
class object {
public:
    virtual ~object() noexcept = default;

    virtual std::vector<property> const& props() const = 0;

    virtual uint64_t structure_hash() const = 0;

    void reset();

    property const* find_property(u8string_view name);

    // TODO: 구조 해시를 통한 compact martialing method ... Key 마셜링 없이 고속으로 value만 마셜 수행

private: // 내부 노출 전용 //
    friend class impl::element_base;

    virtual std::vector<property>& _props() = 0;
    virtual uint64_t& _structure_hash()     = 0;
};

/**
 * 오브젝트 타입을 반환
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

/** internals 구현 */
namespace impl {
    /**
     * 매크로 정의 시 임시 오브젝트가 상속하는 기본 클래스입니다.
     * 현재 스코프에서 현재 클래스 프로퍼티의 노드 리스트가 보이게 합니다.
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

        object_base() noexcept// 가장 먼저 호출 보장
        {
        }

        ~object_base() noexcept // 가장 늦게 호출 보장
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
     * 오브젝트 스코프 내에 인스턴싱되는 모든 엘리먼트가 상속하는 기본 클래스입니다.
     * 최초 생성 시
     * @tparam TempClassId_ 임시 클래스 이름으로, CRTP 적용하여 각 엘리먼트에 정적 플래그 부여
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
            // struct hash 계산.
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
     * 인스턴싱되는 어트리뷰트가 상속하는 클래스.
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

    /** 각 엘리먼트 인스턴스에 대해 고유한 스태틱 변수를 할당하기 위한 클래스 */
    template <typename TempClassId_>
    class element_template_base : protected element_base {
    protected:
        static inline u8string _description;
    };

    /** Ty_가 컨테이너 타입이고, element_type이 object인지 확인 */
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

    /** value type이 오브젝트 어레이일 때만 유효한 매니퓰레이터 레퍼런스를 반환 */
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
                // return &m; TODO: map_manip 구현
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
