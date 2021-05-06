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
    // .props
};

/** TODO: Separate static object into different header, which is only required by macros */
template <typename Ty_>
class static_object_base : public object {
public:
    using traits = object_traits<Ty_>;
};

/** TODO: A object class which can dynamically edit properties on runtime. */
class dynamic_object : public object {};

} // namespace kangsw::refl
