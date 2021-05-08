#include "doctest.h"
#include "kangsw/refl/property.hxx"

using namespace kangsw::refl;

// Types compilation test
static_assert(etype::from_type<int>().exact_type() == etype::integer);
static_assert(etype::from_type<bool>().exact_type() == etype::boolean);
static_assert(etype::from_type<double>().exact_type() == etype::floating_point);
static_assert(etype::from_type<char const*>().exact_type() == etype::string);
static_assert(etype::from_type<std::vector<double>>().exact_type() == etype::floating_point);
static_assert(etype::from_type<std::vector<double>>().is_array());
static_assert(etype::from_type<std::map<u8str, double>>().is_map());

static_assert(std::is_same_v<decltype(etype::deduce(324)), int64_t>);
static_assert(std::is_same_v<decltype(etype::deduce(324.23f)), double>);
static_assert(std::is_same_v<decltype(etype::deduce("hell, world!")), u8str>);
static_assert(std::is_same_v<decltype(etype::deduce(std::vector<int>{})), std::vector<int64_t>>);
static_assert(std::is_same_v<decltype(etype::deduce({1, 2, 4})), std::vector<int64_t>>);
static_assert(std::is_same_v<decltype(etype::deduce(nullptr)), nullptr_t>);
static_assert(std::is_same_v<decltype(etype::deduce(false)), boolean_t>);

namespace tests::types {
TEST_SUITE("Types")
{
    TEST_CASE("Compilation")
    {
        static_assert(kangsw::templates::is_specialization_of<
                          binary_chunk, std::vector>::value == false);
    }
}
} // namespace tests::types