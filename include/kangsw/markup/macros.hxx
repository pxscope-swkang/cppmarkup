#pragma once
#include "reflection/static_object_base.hxx"
#include "reflection/property_proxy.hxx"

// INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(object_type)
#define INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(object_type) \
    struct object_type : ::kangsw::refl::static_object_base<object_type>

// INTERNAL_CPPMARKUP_ELEMENT(elem_var, elem_name, default_value, flags)
// INTERNAL_CPPMARKUP_ELEMENT_WITH_ATTR(elem_var, elem_name, default_value, flags, ...)

// > INTERNAL_CPPMARKUP_ENTITY_former(elem_var, ...)

#define INTERNAL_CPPMARKUP_ENTITY_former(elem_var, elem_name) \
    constexpr static auto _##elem_var##_TAG = elem_name;      \
    struct _##elem_var##_HASH_TYPE {}

#define INTERNAL_CPPMARKUP_ENTITY_former_ATTR(elem_var, elem_name, ...)                     \
    INTERNAL_CPPMARKUP_ENTITY_former(elem_var, elem_name);                                  \
    struct _##elem_var##_ATTRIBUTES {                                                       \
        using _internal_ATTRIBUTES_TYPE = _##elem_var##_ATTRIBUTES;                         \
        static size_t _internal_ATTRSTRUCT_OFFSET() { return _##elem_var##_ATTR_OFFSET(); } \
        constexpr static auto _owner_element_TAG = elem_name;                               \
        __VA_ARGS__;                                                                        \
    } elem_var##_;                                                                          \
    static size_t _##elem_var##_ATTR_OFFSET() { return offsetof(self_type, elem_var##_); }

#define INTERNAL_CPPMARKUP_ENTITY_former_NOATTR(elem_var, elem_name) \
    INTERNAL_CPPMARKUP_ENTITY_former(elem_var, elem_name);           \
    static size_t _##elem_var##_ATTR_OFFSET() { return 0; }

// >
// INTERNAL_CPPMARKUP_ATTRIBUTE(attrib_var, attrib_name, default_value)
#define INTERNAL_CPPMARKUP_ATTRIBUTE(attrib_var, attrib_name, default_value)                    \
    using _##attrib_var##_VALUE_TYPE = decltype(::kangsw::refl::etype::deduce(default_value));  \
    struct _##attrib_var##_HASH_TYPE {};                                                        \
    _##attrib_var##_VALUE_TYPE attrib_var;                                                      \
                                                                                                \
    static size_t _##attrib_var##_OFFSET() {                                                    \
        return _internal_ATTRSTRUCT_OFFSET() + offsetof(_internal_ATTRIBUTES_TYPE, attrib_var); \
    }                                                                                           \
                                                                                                \
    static inline ::kangsw::refl::attribute_registration_t<                                     \
      self_type, _##attrib_var##_VALUE_TYPE, _##attrib_var##_HASH_TYPE>                         \
      _##attrib_var##_REGISTER{                                                                 \
        _owner_element_TAG,                                                                     \
        attrib_name,                                                                            \
        _##attrib_var##_OFFSET(),                                                               \
        ::kangsw::refl::etype::deduce(default_value)};

// > INTERNAL_CPPMARKUP_ENTITY_latter(elem_var, elem_name, flags)
#define INTERNAL_CPPMAKRUP_ENTITY_latter(elem_var, flags)                          \
    _##elem_var##_VALUE_TYPE elem_var;                                             \
                                                                                   \
    static size_t _##elem_var##_OFFSET() { return offsetof(self_type, elem_var); } \
    static inline ::kangsw::refl::element_regestration_t<                          \
      self_type, _##elem_var##_VALUE_TYPE, _##elem_var##_HASH_TYPE>                \
      _##elem_var##_REGISTER {                                                     \
        _##elem_var##_TAG,                                                         \
          _##elem_var##_OFFSET(),                                                  \
          ::kangsw::refl::etype::deduce(_##elem_var##_DEFAULT_VALUE()),            \
          flags                                                                    \
    }
