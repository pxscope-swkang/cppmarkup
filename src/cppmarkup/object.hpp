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
/**
 * 노드의 형식을 나타냅니다. 오브젝트 ~ 비 오브젝트, 어레이를 구별하는 용도롱만 사용.
 */
enum class element_type {
    null,
    object,
    array,
    data,
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
        size_t local_offset;

        // /** 루트 오브젝트에 대한 전역 오프셋 */
        // size_t global_offset; // TODO 필요성 논의

        /** 요소 전체 크기 */
        size_t total_element_size;
    } memory;

    /** 요소 마샬러 */
    impl::basic_marshaller const* pmarshal;

    /** 단일 어트리뷰트 표현 */
    struct attribute_representation {
        /** 어트리뷰트 이름 */
        u8string_view name;

        /** 요소 주소에 대한 오프셋 */
        size_t offset;

        /** 크기 */
        size_t size;

        /** 어트리뷰트 타입 마샬러 */
        impl::basic_marshaller const* pmarshal;

        /** 어트리뷰트 기본 초기화자*/
        void (*pinitializer)(void*);
    };

    /** 어트리뷰트 목록 */
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
 * XML, 또는 JSON 오브젝트를 나타냅니다.
 * 다른 오브젝트 또는 데이터의 컨테이너입니다.
 */
class object {
public:
    virtual ~object()                                  = default;
    virtual std::vector<property> const& props() const = 0;
    virtual uint64_t structure_hash() const            = 0;

    // TODO: 부분적 marshalling, 엄격/느슨 타입체크 marshalling, 부분적 엄격/느슨 타입체크 marshalling

private: // 내부 노출 전용 //
    friend class impl::element_base;

    virtual std::vector<property>& _props() = 0;
    virtual uint64_t& _structure_hash()     = 0;
};

/** 오브젝트 마셜링 특수화 */
template <> bool parse<object>(object& o, u8string_view i);
template <> bool dump(object const& o, u8string& i);

/**
 * 오브젝트 타입을 반환
 */
template <typename Ty_>
element_type get_element_type()
{
    if constexpr (std::is_base_of_v<object, Ty_>) { return element_type::object; }
    if constexpr (templates::is_specialization<Ty_, std::vector>::value) { return element_type::array; }
    if constexpr (std::is_same_v<nullptr_t, Ty_>) { return element_type::null; }
    return element_type::data;
}

/** internals 구현 */
namespace impl {
    /**
     * 매크로 정의 시 임시 오브젝트가 상속하는 기본 클래스입니다.
     * 현재 스코프에서 현재 클래스 프로퍼티의 노드 리스트가 보이게 합니다.
     */
    template <typename ObjClass_>
    struct object_base : object {
        static inline std::vector<property> INTERNAL_props;
        static inline u8string INTERNAL_next_description;
        static inline bool INTERNAL_is_first_entry     = true;
        static inline uint64_t INTERNAL_structure_hash = 0;

        object_base() // 가장 먼저 호출 보장
        {
        }

        ~object_base() // 가장 늦게 호출 보장
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
     * 오브젝트 스코프 내에 인스턴싱되는 모든 엘리먼트가 상속하는 기본 클래스입니다.
     * 최초 생성 시
     * @tparam TempClassId_ 임시 클래스 이름으로, CRTP 적용하여 각 엘리먼트에 정적 플래그 부여
     */
    class element_base {
    protected:
        void INTERNAL_elembase_init(element_type type, object* base, u8string_view tag, u8string& description, size_t value_offset, size_t value_size, size_t total_size, basic_marshaller* pmarshal, void (*pinit)(void*), std::vector<property::attribute_representation>& attrs)
        {
            // struct hash 계산.
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
     * 인스턴싱되는 어트리뷰트가 상속하는 클래스.
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

    /** 각 엘리먼트 인스턴스에 대해 고유한 스태틱 변수를 할당하기 위한 클래스 */
    template <typename TempClassId_>
    class element_template_base : protected element_base {
    protected:
        static inline u8string _description;
    };

    /** 내부의 어트리뷰트가 상속하는 베이스 클래스입니다. */

} // namespace impl

} // namespace kangsw::markup
