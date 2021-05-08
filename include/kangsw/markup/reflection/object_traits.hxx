#pragma once
#include <algorithm>
#include <cassert>
#include "property.hxx"
#include "../utility/template_utils.hxx"

namespace kangsw::refl {

class object_traits {
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

        auto it_insert = std::lower_bound(
            _index.begin(), _index.end(), tag,
            [](auto&& a, auto&& b) { return a.first < b; });

        auto index = _props.size();
        for (auto& pair : _index) { pair.second += pair.second >= index; }
        _index.emplace(it_insert, u8str(tag), index);

        return _props.emplace_back(u8str(tag));
    }

    /** Finds existing property */
    property const* find_property(u8str_view tag) const {
        auto it_index = std::lower_bound(
            _index.begin(), _index.end(), tag,
            [](auto&& pair, auto&& t) { return pair.first < t; });

        if (it_index == _index.end() || it_index->first != tag) {
            return nullptr;
        } else {
            return &_props[it_index->second];
        }
    }

    property* find_property(u8str_view tag) {
        return const_cast<property*>(((object_traits const*)this)
                                         ->find_property(tag));
    }

    // .remove_property

private:
    std::vector<property> _props;
    std::vector<std::pair<u8str, size_t>> _index;
};

} // namespace kangsw::refl
