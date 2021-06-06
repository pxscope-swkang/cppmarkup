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
        CPPMARKUP_ELEMENT(v1_int, 1);
        CPPMARKUP_ELEMENT(v2_int_array, std::vector({1, 3}));

        CPPMARKUP_EMBED_OBJECT_begin(v3_inner_obj) //
        {
            CPPMARKUP_ELEMENT(v4_boolean_vector, std::vector({false, false, true, false}));
            CPPMARKUP_ELEMENT(v5_timestamp, std::chrono::system_clock::now());
        }
        CPPMARKUP_EMBED_OBJECT_end(v3_inner_obj);

        CPPMARKUP_ELEMENT(v6_some_float, 2.0);
        CPPMARKUP_ELEMENT(v7_some_binary, kangsw::refl::binary_chunk::from(3, 4, 1));
    };

    TEST_CASE("Parse") {
        marshal::json_parse parse;
        parsetest test;
        test.reset();

        refl::u8str str;
        marshal::json_dump{}(test, {str});

        SUBCASE("Stringfy JSMN") {
            jsmn::jsmn_parser p;
            jsmn::jsmn_init(&p);

            jsmn::jsmntok_t tokens[128];
            auto n_token = jsmn::jsmn_parse(&p, str.c_str(), str.size(), tokens, (unsigned)std::size(tokens));

            for (int i = 0; i < n_token; ++i) {
                auto const& token             = tokens[i];
                constexpr char const* names[] = {"Undefined", "Object", "Array", "String", "Primitive"};

                printf("[%03d] (%-10s) ", i, names[token.type]);
                for (auto node = tokens + i; node->parent != -1; node = tokens + node->parent) {
                    putchar('\t');
                }

                printf("%.*s\n", token.end - token.start, str.c_str() + token.start);
            }
        }

        SUBCASE("Parse dumped original object into memory") {
            parsetest dest = {};
            auto report    = marshal::json_parse{true, 128}(str, dest);

            if(report)
            {
                MESSAGE(report->code);
                MESSAGE(report->where);
            }
            REQUIRE(report.has_value() == false);

            CHECK(dest.v1_int == test.v1_int);
            CHECK(dest.v2_int_array == test.v2_int_array);
            CHECK(dest.v3_inner_obj.v4_boolean_vector == test.v3_inner_obj.v4_boolean_vector);
            CHECK(
              dest.v3_inner_obj.v5_timestamp.time_since_epoch().count() / 1'000'000 == test.v3_inner_obj.v5_timestamp.time_since_epoch().count() / 1'000'000);
            CHECK(dest.v6_some_float == test.v6_some_float);
            CHECK(dest.v7_some_binary == test.v7_some_binary);
        }
    }
}