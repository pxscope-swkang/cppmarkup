#pragma once
#include "object_traits.hxx"

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

public:
    void reset() {
        for (auto& prop : traits().props()) {
            prop.memory().init_fn((*this)[prop]);
            for (auto& attr : prop.attributes()) {
                attr.memory.init_fn((*this)[attr]);
            }
        }
    }


protected:
    /** Gets actual origin of contigous memory which is represented by properties. */
    virtual void* base()             = 0;
    virtual void const* base() const = 0;

public:
    void* operator[](property const& p) { return p.memory()(base()); }
    void const* operator[](property const& p) const { return p.memory()(base()); }

    void* operator[](property::attribute const& p) { return p.memory(base(), p.offset()); }
    void const* operator[](property::attribute const& p) const { return p.memory(base(), p.offset()); }
};

} // namespace kangsw::refl
