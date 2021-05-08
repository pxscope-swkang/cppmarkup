#pragma once
#include "object.hxx"

namespace kangsw::refl {

class property_type_mismatch_exception : public std::logic_error {
    using std::logic_error::logic_error;
};

// Supports non-const, const version both.
template <typename Ty_, bool Constant_>
class property_proxy {
public:
    using void_pointer = std::conditional_t<Constant_, void const*, void*>;
    using value_type   = std::conditional_t<Constant_, Ty_ const, Ty_>;
    using pointer      = value_type*;
    using reference    = value_type&;

public:
    property_proxy(property::memory_t const& m, void_pointer ptr)
        : _p(static_cast<pointer>(ptr)) {
        if (m.type != etype::from_type_exact<Ty_>()) {
            throw property_type_mismatch_exception{"Type mismatch"};
        }
    }

    reference operator*() const { return *_p; }
    pointer operator->() const { return _p; }

private:
    pointer _p;
};

template <typename ValueTy_, bool Constant_>
class property_proxy<std::vector<ValueTy_>, Constant_> {};

template <typename ValueTy_, bool Constant_>
class property_proxy<u8str_map<ValueTy_>, Constant_> {};

template <bool Constant_>
class property_proxy<std::vector<object>, Constant_> {};

template <bool Constant_>
class property_proxy<u8str_map<object>, Constant_> {};

template <typename Ty_>
auto make_proxy(object const* obj, property::memory_t const& m)
    -> property_proxy<Ty_, true> { return {m, m(obj->base())}; }

template <typename Ty_>
auto make_proxy(object* obj, property::memory_t const& m)
    -> property_proxy<Ty_, false> { return {m, m(obj->base())}; }

} // namespace kangsw::refl

void pewpew() {
    using namespace kangsw::refl;
    auto r = make_proxy<object>((object const*)nullptr, {});
}