#include "catch.hpp"
#include "cppmarkup.hpp"
#include "test-common-type.hpp"

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
            dump(json_dump{buff}, s);

            THEN("Create another empty structure")
            {
                AND_THEN("Parsing result should be equal with original data")
                {
                    FAIL("TEST_NOT_IMPLEMENTED");
                }
            }
        }
    }
}

TEST_CASE("JSON Dump test", "[.]")
{
    using namespace kangsw;
    markup::u8string buff;
    markup::dump(markup::json_dump{buff, 4, 0}, sample_type::get_default());
    WARN(buff);
}