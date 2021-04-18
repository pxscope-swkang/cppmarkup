#pragma once
#include "cppmarkup/impl_macros.hpp"
#include "cppmarkup/marshal_defaults.hpp"

/*
-----------------------------------------------------------------------------------
 Usage
-----------------------------------------------------------------------------------
    XML:
        <Body>
            <Inner1 Attr1="AttrValue1" Attr2="2341">
                <IInner1 Attr1="IAttrValue">abcd eeeffg</IInner1>
                <IInner2>13<IInner2>
            <Inner1>
        </Body>
-----------------------------------------------------------------------------------
    Json: {
        "Body": {
            "Inner1": {
                "IInner1": "abcd eeeffg",
                "IInner1~@@ATTR@@": { "Attr1": "IAttrValue" },
                "IInner2": 13
            },
            "Inner1~@@ATTR@@": {
                "Attr1": "AttrValue1",
                "Attr2": 2341
            }
        }
    }
-----------------------------------------------------------------------------------
    CppMarkup:
        CPPMARKUP_OBJECT_TEMPLATE(body_type) {
            CPPMARKUP_OBJECT_GENERATE_BODY(body_type);

            CPPMARKUP_EMBED_OBJECT_begin(
                inner1, u8"Inner1",
                CPPMARKUP_ATTRIBUTE(attr1, u8"AttrValue1");
                CPPMARKUP_ATTRIBUTE(attr2, 2341));

                CPPMARKUP_ADD(
                    iinner1, u8"IInner1", u8"abcd eeeffg",
                    CPPMARKUP_ATTRIBUTE(attr1, u8"Attr1", u8"IAttrValue"));

                CPPMARKUP_ADD(iinner2, u8"IInner2", 2341);
                
            CPPMARKUP_EMBED_OBJECT_end(inner1);
        }

        // This macro will create <Body></Body> wrapper.
        CPPMARKUP_WRAPPED_OBJECT_TEMPLATE(body_type_wrapper, body_type, u8"Body");
-----------------------------------------------------------------------------------
*/

/** */
#define CPPMARKUP_OBJECT_TEMPLATE(type_name)      INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(type_name)
#define CPPMARKUP_OBJECT_GENERATE_BODY(type_name) INTERNAL_CPPMARKUP_OBJECT_TEMPLATE_BODY(type_name)

/** */
#define CPPMARKUP_ADD(tag, default_value, ... /*ATTRIBUTES*/) \
    INTERNAL_CPPMARKUP_ADD(tag, u8## #tag, default_value, ##__VA_ARGS__)

/** */
#define CPPMARKUP_ADD_EX(varname, tag, default_value, ... /*ATTRIBUTES*/) \
    INTERNAL_CPPMARKUP_ADD(varname, tag, default_value, ##__VA_ARGS__)

/** */
#define CPPMARKUP_EMBED_OBJECT_begin(tag, ... /*ATTRIBUTES*/) \
    INTERNAL_CPPMARKUP_EMBED_OBJECT_begin(tag, u8## #tag, ##__VA_ARGS__)
#define CPPMARKUP_EMBED_OBJECT_end(tag) INTERNAL_CPPMARKUP_EMBED_OBJECT_end(tag)

/** */
#define INTERNAL_CPPMARKUP_EMBED_OBJECT_EX_begin(varname, tag_name, ...) \
    INTERNAL_CPPMARKUP_EMBED_OBJECT_begin(varname, tag_name, ...)
#define CPPMARKUP_EMBED_OBJECT_EX_end(tag) INTERNAL_CPPMARKUP_EMBED_OBJECT_end(tag)

/** */
#define CPPMARKUP_WRAPPED_OBJECT_TEMPLATE(type_name, target, tag, ... /*ATTRIBUTES*/) \
    INTERNAL_CPPMARKUP_WRAPPED_OBJECT_TEMPLATE(type_name, target, tag, u8## #tag, ##__VA_ARGS__)

/** */
#define CPPMARKUP_ATTRIBUTE(attr_name, default_value) \
    INTERNAL_CPPMARKUP_ATTRIBUTE(attr_name, u8## #attr_name, default_value)

/** */
#define CPPMARKUP_ATTRIBUTE_EX(attr_varname, attr_name, default_value) \
    INTERNAL_CPPMARKUP_ATTRIBUTE(attr_varname, attr_name, default_value)

/** */
#define CPPMARKUP_DESCRIPTION_BELOW(u8description) INTERNAL_CPPMARKUP_DESCRIPTION(u8description)