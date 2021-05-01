#pragma once
#include <string>
#include <string_view>
#include <vector>

#if __cplusplus > 202000
#include <span>
#else
#endif

/** ������ */
namespace kangsw::markup {
namespace impl {
    class basic_marshaller;
    template <typename> class element_template_base;
    class element_base;
} // namespace impl

class object;
} // namespace kangsw::markup

namespace kangsw::markup {
#if __cplusplus > 202000
using u8string      = std::u8string;
using u8string_view = std::u8string_view;
#else
using u8string            = std::string;
using u8string_view       = std::string_view;
#endif

/** ������� ���̳ʸ� Ÿ���� ��Ÿ���ϴ�. Markup���� base64�� ���ڵ� ��. */
struct binary_chunk {
    std::vector<std::byte> data;
};

/** ������ ��� �ۼ����� ���� ���̳ʸ� ǥ���Դϴ�. */
enum class compact_byte : uint8_t;
using compact_binary = std::vector<compact_byte>;

#if __cplusplus > 202000
using compact_binary_view = std::span<compact_byte>;
#else
using compact_binary_view = std::vector<compact_byte> const&;
#endif

/**
 * ���ȸ� ���� �ڵ�
 */
struct marshalerr_t {
    enum type : intptr_t {
        ok,
        fail = std::numeric_limits<std::underlying_type_t<type>>::min(), // 0x10000000'00000000...
    } value;

    // clang-format off
    constexpr marshalerr_t(type v = ok) : value(v) {}
    constexpr marshalerr_t(std::underlying_type_t<type> v) : value((type)v) {}
    // clang-format on

    constexpr operator type() const { return value; }
    constexpr operator bool() const { return !(value & fail); }
};

/**
 * ����� ������ ��Ÿ���ϴ�. ������Ʈ ~ �� ������Ʈ, ��̸� �����ϴ� �뵵�ո� ���.
 */
struct element_type {
    enum type : uint8_t {
        null,
        boolean,
        integer,
        floating_point,
        string,
        object,
        binary,

        number = 0x10,

        map   = 0x40,
        array = 0x80,
    } value;

    // clang-format off
    constexpr element_type(type v = null) : value(v) {}
    constexpr element_type(std::underlying_type_t<type> v) : value((type)v) {}
    // clang-format on

    constexpr operator type() const { return value; }
    constexpr bool is_array() const { return value & array; }
    constexpr bool is_map() const { return value & map; }

    constexpr type value_type() const { return type(value & ~(map | array | number)); }
};

} // namespace kangsw::markup