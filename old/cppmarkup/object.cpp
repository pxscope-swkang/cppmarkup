#include "object.hpp"

void kangsw::markup::object::reset()
{
    for (auto& p : props()) {
        auto elem = p.memory(this);
        p.pinitializer(p.value(elem));

        for (auto& attr : p.attributes) {
            attr.pinitializer(attr.memory(elem));
        }
    }
}

kangsw::markup::property const* kangsw::markup::object::find_property(u8string_view name)
{
    auto prs     = props();
    auto find_it = std::find_if(prs.begin(), prs.end(),
                                [&name](property const& c) { return c.tag == name; });

    return find_it == prs.end() ? nullptr : &*find_it;
}
