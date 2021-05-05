#include "catch.hpp"
#include <cppmarkup.hpp>

CPPMARKUP_OBJECT_TEMPLATE(sample_type)
{
    CPPMARKUP_ELEMENT(SomeBoolValue, true);
    CPPMARKUP_ELEMENT(SomeNullValueWithAttr, nullptr,
                      CPPMARKUP_ATTRIBUTE(SomeAttr1, "Abcd");
                      CPPMARKUP_ATTRIBUTE(SomeAttr2, false);
                      CPPMARKUP_ATTRIBUTE(SomeAttr3, 315.241););

    CPPMARKUP_EMBED_OBJECT_begin(SomeEmbeddedObject,
                                 CPPMARKUP_ATTRIBUTE(SomeDateValue, 1534))
    {
        CPPMARKUP_ELEMENT(EmbeddedArray, CPPMARKUP_ARRAY(1, 2, 3, 4, 5));
    }
    CPPMARKUP_EMBED_OBJECT_end(SomeEmbeddedObject)
};

SCENARIO("Markup object can be marshaled from/to JSON", "[json]")
{
    GIVEN("An empty sample object reset as default initial value")
    {
        auto s = sample_type::get_default();

        THEN("Values should correctly be intialized")
        {
            REQUIRE(s.SomeBoolValue == true);

            REQUIRE(s.SomeNullValueWithAttr.SomeAttr1.value() == "Abcd");
            REQUIRE(s.SomeNullValueWithAttr.SomeAttr2 == false);
            REQUIRE(s.SomeNullValueWithAttr.SomeAttr3 == 315.241);

            REQUIRE(s.SomeEmbeddedObject.SomeDateValue == 1534);
            REQUIRE(s.SomeEmbeddedObject->EmbeddedArray.value() == std::vector<int64_t>{1, 2, 3, 4, 5});
        }

        WHEN("Embedded object is dumped into json")
        {
            using namespace kangsw::markup;
            u8string buff;
            dump(json_dump{buff, 4, 0}, s);

            INFO(buff);
            CHECK(false);

            THEN("")
            {
            }
        }
    }
}