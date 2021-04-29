#include "object.hpp"

void kangsw::markup::object::reset()
{
    for (auto& p : props()) {
        auto elem = p.get(this);
        p.pinitializer(elem);

        for (auto& attr : p.attributes) {
            attr.pinitializer(attr.get(elem));
        }
    }
}
