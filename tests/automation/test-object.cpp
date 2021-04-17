#include "catch.hpp"
#include "cppmarkup/cppmarkup.hpp"


namespace test::cppmarkup::objects {
CPPMARKUP_OBJECT_TEMPLATE(some_type)
{
    CPPMARKUP_DESCRIPTION_BELOW(u8"");
    CPPMARKUP_ADD(fooa, u8"Fooa", 3.141, CPPMARKUP_ATTR(Attr1, u8"Hell, world!"));
    CPPMARKUP_ADD(grr, u8"EOFO", 2);

    CPPMARKUP_ADD2(AdminPw, 0x864321, CPPMARKUP_ATTR(Attr1, u8""); CPPMARKUP_ATTR(Attr2, u8""));

    CPPMARKUP_OBJECT_TEMPLATE(nested_type)
    {
        CPPMARKUP_DESCRIPTION_BELOW(u8"");
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
    r.AdminPw.Attr2 = u8"df";
}
} // namespace test::cppmarkup::objects