#include "doctest.h"
#include "kangsw/markup/marshal/json.hxx"
#include "test_type.hxx"
#include <conio.h>

namespace refl    = kangsw::refl;
namespace marshal = refl::marshal;

TEST_SUITE("Marshal.Json") {
    TEST_CASE("Dump Visualize") {
        my_markup_type mk;
        mk.reset();

        refl::u8str s;
        marshal::json_dump{}(mk, {s, 4});

        MESSAGE(s);
    }

    TEST_CASE("minify ... visualize") {
        auto mk = my_markup_type::get_default();

        refl::u8str s;
        s.reserve(2048);
        marshal::json_dump{}(mk, {s, 4});

        refl::u8str js;
        js.reserve(s.size());

        marshal::json_object_from_stream fence{js};
        for (auto it = s.begin(), end = s.end(); it != end; ++it) {
            auto ch = *it;

            // _getch();
            // printf("\r%s", minified.substr(std::max(0ll, (ptrdiff_t)minified.size() - 42)).c_str());

            if (fence(ch) != marshal::json_object_from_stream::ready) {
                break;
            }
        }

        MESSAGE(js);
    }
}