// >

// Custom
#define INTERNAL_CPPMARKUP_ELEMENT_ATTR(elem_var, elem_name, default_value, flags, ...)      \
    INTERNAL_CPPMARKUP_ENTITY_former_ATTR(elem_var, elem_name, ##__VA_ARGS__);               \
    using _##elem_var##_VALUE_TYPE = decltype(::kangsw::refl::etype::deduce(default_value)); \
                                                                                             \
    static inline const auto _##elem_var##_DEFAULT_VALUE = []() { return default_value; };   \
    INTERNAL_CPPMAKRUP_ENTITY_latter(elem_var, flags)

#define INTERNAL_CPPMARKUP_ELEMENT_NOATTR(elem_var, elem_name, default_value, flags)         \
    INTERNAL_CPPMARKUP_ENTITY_former_NOATTR(elem_var, elem_name);                            \
    using _##elem_var##_VALUE_TYPE = decltype(::kangsw::refl::etype::deduce(default_value)); \
                                                                                             \
    static inline const auto _##elem_var##_DEFAULT_VALUE = []() { return default_value; };   \
    INTERNAL_CPPMAKRUP_ENTITY_latter(elem_var, flags)

#define INTERNAL_CPPMARKUP_EMBED_OBJECT_begin_ATTR(elem_var, elem_name, flags, ...) \
    static constexpr auto _##elem_var##_FLAGS = flags;                              \
    INTERNAL_CPPMARKUP_ENTITY_former_ATTR(elem_var, elem_name, ##__VA_ARGS__);      \
    INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(_##elem_var##_VALUE_TYPE)

#define INTERNAL_CPPMARKUP_EMBED_OBJECT_begin_NOATTR(elem_var, elem_name, flags) \
    static constexpr auto _##elem_var##_FLAGS = flags;                           \
    INTERNAL_CPPMARKUP_ENTITY_former_NOATTR(elem_var, elem_name);                \
    INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(_##elem_var##_VALUE_TYPE)

#define INTERNAL_CPPMARKUP_EMBED_OBJECT_end(elem_var)           \
    ;                                                           \
    static inline const auto _##elem_var##_DEFAULT_VALUE =      \
      []() { return _##elem_var##_VALUE_TYPE::get_default(); }; \
    INTERNAL_CPPMAKRUP_ENTITY_latter(elem_var, _##elem_var##_FLAGS)

namespace kangsw::refl::_internal {
template <typename DTy_, typename Ty_, typename... Args_>
auto _deduce_map_impl(u8str_map<DTy_>& acc, u8str_view a, Ty_&& b, Args_&&... args) {
    acc.emplace(a, etype::deduce(std::forward<Ty_>(b)));
    if constexpr (sizeof...(args) > 2)
        return _deduce_map_impl(acc, std::forward<Args_>(args)...);
    else
        return acc;
}

template <typename Ch_, size_t N, typename Ty_, typename... Args_>
auto deduce_map(Ch_ const (&a)[N], Ty_&& b, Args_&&... args) {
    using deduced_t = decltype(etype::deduce(b));
    u8str_map<deduced_t> map;
    return _deduce_map_impl<deduced_t>(
      map, reinterpret_cast<char const*>(a),
      (std::forward<Ty_>(b)),
      std::forward<Args_>(args)...);
}

template <size_t N>
#if __cplusplus >= 202000
constexpr char const* to_cstr(char8_t const (&str)[N])
#else
constexpr char const* to_cstr(char const (&str)[N])
#endif
{
    return (char const*)str;
}
} // namespace kangsw::refl::_internal

#define INTERNAL_CPPMARKUP_MAP(...) ::kangsw::refl::_internal::deduce_map(__VA_ARGS__)
#define CPPMARKUP_MAP(...)          INTERNAL_CPPMARKUP_MAP(__VA_ARGS__)

#define CPPMARKUP_OBJECT_TEMPLATE(objtype) INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(objtype)

#define CPPMARKUP_ELEMENT_AF(tag, default_value, flags, ...) INTERNAL_CPPMARKUP_ELEMENT_ATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), default_value, flags, ##__VA_ARGS__);
#define CPPMARKUP_ELEMENT_A(tag, default_value, ...)         INTERNAL_CPPMARKUP_ELEMENT_ATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), default_value, 0, ##__VA_ARGS__);
#define CPPMARKUP_ELEMENT_F(tag, default_value, flags)       INTERNAL_CPPMARKUP_ELEMENT_NOATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), default_value, flags);
#define CPPMARKUP_ELEMENT(tag, default_value)                INTERNAL_CPPMARKUP_ELEMENT_NOATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), default_value, 0);
#define CPPMARKUP_ATTRIBUTE(name, default_value)             INTERNAL_CPPMARKUP_ATTRIBUTE(name, ::kangsw::refl::_internal::to_cstr(u8## #name), default_value);

#define CPPMARKUP_EMBED_OBJECT_AF_begin(tag, flags, ...) INTERNAL_CPPMARKUP_EMBED_OBJECT_begin_ATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), flags, __VA_ARGS__)
#define CPPMARKUP_EMBED_OBJECT_A_begin(tag, ...)         INTERNAL_CPPMARKUP_EMBED_OBJECT_begin_ATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), 0, __VA_ARGS__)
#define CPPMARKUP_EMBED_OBJECT_F_begin(tag, flags)       INTERNAL_CPPMARKUP_EMBED_OBJECT_begin_NOATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), flags)
#define CPPMARKUP_EMBED_OBJECT_begin(tag)                INTERNAL_CPPMARKUP_EMBED_OBJECT_begin_NOATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), 0)
#define CPPMARKUP_EMBED_OBJECT_end(tag)                  INTERNAL_CPPMARKUP_EMBED_OBJECT_end(tag)

// #define CPPMARKUP_EMBED_OBJECT_AF_begin(tag, flags, ...) INTERNAL_CPPMARKUP_EMBED_OBJECT_begin_ATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), flags, __VA_ARGS__)
// #define CPPMARKUP_EMBED_OBJECT_A_begin(tag, ...)         INTERNAL_CPPMARKUP_EMBED_OBJECT_begin_ATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), 0, __VA_ARGS__)
// #define CPPMARKUP_EMBED_OBJECT_F_begin(tag, flags)       INTERNAL_CPPMARKUP_EMBED_OBJECT_begin_NOATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), flags)
// #define CPPMARKUP_EMBED_OBJECT_begin(tag)                INTERNAL_CPPMARKUP_EMBED_OBJECT_begin_NOATTR(tag, ::kangsw::refl::_internal::to_cstr(u8## #tag), 0)
// #define CPPMARKUP_EMBED_OBJECT_end(tag)                  INTERNAL_CPPMARKUP_EMBED_OBJECT_end(tag)
//
// #define INTERNAL_CPPMARKUP_WRAP_ATTRIBUTES(...) __VA_ARGS__
// #define CPPMARKUP_WRAP_ATTRIBUTES(...)          INTERNAL_CPPMARKUP_WRAP_ATTRIBUTES(__VA_ARGS__)
//
// #define CPPMARKUP_EMBED_OBJECT_AF(tag, flags, attrs, ...) CPPMARKUP_EMBED_OBJECT_AF_begin(tag, flags, attrs){__VA_ARGS__} CPPMARKUP_EMBED_OBJECT_end(tag);
// #define CPPMARKUP_EMBED_OBJECT_F(tag, flags, ...)         CPPMARKUP_EMBED_OBJECT_F_begin(tag, flags){__VA_ARGS__} CPPMARKUP_EMBED_OBJECT_end(tag);
// #define CPPMARKUP_EMBED_OBJECT_A(tag, attrs, ...)         CPPMARKUP_EMBED_OBJECT_AF(tag, 0, attrs, __VA_ARGS__)
// #define CPPMARKUP_EMBED_OBJECT(tag, ...)                  CPPMARKUP_EMBED_OBJECT_F(tag, 0, __VA_ARGS__)
