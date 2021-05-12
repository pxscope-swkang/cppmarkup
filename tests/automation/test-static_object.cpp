#include <optional>
#include "doctest.h"
#include "kangsw/markup/reflection/static_object_base.hxx"
#include "kangsw/markup/macros.hxx"

#define default_value "FAS"
#define elem_name     "hell, wrold!"
#define attrib_name   "someattrib"
#define elem_flags    0

// INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(object_type)
struct object_type : ::kangsw::refl::static_object_base<object_type> {
    // INTERNAL_CPPMARKUP_ELEMENT(elem_var, elem_name, default_value, flags)
    // INTERNAL_CPPMARKUP_ELEMENT_WITH_ATTR(elem_var, elem_name, default_value, flags, ...)
    constexpr static auto _elem_var_TAG  = elem_name;
    constexpr static int _elem_var_FLAGS = elem_flags;

    // > INTERNAL_CPPMARKUP_ENTITY_former(elem_var, ...)
    struct _elem_var_HASH_TYPE {};
    struct _elem_var_ATTRIBUTES {
        using _attributes_SELF_TYPE = _elem_var_ATTRIBUTES;
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
    static size_t _elem_var_ATTR_OFFSET() { return offsetof(self_type, elem_var_); }
    // static size_t _elem_var_ATTR_OFFSET()

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

TEST_SUITE("Types.Static Object") {
    TEST_CASE("Prove of concept for prototype") {
        object_type o;

        o.elem_var_.attrib_var = "hell, world!";
        o.elem_var             = "Do we need to argue?";

        MESSAGE("Object size: ", sizeof o);

        auto pprop = o.traits().find_property(elem_name);
        REQUIRE(pprop);
        CHECK(pprop->memory().size == sizeof o.elem_var);
        CHECK(pprop->memory().offset == offsetof(object_type, elem_var));
        CHECK(pprop->memory().type == kangsw::refl::etype::string);

        REQUIRE(pprop->attributes().size() == 1);
        CHECK(pprop->attributes()[0].name == attrib_name);
        CHECK(pprop->attributes()[0]._memory.size == sizeof o.elem_var_.attrib_var);
        CHECK(pprop->attributes()[0]._memory.offset == sizeof nullptr);
    }

    INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(testobj) {
        INTERNAL_CPPMARKUP_ELEMENT_ATTR(testvarf, "SomeTestVar1", 15.42, 0);
        INTERNAL_CPPMARKUP_ELEMENT_ATTR(testvari, "SomeTestVar2", 154, 0);
        INTERNAL_CPPMARKUP_ELEMENT_ATTR(testvars, "SomeTestVar3", "hell, world!", 0);
        INTERNAL_CPPMARKUP_ELEMENT_ATTR(testvaria, "SomeTestVar4", std::vector({1, 2, 3}), 0);
    };

    TEST_CASE("Internal macro functionality test") {
        auto v = kangsw::refl::etype::deduce("faer");

        testobj k;
        static_assert(std::is_same_v<decltype(k.testvarf), double>);
        static_assert(std::is_same_v<decltype(k.testvari), int64_t>);
        static_assert(std::is_same_v<decltype(k.testvars), kangsw::refl::u8str>);
        static_assert(std::is_same_v<decltype(k.testvaria), std::vector<int64_t>>);

        k.reset();
        CHECK(k.testvarf == 15.42);
        CHECK(k.testvari == 154);
        CHECK(k.testvars == "hell, world!");
        CHECK(k.testvaria == std::vector<int64_t>({1, 2, 3}));

        k = {};
        CHECK(k.testvarf == 0);
        CHECK(k.testvari == 0);
        CHECK(k.testvars == "");
        CHECK(k.testvaria == std::vector<int64_t>());

        k = testobj::get_default();
        CHECK(k.testvarf == 15.42);
        CHECK(k.testvari == 154);
        CHECK(k.testvars == "hell, world!");
        CHECK(k.testvaria == std::vector<int64_t>({1, 2, 3}));
    }

    INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(attrtestobj) {
        INTERNAL_CPPMARKUP_ELEMENT_ATTR(
          attrvarf, "SomeTestVar1", nullptr, 0,
          INTERNAL_CPPMARKUP_ATTRIBUTE(attr1, "Attr1", 154);
          INTERNAL_CPPMARKUP_ATTRIBUTE(attr2, "Attr2", "galer");
          INTERNAL_CPPMARKUP_ATTRIBUTE(attr3, "Attr3", true) //
        );

        INTERNAL_CPPMARKUP_ELEMENT_ATTR(
          attrvarfd, "SomeTestVar2", nullptr, 0,
          INTERNAL_CPPMARKUP_ATTRIBUTE(attr1, "Attr1b", 211);
          INTERNAL_CPPMARKUP_ATTRIBUTE(attr2, "Attr2b", "pewpew");
          INTERNAL_CPPMARKUP_ATTRIBUTE(attr3, "Attr3b", false); //
        );

        INTERNAL_CPPMARKUP_ELEMENT_NOATTR(testvar, "Teststvar", 154, 0);

        INTERNAL_CPPMARKUP_EMBED_OBJECT_begin_ATTR(embobj, "embobj", 0) //
        {
            INTERNAL_CPPMARKUP_ELEMENT_NOATTR(
              testmap, "TestMap",
              INTERNAL_CPPMARKUP_MAP("hell", 3.14, "world", 2.54),
              0);

            INTERNAL_CPPMARKUP_ELEMENT_NOATTR(
              testobjarr, "TestObjArr",
              std::vector({testobj{}}), 0);

            INTERNAL_CPPMARKUP_ELEMENT_NOATTR(
              testobjmap, "TestObjMap",
              INTERNAL_CPPMARKUP_MAP("hell", testobj{}),
              0);
        }
        INTERNAL_CPPMARKUP_EMBED_OBJECT_end(embobj);

        INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(embedded) {
            INTERNAL_CPPMARKUP_ELEMENT_ATTR(
              attrvarf, "SomeTestVar1", nullptr, 0,
              INTERNAL_CPPMARKUP_ATTRIBUTE(attr1, "Attr1", 154);
              INTERNAL_CPPMARKUP_ATTRIBUTE(attr2, "Attr2", "galer");
              INTERNAL_CPPMARKUP_ATTRIBUTE(attr3, "Attr3", true) //
            );

            INTERNAL_CPPMARKUP_ELEMENT_ATTR(
              attrvarfd, "SomeTestVar2", nullptr, 0,
              INTERNAL_CPPMARKUP_ATTRIBUTE(attr1, "Attr1b", 211);
              INTERNAL_CPPMARKUP_ATTRIBUTE(attr2, "Attr2b", "pewpew");
              INTERNAL_CPPMARKUP_ATTRIBUTE(attr3, "Attr3b", false); //
            );

            INTERNAL_CPPMARKUP_ELEMENT_NOATTR(testvar, "Teststvar", 154, 0);
        };

        INTERNAL_CPPMARKUP_ELEMENT_NOATTR(testobj, "TestObj", embedded::get_default(), 0);
    };

    TEST_CASE("Internal macro with attributed elements functionality test") {
        attrtestobj tt;

        MESSAGE("Size of attrtestobj: ", sizeof tt);
        tt = attrtestobj ::get_default();

        CHECK(tt.attrvarf_.attr1 == 154);
        CHECK(tt.attrvarf_.attr2 == "galer");
        CHECK(tt.attrvarf_.attr3 == true);

        CHECK(tt.attrvarfd_.attr1 == 211);
        CHECK(tt.attrvarfd_.attr2 == "pewpew");
        CHECK(tt.attrvarfd_.attr3 == false);

        CHECK(tt.testvar == 154);

        CHECK(tt.testobj.attrvarf_.attr1 == 154);
        CHECK(tt.testobj.attrvarf_.attr2 == "galer");
        CHECK(tt.testobj.attrvarf_.attr3 == true);

        CHECK(tt.testobj.attrvarfd_.attr1 == 211);
        CHECK(tt.testobj.attrvarfd_.attr2 == "pewpew");
        CHECK(tt.testobj.attrvarfd_.attr3 == false);

        CHECK(tt.testobj.testvar == 154);

        tt.embobj.testobjarr;

        REQUIRE(tt.traits().find_property("SomeTestVar1"));
        REQUIRE(tt.traits().find_property("SomeTestVar1")->tag() == "SomeTestVar1");
        REQUIRE(tt.traits().find_property("SomeTestVar2"));
        REQUIRE(tt.traits().find_property("SomeTestVar2")->tag() == "SomeTestVar2");
        REQUIRE(tt.traits().find_property("Teststvar"));
        REQUIRE(tt.traits().find_property("Teststvar")->tag() == "Teststvar");

        auto map = INTERNAL_CPPMARKUP_MAP("abc", 3, "def", 4);
    }

    // new macro test
    CPPMARKUP_OBJECT_TEMPLATE(newmacrotest) {
        CPPMARKUP_ELEMENT(SomeBoolean, true);
        CPPMARKUP_ELEMENT(SomeInt, 134);
        CPPMARKUP_ELEMENT(SomeFloat, 134.341f);
        CPPMARKUP_ELEMENT_A(
          SomeNullWithAttr, nullptr,
          CPPMARKUP_ATTRIBUTE(At1, 100);
          CPPMARKUP_ATTRIBUTE(At2, false);
          CPPMARKUP_ATTRIBUTE(At3, "SomeStr"););

        CPPMARKUP_EMBED_OBJECT_begin(SomeEmbed) //
        {
            CPPMARKUP_ELEMENT(SomeBoolean, false);
            CPPMARKUP_ELEMENT(SomeInt, 1345);
            CPPMARKUP_ELEMENT(SomeFloat, 134.561f);
            CPPMARKUP_ELEMENT_A(
              SomeNullWithAttr, nullptr,
              CPPMARKUP_ATTRIBUTE(At1, 101);
              CPPMARKUP_ATTRIBUTE(At2, true);
              CPPMARKUP_ATTRIBUTE(At3, "SomeStr2"););
        }
        CPPMARKUP_EMBED_OBJECT_end(SomeEmbed);

        CPPMARKUP_EMBED_OBJECT_A_begin(SomeEmbed2,
                                       CPPMARKUP_ATTRIBUTE(At1, 141);
                                       CPPMARKUP_ATTRIBUTE(At2, "abcd");
                                       CPPMARKUP_ATTRIBUTE(At3, 331.411);) //
        {
            CPPMARKUP_ELEMENT(SomeInt, 511);
            CPPMARKUP_ELEMENT(SomeArr, std::vector({1.4, 1.5, 3.11}));
        }
        CPPMARKUP_EMBED_OBJECT_end(SomeEmbed2);
    };

    TEST_CASE("API macros test") {
        auto f = newmacrotest::get_default();

        REQUIRE(f.SomeEmbed.SomeNullWithAttr_.At3 == "SomeStr2");
        REQUIRE(f.SomeEmbed2.SomeArr == std::vector({1.4, 1.5, 3.11}));

        static_assert(std::is_move_assignable_v<newmacrotest>);
        static_assert(std::is_move_constructible_v<newmacrotest>);
    }
}