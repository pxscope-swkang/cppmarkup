#pragma once
#include "cppmarkup/impl_macros.hpp"

/*
-----------------------------------------------------------------------------------
 Usage
-----------------------------------------------------------------------------------

-----------------------------------------------------------------------------------
*/

/** */
#define CPPMARKUP_OBJECT_TEMPLATE(type_name) INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(type_name)

/** */
#define CPPMARKUP_ADD(tag, default_value, ... /*ATTRIBUTES*/) \
    INTERNAL_CPPMARKUP_ADD(tag, #tag, default_value, ##__VA_ARGS__)

/** */
#define CPPMARKUP_ADD_EX(varname, tag, default_value, ... /*ATTRIBUTES*/) \
    INTERNAL_CPPMARKUP_ADD(varname, tag, default_value, ##__VA_ARGS__)

/** */
#define CPPMARKUP_EMBED_OBJECT_begin(tag, ... /*ATTRIBUTES*/) \
    INTERNAL_CPPMARKUP_EMBED_OBJECT_begin(tag, #tag, ##__VA_ARGS__)
#define CPPMARKUP_EMBED_OBJECT_end(tag) INTERNAL_CPPMARKUP_EMBED_OBJECT_end(tag)

/** */
#define INTERNAL_CPPMARKUP_EMBED_OBJECT_EX_begin(varname, tag_name, ...) \
    INTERNAL_CPPMARKUP_EMBED_OBJECT_begin(varname, tag_name, ...)
#define CPPMARKUP_EMBED_OBJECT_EX_end(tag) INTERNAL_CPPMARKUP_EMBED_OBJECT_end(tag)

/** */
#define CPPMARKUP_WRAPPED_OBJECT_TEMPLATE(type_name, target, tag, ... /*ATTRIBUTES*/) \
    INTERNAL_CPPMARKUP_WRAPPED_OBJECT_TEMPLATE(type_name, target, tag, #tag, ##__VA_ARGS__)

/** */
#define CPPMARKUP_ATTRIBUTE(attr_name, default_value) \
    INTERNAL_CPPMARKUP_ATTRIBUTE(attr_name, #attr_name, default_value)

/** */
#define CPPMARKUP_ATTRIBUTE_EX(attr_varname, attr_name, default_value) \
    INTERNAL_CPPMARKUP_ATTRIBUTE(attr_varname, attr_name, default_value)

/** */
#define CPPMARKUP_DESCRIPTION_BELOW(u8description) INTERNAL_CPPMARKUP_DESCRIPTION(u8description)

/** initializer_list wrapper */
#define CPPMARKUP_ARRAY(...) std::initializer_list({__VA_ARGS__})

/** map wrapper */
#define CPPMARKUP_MAP(...) ::kangsw::markup::impl::deduce_map(__VA_ARGS__)