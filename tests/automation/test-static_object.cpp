#include <optional>
#include "doctest.h"
#include "kangsw/refl/static_object_base.hxx"

#define default_value "FAS"
#define elem_name     "hell, wrold!"
#define attrib_name   "someattrib"
#define elem_flags    0

// INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(object_type)
struct object_type : ::kangsw::refl::static_object_base<object_type> {
    // INTERNAL_CPPMARKUP_ELEMENT(elem_var, elem_name, default_value, flags)
    // INTERNAL_CPPMARKUP_ELEMENT_WITH_ATTR(elem_var, elem_name, default_value, flags, ...)
    constexpr static auto _elem_var_TAG = elem_name;
    constexpr static int _elem_var_FLAGS = elem_flags;

    // > INTERNAL_CPPMARKUP_ENTITY_former(elem_var, ...)
    struct _elem_var_HASH_TYPE {};
    struct _elem_var_ATTRIBUTES {
        // INTERNAL_CPPMARKUP_ATTRIBUTE(attrib_var, attrib_name, default_value)
        using _attrib_var_VALUE_TYPE = decltype(::kangsw::refl::etype::deduce(default_value));
        struct _attrib_var_HASH_TYPE {};
        _attrib_var_VALUE_TYPE attrib_var;

        static size_t _attrib_var_OFFSET() {
            return (size_t)(&(((self_type*)nullptr)->elem_var_.attrib_var));
        }

        static inline ::kangsw::refl::attribute_registration_t<
            self_type, _attrib_var_VALUE_TYPE, _attrib_var_HASH_TYPE>
            _attrib_var_REGISTER{
                _elem_var_TAG,
                attrib_name,
                _attrib_var_OFFSET(),
                ::kangsw::refl::etype::deduce(default_value)};

    } elem_var_;
    // >

    // Custom
    using _elem_var_VALUE_TYPE = decltype(::kangsw::refl::etype::deduce(default_value));

    static inline const _elem_var_VALUE_TYPE _elem_var_DEFAULT_VALUE = default_value;

    // > INTERNAL_CPPMARKUP_ENTITY_latter(elem_var, elem_name, flags)
    _elem_var_VALUE_TYPE elem_var;

    static size_t _elem_var_OFFSET() { return offsetof(object_type, elem_var); }
    static inline ::kangsw::refl::element_regestration_t<
        self_type, _elem_var_VALUE_TYPE, _elem_var_HASH_TYPE>
        _elem_var_REGISTER{
            _elem_var_TAG,
            _elem_var_OFFSET(),
            _elem_var_VALUE_TYPE(_elem_var_DEFAULT_VALUE),
            _elem_var_FLAGS};
    // >

    struct _embobj_var_VALUE_TYPE {
    };
};

TEST_SUITE("Static Object") {
    TEST_CASE("Prove of concept for prototype") {
        object_type o;

        o.elem_var_.attrib_var = "hell, world!";
        o.elem_var             = "Do we need to argue?";

        MESSAGE("Object size: ", sizeof o);

        auto pprop = o.traits().find_property(elem_name);
        REQUIRE(pprop);
        REQUIRE(pprop->memory().size == sizeof o.elem_var);
        REQUIRE(pprop->memory().offset == offsetof(object_type, elem_var));
        REQUIRE(pprop->memory().type == kangsw::refl::etype::string);

        REQUIRE(pprop->attributes().size() == 1);
        REQUIRE(pprop->attributes()[0].name == attrib_name);
        REQUIRE(pprop->attributes()[0].memory.size == sizeof o.elem_var_.attrib_var);
        REQUIRE(pprop->attributes()[0].memory.offset == sizeof nullptr);
    }
}