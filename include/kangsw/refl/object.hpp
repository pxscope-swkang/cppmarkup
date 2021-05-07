#pragma once
#include "object_traits.hpp"

namespace kangsw::refl {

/** TODO: A property proxy class that wraps memory address as type-safe property instance. */

/**
 * 
 */
class object {
public:
    virtual ~object() = default;

    // .find_subobject -> object*
    // .find -> property_proxy
    virtual object_traits_base const& traits() = 0;
};

} // namespace kangsw::refl
