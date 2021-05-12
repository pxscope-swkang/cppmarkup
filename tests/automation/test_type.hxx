#pragma once
#include "kangsw/markup.hxx"

CPPMARKUP_OBJECT_TEMPLATE(my_markup_type) {
    CPPMARKUP_ELEMENT(is_valid, false);

    CPPMARKUP_OBJECT_TEMPLATE(internal_object_type) {
        CPPMARKUP_ELEMENT_A(single_elem, nullptr,
                            CPPMARKUP_ATTRIBUTE(creation, kangsw::refl::timestamp_t::clock::now());
                            CPPMARKUP_ATTRIBUTE(ref_path, "/doc/args"));
    };

    CPPMARKUP_ELEMENT(list_author, std::vector({"abc", "Def"}));
    CPPMARKUP_ELEMENT_A(some_obj_arr, std::vector({internal_object_type::get_default()}),
                        CPPMARKUP_ATTRIBUTE(encrypt, kangsw::refl::binary_chunk::from("hello, world!", 32, 42));
                        CPPMARKUP_ATTRIBUTE(stamp, kangsw::refl::timestamp_t::clock::now()));
    CPPMARKUP_ELEMENT_A(some_obj_map, CPPMARKUP_MAP(u8"entity", internal_object_type::get_default()));
    CPPMARKUP_ELEMENT(rev_major, 0);
    CPPMARKUP_ELEMENT(rev_minor, 31);
    CPPMARKUP_ELEMENT(rev_minor2, 315);
    CPPMARKUP_ELEMENT(some_dbl, 315.411);
    CPPMARKUP_ELEMENT(version_vector, std::vector({1, 0, 141, 5}));
};