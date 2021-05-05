#include "catch.hpp"
#include "cppmarkup/base64.hxx"

namespace test::kangsw::base64 {
namespace b64 = ::kangsw::base64;

constexpr std::string_view mystring = (char*)"Hello, world! al dl ell lorem ipsum æ»≥Á«œººø‰f";

TEST_CASE("base64")
{
    std::string base64str;
    base64str.reserve(b64::encoded_size(mystring.length()));
    b64::encode(mystring.data(), mystring.size() * sizeof mystring[0], std::back_inserter(base64str));

    printf("length: %lld \n", base64str.size());
    puts(base64str.c_str());

    std::string sourcestr;
    auto result = b64::decode(base64str.begin(), base64str.end(), std::back_inserter(sourcestr));
    printf(sourcestr.data());

    REQUIRE(result);
    REQUIRE(sourcestr == mystring);
}

} // namespace test::kangsw::base64
