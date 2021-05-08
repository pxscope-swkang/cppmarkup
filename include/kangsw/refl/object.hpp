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

public:
    std::vector<property> const& properties() const { return traits().props(); }

public:
    // .find_subobject -> object*
    // .find -> property_proxy
    virtual object_traits_base const& traits() const = 0;

protected:
    /** Gets actual origin of contigous memory which is represented by properties. */
    virtual void* base()             = 0;
    virtual void const* base() const = 0;

public:
    auto operator[](property const& p) { return p.memory()(base()); }
    auto operator[](property const& p) const { return p.memory()(base()); }

    auto operator[](property::attribute const& p) { return p.memory(base()); }
    auto operator[](property::attribute const& p) const { return p.memory(base()); }
};

} // namespace kangsw::refl
