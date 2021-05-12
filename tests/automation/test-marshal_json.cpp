#include "doctest.h"
#include "kangsw/markup/marshal/json.hxx"
#include "test_type.hxx"

namespace refl    = kangsw::refl;
namespace marshal = refl::marshal;

TEST_SUITE("Marshal.Json") {
    TEST_CASE("Dump Visualize") {
        my_markup_type mk;
        mk.reset();

        refl::u8str s;
        marshal::string_output os{s, 4};
        marshal::json_dump{}(mk, os);

        MESSAGE(os.out);
    }
}