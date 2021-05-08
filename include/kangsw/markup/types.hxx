#pragma once
#include "utility/template_utils.hxx"
#include <string>
#include <string_view>
#include <vector>
#include <chrono>

namespace kangsw ::refl {
/** forwardings */
class object;

/** aliases */
using u8str      = std::string;
using u8str_view = std::string_view;
using clock_type = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;

/** bin_pack */
struct binary_chunk : std::vector<std::byte> {
    using std::vector<std::byte>::vector;

    auto& chars() const { return reinterpret_cast<std::vector<char> const&>(*this); }
    auto& chars() { return reinterpret_cast<std::vector<char>&>(*this); }
};

/** bool wrapper for boolean vector. Maps to bool 1:1. To avoid vector<bool> specialization */
struct boolean_t {
    boolean_t() = default;
    boolean_t(bool value) { *this = value; }
    boolean_t& operator=(bool value) { return _value = value, *this; }
    operator bool&() { return _value; }
    operator bool const &() const { return _value; }

    bool _value;
};

/** element type */
class etype {
public:
    enum _type : uint8_t {
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
    constexpr etype(_type v = {}) noexcept : _value(v)       {}
    constexpr etype(int  v)      noexcept : _value((_type)v) {}
    // clang-format on

    constexpr _type& operator=(_type v) noexcept { return _value = v, *this; }
    constexpr _type& operator=(int v) noexcept { return _value = (_type)v, *this; }

    constexpr operator _type const &() const noexcept { return _value; }
    constexpr operator _type&() noexcept { return _value; }

    constexpr bool is_map() const noexcept { return _value & map; }
    constexpr bool is_array() const noexcept { return _value & array; }
    constexpr bool is_container() const noexcept { return _value & (map | array); }
    constexpr bool is_object() const noexcept { return exact_type() == object; }
    constexpr bool is_number() const noexcept { return _value & number; }

    constexpr etype exact_type() const noexcept { return _value & value_mask; }
    constexpr _type get() const noexcept { return _value; }

public:
    template <typename Ty_>
    constexpr static etype from_type() {
        using namespace templates;
        using namespace std;
        using eval_type = std::remove_const_t<std::remove_reference_t<Ty_>>;

        // clang-format off
        if      constexpr(is_same_v<eval_type, bool>) { return boolean; }
        else if constexpr(is_same_v<eval_type, boolean_t>) { return boolean; }
        else if constexpr(is_same_v<eval_type, nullptr_t>) { return null; }
        else if constexpr(is_same_v<eval_type, binary_chunk>) { return binary; }
        else if constexpr(is_integral_v<eval_type>) { return integer | number; }
        else if constexpr(is_floating_point_v<eval_type>) { return floating_point | number; }
        else if constexpr(is_same_v<eval_type, char const*>) { return string; }
        else if constexpr(is_specialization_of<eval_type, basic_string>::value) { return string; }
        else if constexpr(is_specialization_of<eval_type, std::chrono::time_point>::value) { return timestamp; }
        else if constexpr(is_base_of_v<refl::object&, eval_type>) { return object; }
        else if constexpr(is_specialization_of<eval_type, std::vector>::value) {
            using value_type = typename eval_type::value_type;
            static_assert(!from_type<value_type>().is_container(), "nested container is not supported.");
            return array | from_type<value_type>();
        }
        else if constexpr(is_specialization_of<eval_type, std::map>::value) {
            using mapped_type = typename eval_type::mapped_type;
            static_assert(!from_type<mapped_type>().is_container(), "nested container is not supported.");
            static_assert(is_same_v<u8str, typename eval_type::key_type>, "key of map type must be 'u8str'");
            return map | from_type<mapped_type>();
        }
        else { static_assert(false, "Unsupported type"); return {}; }
        // clang-format on
    }

private:
    template <int V_>
    static constexpr decltype(auto) _deduce_from_exact() {
        auto constexpr V = V_ & ~number;

        // clang-format off
        if constexpr (V == null)   { return nullptr; }
        if constexpr (V == object) { return static_cast<refl::object*>(nullptr); }
        if constexpr (V == boolean) { return static_cast<boolean_t*>(nullptr);}
        if constexpr (V == string) { return static_cast<u8str*>(nullptr); }
        if constexpr (V == integer) { return static_cast<int64_t*>(nullptr); }
        if constexpr (V == floating_point) { return static_cast<double*>(nullptr); }
        if constexpr (V == binary) { return static_cast<binary_chunk*>(nullptr); }
        if constexpr (V == timestamp) { return static_cast<clock_type*>(nullptr); }
        // clang-format on
    }

    template <int V_>
    static constexpr decltype(auto) _deduce_from() {
        auto constexpr V = V_ & ~number;

        if constexpr (!etype(V).is_container()) {
            return _deduce_from_exact<V_>();
        }
        if constexpr (etype(V).is_array()) {
            using value_type = std::remove_reference_t<decltype(*_deduce_from_exact<etype(V).exact_type()>())>;
            return static_cast<std::vector<value_type>*>(nullptr);
        }
        if constexpr (etype(V).is_map()) {
            using value_type = std::remove_reference_t<decltype(*_deduce_from_exact<etype(V).exact_type()>())>;
            return static_cast<std::map<u8str, value_type>*>(nullptr);
        }
    }

    template <int V_, typename Ty_>
    using _deduced_type_t =
        std::conditional_t<
            std::is_const_v<std::remove_reference_t<Ty_>>,
            std::remove_reference_t<decltype(*_deduce_from<V_>())> const*,
            std::remove_reference_t<decltype(*_deduce_from<V_>())>*>;

public:
    template <typename Ty_>
    using deduce_result_t = std::remove_pointer_t<_deduced_type_t<from_type<Ty_>(), Ty_>>;

    template <typename Ty_>
    static decltype(auto) deduce(Ty_&& v) {
        if constexpr (std::is_base_of_v<refl::object, Ty_>) {
            return std::forward<Ty_>(v);
        } else if constexpr (std::is_same_v<Ty_, nullptr_t>) {
            return nullptr;
        } else if constexpr (templates::is_specialization_of<Ty_, std::vector>::value) {
            return std::vector<deduce_result_t<typename Ty_::value_type>>(v.begin(), v.end());
        } else if constexpr (templates::is_specialization_of<Ty_, std::map>::value) {
            return std::map<u8str, deduce_result_t<typename Ty_::value_type>>(v.begin(), v.end());
        } else {
            return deduce_result_t<Ty_>(std::forward<Ty_>(v));
        }
    }

    template <typename Ty_, size_t N>
    static decltype(auto) deduce(Ty_ (&v)[N]) {
        return u8str(v);
    }

    template <typename Ty_>
    static decltype(auto) deduce(std::initializer_list<Ty_> v) {
        return std::vector<deduce_result_t<Ty_>>(v.begin(), v.end());
    }

private:
    _type _value;
};

} // namespace kangsw::refl
