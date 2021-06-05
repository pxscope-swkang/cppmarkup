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

    CPPMARKUP_OBJECT_TEMPLATE(parsetest) {
        CPPMARKUP_ELEMENT(abcd, 0);
        CPPMARKUP_ELEMENT(alph, std::vector({1, 3}));

        CPPMARKUP_EMBED_OBJECT_begin(CFV) //
        {
            CPPMARKUP_ELEMENT(AAD, std::vector<kangsw::refl::boolean_t>());
        }
        CPPMARKUP_EMBED_OBJECT_end(CFV);

        CPPMARKUP_ELEMENT(xc, 0);
        CPPMARKUP_ELEMENT(foov, true);
    };

    TEST_CASE("Parse") {
        marshal::json_parse parse;
        parsetest test;
        test.reset();

        parse(R"({"abcd":"12345", "alph": [1,2,3,4], "CFV": { "AAD": [false, true, true] }, "xc": 1312.413, "foov" : false})",
              test);
    }
}