#pragma once
#include <string>
#include <vector>
#include "template_utils.hxx"

namespace kangsw::refl {
/** forwardings */
class object;

/** aliases */
using u8str = std::string;

/** bin_pack */
struct binary_chunk {
    auto& bytes() const { return _data; }
    auto& bytes() { return _data; }
    auto& chars() const { return reinterpret_cast<std::vector<char> const&>(_data); }
    auto& chars() { return reinterpret_cast<std::vector<char>&>(_data); }

    std::vector<std::byte> _data;
};

/** element type */
class element_type {
public:
    enum type : uint8_t {
        null           = 0x00,
        boolean        = 0x01,
        integer        = 0x02,
        floating_point = 0x03,
        string         = 0x04,
        timestamp      = 0x05,
        binary         = 0x06,
        object         = 0x07,

        reserved_container = 0x10,
        number             = 0x20,
        map                = 0x40,
        array              = 0x80,

        value_mask = 0x07,
    };

public:
    // clang-format off
    constexpr element_type(type v = {}) noexcept : _value(v)       {}
    constexpr element_type(int  v)      noexcept : _value((type)v) {}
    // clang-format on

    constexpr type& operator=(type v) noexcept { return _value = v, *this; }
    constexpr type& operator=(int v) noexcept { return _value = (type)v, *this; }

    constexpr operator type const &() const noexcept { return _value; }
    constexpr operator type&() noexcept { return _value; }

    constexpr bool is_map() const noexcept { return _value & map; }
    constexpr bool is_array() const noexcept { return _value & array; }
    constexpr bool is_object() const noexcept { return _value & object; }

    constexpr bool get_value() const noexcept { return _value & value_mask; }

    template <typename Ty_>
    static element_type from_type()
    {

    }

    template<typename Ty_>
    static decltype(auto) deduce(Ty_&& v)
    {
        
    }

private:
    type _value;
};

} // namespace kangsw::refl
