#pragma once
#include <string>

namespace kangsw::refl {
/** forwardings */
class object;

/** aliases */
using u8str = std::string;

/** element type */
class element_type {
public:
    enum type : uint8_t {
        null,
        boolean,
        integer,
        floating_point,
        string,
        timestamp,
        binary,
        object,

        number = 0x20,
        map    = 0x40,
        array  = 0x80,
    };

public:
    // clang-format off
    element_type(type v) : _value(v)       {}
    element_type(int  v) : _value((type)v) {}
    // clang-format on

    operator type() const { return _value; }

    bool is_map() const { return _value & map; }
    bool is_array() const { return _value & array; }
    bool is_object() const { return _value & object; }

private:
    type _value = {};
};

} // namespace kangsw::refl
