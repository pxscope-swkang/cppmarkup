#include "doctest.h"
#include "test_type.hxx"
#include "kangsw/markup/marshal/generics.hxx"

using namespace kangsw;

struct visitor {
    template <typename Ty_>
    void operator()(refl::property_proxy<Ty_, false> pr) {
        auto stringfy_trivial = [this](auto const& pv) {
            using type = std::remove_const_t<std::remove_reference_t<decltype(pv)>>;
            if constexpr (refl::generic_is_trivially_marshalable_v<type>) {
                std::string ostr1, ostr2;
                refl::generic_stringfy<type>{}(pv, std::back_inserter(ostr1));

                type v;
                refl::generic_parse<type>{}(ostr1.begin(), ostr1.end(), v);
                refl::generic_stringfy<type>{}(v, std::back_inserter(ostr2));

                MESSAGE(name, ": ", typeid(type).name(), " [", ostr1, "] -> [", ostr2, "]");
                REQUIRE(ostr1 == ostr2);
            }
        };

        if constexpr (refl::generic_is_trivially_marshalable_v<Ty_>) {
            stringfy_trivial(*pr);
        } else if constexpr (pr.type().is_array()) {
            for (int i = 0; i < pr.size(); ++i) {
                stringfy_trivial(pr[i]);
            }
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
            refl::visit_property(ss.base(), prop, visitor{prop.tag()});
            for (auto& attr : prop.attributes()) {
                refl::visit_property(ss.base(), attr, visitor{attr.name});
            }
        }

        // df.find(std::string_view("fae"));
    }
}