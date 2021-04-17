#include "ezdata/object.hpp"

static inline struct f {
    f(int a, int b);
} d{1, 2};

namespace {

// I_EZDATA_OBJECT_TEMPLATE_FULL(foo, {{u8"a", u8"b"}}, u8"hell, world!");


void compile_test()
{

    std::u8string_view d;
}

} // namespace
