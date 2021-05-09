#include "doctest.h"
#include "test_type.hxx"

using namespace kangsw;

struct visitor {
    template <typename Ty_>
    void operator()(refl::property_proxy<Ty_, false> pr) {
        MESSAGE(name, ": ", typeid(Ty_).name());
    }

    refl::u8str_view name;
};

TEST_SUITE("Types") {
    TEST_CASE("Property Proxy") {
        my_markup_type ss;

        for (auto& prop : ss.properties()) {
            refl::apply_property_op(ss, prop, visitor{prop.tag()});
            for (auto& attr : prop.attributes()) {
            refl::apply_property_op(ss, attr, visitor{attr.name});
            }
        }

        std::map<std::string, int, std::less<>> df;
        // df.find(std::string_view("fae"));
    }
}