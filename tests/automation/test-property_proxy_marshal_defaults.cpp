#include "doctest.h"
#include "test_type.hxx"
#include "kangsw/markup/marshal_default.hxx"

using namespace kangsw;

struct visitor {
    template <typename Ty_>
    void operator()(refl::property_proxy<Ty_, false> pr) {
        if constexpr (refl::generic_can_trivially_marshalable_v<Ty_>) {
            std::string o;
            refl::generic_stringfy<Ty_>{}(*pr, std::back_inserter(o));

            MESSAGE(name, ": ", typeid(Ty_).name(), " [", o, "] ");
        } else {
            MESSAGE(name, ": ", typeid(Ty_).name());
        }
    }

    refl::u8str_view name;
};

TEST_SUITE("Types") {
    TEST_CASE("Property Proxy") {
        my_markup_type ss = my_markup_type::get_default();
        std::vector<int> cr;
        ss.some_obj_arr_.encrypt.write(cr.begin(), cr.end());

        for (auto& prop : ss.properties()) {
            refl::apply_property_op(ss, prop, visitor{prop.tag()});
            for (auto& attr : prop.attributes()) {
                refl::apply_property_op(ss, attr, visitor{attr.name});
            }
        }

        // df.find(std::string_view("fae"));
    }
}