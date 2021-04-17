#include "catch.hpp"
#include "cppmarkup/object.hpp"

namespace test::cppmarkup::objects {
CPPMARKUP_OBJECT_TEMPLATE(some_type)
{
    /*
    struct INTERNAL_EZ_INSTANCE_varname {
        using value_type                                 = decltype(TEMP_DEFAULTVAL);
        alignas(CPPMARKUP_ALIGN) value_type _value = TEMP_DEFAULTVAL;

        static inline cppmarkup::impl::object_inst_init<INTERNAL_EZ_INSTANCE_varname, value_type, CPPMARKUP_ALIGN> _init{
            INTERNAL_EZ_node_list, u8"TagName", sizeof _value, &INTERNAL_EZ_description_str,
            [](void *r, size_t size, pugi::xml_node const &s) -> bool {
                assert(sizeof(value_type) == size);
                return cppmarkup::marshal::parse(*(value_type *)r, s);
            }};

        operator value_type() const
        {
            return _value;
        }

        INTERNAL_EZ_INSTANCE_varname(const INTERNAL_EZ_INSTANCE_varname &r) = default;
        INTERNAL_EZ_INSTANCE_varname(INTERNAL_EZ_INSTANCE_varname &&r)      = default;
        INTERNAL_EZ_INSTANCE_varname(value_type const &r = value_type{}) : _value(r) {}
        INTERNAL_EZ_INSTANCE_varname(value_type &&r) : _value(std::move(r)) {}
        INTERNAL_EZ_INSTANCE_varname &operator=(const INTERNAL_EZ_INSTANCE_varname &r) = default;
        INTERNAL_EZ_INSTANCE_varname &operator=(INTERNAL_EZ_INSTANCE_varname &&r) = default;

        INTERNAL_EZ_INSTANCE_varname &operator=(value_type const &r) { return _value = r, *this; }
        INTERNAL_EZ_INSTANCE_varname &operator=(value_type &&r) { return _value = std::move(r), *this; }

        // attribute
        std::u8string Attr1;
        struct INTERNAL_EZ_ATTR_Attr1 {
            static inline cppmarkup::impl::object_inst_attr_init<INTERNAL_EZ_ATTR_Attr1, CPPMARKUP_ALIGN>
                _init{
                    INTERNAL_EZ_node_list, u8"Attr1", u8"Attr1Default"};
        };
    } d;
    */

    // CPPMARKUP_DESCRIPTION_BELOW(u8"");
    CPPMARKUP_ADD(fooa, u8"Fooa", 3.141, CPPMARKUP_ATTR(Attr1, u8"Hell, world!"));
    CPPMARKUP_ADD(grr, u8"EOFO", 2);

    CPPMARKUP_OBJECT_TEMPLATE(nested_type)
    {
        // CPPMARKUP_DESCRIPTION_BELOW(u8"");
        CPPMARKUP_ADD(fooale, u8"Fooa", 3.141, CPPMARKUP_ATTR(Attr1, u8"Hell, world!"));
    };

    CPPMARKUP_ADD(poop, u8"Poop", nested_type{});

    CPPMARKUP_NESTED_OBJECT(
        lod, u8"Lod",
        CPPMARKUP_ADD(laa, u8"Laa", 34));
};

TEST_CASE("instanciation")
{
    some_type r;
    std::atomic<void*> k;
}
} // namespace test::cppmarkup::objects