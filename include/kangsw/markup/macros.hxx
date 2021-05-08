#pragma once
#include "reflection/static_object_base.hxx"

// INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(object_type)
#define INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(object_type) \
    struct object_type : ::kangsw::refl::static_object_base<object_type>

// INTERNAL_CPPMARKUP_ELEMENT(elem_var, elem_name, default_value, flags)
// INTERNAL_CPPMARKUP_ELEMENT_WITH_ATTR(elem_var, elem_name, default_value, flags, ...)

// > INTERNAL_CPPMARKUP_ENTITY_former(elem_var, ...)

#define INTERNAL_CPPMARKUP_ENTITY_former(elem_var, elem_name) \
    constexpr static auto _##elem_var##_TAG = elem_name;      \
    struct _##elem_var##_HASH_TYPE {}

#define INTERNAL_CPPMARKUP_ENTITY_former_ATTR(elem_var, elem_name, ...) \
    INTERNAL_CPPMARKUP_ENTITY_former(elem_var, elem_name);              \
    struct _##elem_var##_ATTRIBUTES {                                   \
        constexpr static auto _owner_element_TAG = elem_name;           \
        __VA_ARGS__;                                                    \
    } elem_var##_;                                                      \
    static size_t _##elem_var##_ATTR_OFFSET() { return offsetof(self_type, elem_var##_); }

#define INTENRAL_CPPMARKUP_ENTITY_fromer_NOATTR(elem_var, elem_name) \
    INTERNAL_CPPMARKUP_ENTITY_former(elem_var, elem_name);           \
    static size_t _##elem_var##_ATTR_OFFSET() { return 0; }

// >
// INTERNAL_CPPMARKUP_ATTRIBUTE(attrib_var, attrib_name, default_value)
#define INTERNAL_CPPMARKUP_ATTRIBUTE(attrib_var, attrib_name, default_value)                   \
    using _##attrib_var##_VALUE_TYPE = decltype(::kangsw::refl::etype::deduce(default_value)); \
    struct _##attrib_var##_HASH_TYPE {};                                                       \
    _##attrib_var##_VALUE_TYPE attrib_var;                                                     \
                                                                                               \
    static size_t _##attrib_var##_OFFSET() {                                                   \
        return (size_t)(&(((self_type*)nullptr)->elem_var_.attrib_var));                       \
    }                                                                                          \
                                                                                               \
    static inline ::kangsw::refl::attribute_registration_t<                                    \
        self_type, _##attrib_var##_VALUE_TYPE, _##attrib_var##_HASH_TYPE>                      \
        _##attrib_var##_REGISTER{                                                              \
            _owner_element_TAG,                                                                \
            attrib_name,                                                                       \
            _##attrib_var##_OFFSET(),                                                          \
            ::kangsw::refl::etype::deduce(default_value)};

// > INTERNAL_CPPMARKUP_ENTITY_latter(elem_var, elem_name, flags)
#define INTERNAL_CPPMAKRUP_ENTITY_latter(elem_var, flags)                          \
    constexpr static int _##elem_var##_FLAGS = flags;                              \
    _##elem_var##_VALUE_TYPE elem_var;                                             \
                                                                                   \
    static size_t _##elem_var##_OFFSET() { return offsetof(self_type, elem_var); } \
    static inline ::kangsw::refl::element_regestration_t<                          \
        self_type, _##elem_var##_VALUE_TYPE, _##elem_var##_HASH_TYPE>              \
        _##elem_var##_REGISTER {                                                   \
        _##elem_var##_TAG,                                                         \
            _##elem_var##_OFFSET(),                                                \
            _##elem_var##_ATTR_OFFSET(),                                           \
            ::kangsw::refl::etype::deduce(_##elem_var##_DEFAULT_VALUE()),          \
            _##elem_var##_FLAGS                                                    \
    }
// >

// Custom
#define INTERNAL_CPPMARKUP_ELEMENT_FULL(elem_var, elem_name, default_value, flags, ...)      \
    INTERNAL_CPPMARKUP_ENTITY_former_ATTR(elem_var, elem_name, ##__VA_ARGS__);               \
    using _##elem_var##_VALUE_TYPE = decltype(::kangsw::refl::etype::deduce(default_value)); \
                                                                                             \
    static inline const auto _##elem_var##_DEFAULT_VALUE = []() { return default_value; };   \
    INTERNAL_CPPMAKRUP_ENTITY_latter(elem_var, flags)

   
