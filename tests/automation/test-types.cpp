#include "catch.hpp"
#include "cppmarkup/object_container_proxy.hxx"

using namespace ::kangsw;

namespace test::kangsw {

TEST_CASE("object array proxy compilation", "[.]")
{
    markup::property prop;
    auto objprx_const = markup::make_object_array_proxy(prop.as_array(), (void const*)0);
    objprx_const.for_each([](markup::object const& r) { r.structure_hash(); });
}

} // namespace test::kangsw