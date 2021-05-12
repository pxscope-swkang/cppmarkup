#pragma once
#include "object_traits.hxx"

namespace kangsw::refl {

/** TODO: A property proxy class that wraps memory address as type-safe property instance. */

/**
 * 
 */
class object {
public:
    virtual ~object() noexcept = default;

public:
    std::vector<property> const& properties() const { return traits().props(); }

public:
    // .find_subobject -> object*
    // .find -> property_proxy
    virtual object_traits const& traits() const = 0;

    object_baseaddr_t* base() { return static_cast<object_baseaddr_t*>(_base()); }
    object_baseaddr_t const* base() const { return static_cast<object_baseaddr_t const*>(_base()); }

protected:
    /** Gets actual origin of contigous memory which is represented by properties. */
    virtual void* _base()             = 0;
    virtual void const* _base() const = 0;

public:
    void reset();

public:
    void* operator[](property const& p) { return p.memory()(_base()); }
    void const* operator[](property const& p) const { return p.memory()(_base()); }

    void* operator[](property::attribute const& p) { return p._memory(_base()); }
    void const* operator[](property::attribute const& p) const { return p._memory(_base()); }
};

inline void object::reset() {
    for (auto& prop : traits().props()) {
        prop.memory().init_fn((*this)[prop]);
        for (auto& attr : prop.attributes()) {
            attr._memory.init_fn((*this)[attr]);
        }
    }
}

} // namespace kangsw::refl
