#pragma once
#include <cassert>
#include "property.hxx"
#include "template_utils.hxx"

namespace kangsw::refl {

class object_traits_base {
public:
    auto& props() const { return _props; }
    auto begin() { return _props.begin(); }
    auto begin() const { return _props.begin(); }
    auto end() { return _props.end(); }
    auto end() const { return _props.end(); }

public:
    /** Add new or find existing property. */
    property& find_or_add_property(u8str_view tag) {
        // Since it is likely to refer to the property that has been added recently,
        //finds property from the last
        if (auto pprop = find_property(tag)) {
            return *pprop;
        }

        return _props.emplace_back(u8str(tag));
    }

    /** Finds existing property */
    property const* find_property(u8str_view tag) const {
        auto prop_it = std::find_if(
            _props.rbegin(), _props.rend(),
            [&tag](property const& v) { return v.tag() == tag; });

        if (prop_it == _props.rend()) {
            return nullptr;
        } else {
            return &*prop_it;
        }
    }

    property* find_property(u8str_view tag) {
        return const_cast<property*>(((object_traits_base const*)this)
                                         ->find_property(tag));
    }

    // .remove_property

private:
    std::vector<property> _props;
};

template <typename Ty_>
using object_traits = templates::singleton<object_traits_base, Ty_>;

} // namespace kangsw::refl
