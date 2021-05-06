#pragma once
#include "property.hpp"
#include "template_utils.hxx"

namespace kangsw::refl {

class object_traits_base {
public:
    auto& props() const { return _props; }

public:
    void add_property(property&& v) { _props.push_back(std::move(v)); }

public:
    auto begin() { return _props.begin(); }
    auto begin() const { return _props.begin(); }
    auto end() { return _props.end(); }
    auto end() const { return _props.end(); }

private:
    std::vector<property> _props;
};

template<typename Ty_>
using object_traits = templates::singleton<object_traits_base, Ty_>;

} // namespace kangsw::refl
