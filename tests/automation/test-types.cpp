#include "doctest.h"
#include "kangsw/markup/reflection/property.hxx"

using namespace kangsw::refl;

// Types compilation test
static_assert(etype::from_type<int>().leap() == etype::integer);
static_assert(etype::from_type<bool>().leap() == etype::boolean);
static_assert(etype::from_type<double>().leap() == etype::floating_point);
static_assert(etype::from_type<char const*>().leap() == etype::string);
static_assert(etype::from_type<std::vector<double>>().leap() == etype::floating_point);
static_assert(etype::from_type<std::vector<double>>().is_array());
static_assert(etype::from_type<u8str_map<double>>().is_map());

static_assert(std::is_same_v<decltype(etype::deduce(324)), int64_t>);
static_assert(std::is_same_v<decltype(etype::deduce(324.23f)), double>);
static_assert(std::is_same_v<decltype(etype::deduce("hell, world!")), u8str>);
static_assert(std::is_same_v<decltype(etype::deduce(std::vector<int>{})), std::vector<int64_t>>);
static_assert(std::is_same_v<decltype(etype::deduce({1, 2, 4})), std::vector<int64_t>>);
static_assert(std::is_same_v<decltype(etype::deduce(nullptr)), nullptr_t>);
static_assert(std::is_same_v<decltype(etype::deduce(false)), boolean_t>);

namespace tests::types {
TEST_SUITE("Types") {
    TEST_CASE("Compilation") {
        static_assert(kangsw::templates::is_specialization_of<
                        binary_chunk, std::vector>::value == false);
    }
}
} // namespace tests::types