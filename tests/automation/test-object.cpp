#include "catch.hpp"
#include <ezdata/object.hpp>

namespace test::ezdata::objects {
EZDATA_OBJECT_TEMPLATE(some_type)
{
    /*
    struct INTERNAL_EZ_INSTANCE_varname {
        using value_type                                 = decltype(TEMP_DEFAULTVAL);
        alignas(EZDATA_ALIGN) value_type _value = TEMP_DEFAULTVAL;

        static inline ezdata::impl::object_inst_init<INTERNAL_EZ_INSTANCE_varname, value_type, EZDATA_ALIGN> _init{
            INTERNAL_EZ_node_list, u8"TagName", sizeof _value, &INTERNAL_EZ_description_str,
            [](void *r, size_t size, pugi::xml_node const &s) -> bool {
                assert(sizeof(value_type) == size);
                return ezdata::marshal::parse(*(value_type *)r, s);
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
            static inline ezdata::impl::object_inst_attr_init<INTERNAL_EZ_ATTR_Attr1, EZDATA_ALIGN>
                _init{
                    INTERNAL_EZ_node_list, u8"Attr1", u8"Attr1Default"};
        };
    } d;
    */

    // EZDATA_DESCRIPTION_BELOW(u8"");
    EZDATA_ADD(fooa, u8"Fooa", 3.141, EZDATA_ATTR(Attr1, u8"Hell, world!"));
    EZDATA_ADD(grr, u8"EOFO", 2);

    EZDATA_OBJECT_TEMPLATE(nested_type)
    {
        // EZDATA_DESCRIPTION_BELOW(u8"");
        EZDATA_ADD(fooale, u8"Fooa", 3.141, EZDATA_ATTR(Attr1, u8"Hell, world!"));
    };

    EZDATA_ADD(poop, u8"Poop", nested_type{});

    EZDATA_NESTED_OBJECT(
        lod, u8"Lod",
        EZDATA_ADD(laa, u8"Laa", 34));
};

TEST_CASE("instanciation")
{
    some_type r;
    std::atomic<void*> k;
}
} // namespace test::ezdata::objects