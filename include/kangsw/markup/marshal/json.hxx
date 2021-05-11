#pragma once
#include "../reflection/property_proxy.hxx"
#include "generics.hxx"

namespace kangsw::refl::marshal {

/** Parses input sequence based on json parsing rule ... */
class json {

    /** Json dump can be done statelessly. */
    template <typename OutIt_>
    static void dump(object& obj, OutIt_ o);
};

struct _impl {
};

template <typename OutIt_>
void json::dump(object& obj, OutIt_ o) {
}
} // namespace kangsw::refl::marshal