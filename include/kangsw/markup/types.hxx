#pragma once
#include "utility/template_utils.hxx"
#include <stdexcept>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>
#include <chrono>

namespace kangsw ::refl {
/** forwardings */
class object;
struct object_baseaddr_t; // Never dereferenced.
class object_traits;

  /** aliases */
using u8str       = std::string;
using u8str_view  = std::string_view;
using timestamp_t = std::chrono::system_clock::time_point; //std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;
using integer_t   = int64_t;
using float_t     = double;

template <typename Ty_>
class u8str_map : public std::map<u8str, Ty_, std::less<>> {
public:
    using std::map<u8str, Ty_, std::less<>>::map;
};

/** bin_pack */
struct binary_chunk : std::vector<std::byte> {
    using std::vector<std::byte>::vector;

    auto& chars() const noexcept { return reinterpret_cast<std::vector<char> const&>(*this); }
    auto& chars() noexcept { return reinterpret_cast<std::vector<char>&>(*this); }

    template <typename Ty_>
    void write(Ty_ const& value) {
        static_assert(std::is_trivial_v<Ty_>);
        std::initializer_list il(
          reinterpret_cast<std::byte const*>(&value),
          reinterpret_cast<std::byte const*>(&value + 1));
        this->insert(this->end(), il);
    }

    template <typename Ty_>
    void write(Ty_ const* data, const size_t n) {
        static_assert(std::is_trivial_v<Ty_>);
        this->reserve(size() + n * sizeof(Ty_));

        std::initializer_list const il(
          reinterpret_cast<std::byte const*>(data),
          reinterpret_cast<std::byte const*>(data + n));
        this->insert(this->end(), il);
    }

    template <typename It_>
    void write(It_ begin, It_ end) {
        using Ty_ = typename std::iterator_traits<It_>::value_type;
        static_assert(std::is_trivial_v<Ty_>);
        auto dist = std::distance(begin, end);
        this->reserve(size() + dist * sizeof(Ty_));

        for (; begin != end; ++begin) {
            this->write(*begin);
        }
    }

    template <typename... Args_>
    void write_many(Args_&&... args) {
        (this->write(std::forward<Args_>(args)), ...);
    }

    template <typename... Args_>
    static binary_chunk from(Args_&&... args) {
        binary_chunk out;
        return out.write_many(std::forward<Args_>(args)...), out;
    }
};

/** bool wrapper for boolean vector. Maps to bool 1:1. To avoid vector<bool> specialization */
struct boolean_t {
    boolean_t() noexcept = default;
    boolean_t(bool value) noexcept { *this = value; }
    boolean_t& operator=(bool value) noexcept { return _value = value, *this; }
    operator bool&() noexcept { return _value; }
    operator bool const &() const noexcept { return _value; }

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

        map                = 0x40,
        array              = 0x80,

        _value_mask = 0x07,
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
    constexpr bool is_number() const noexcept { return is_integer() || is_floating_point(); }

    constexpr bool is_object() const noexcept { return leap() == object; }
    constexpr bool is_boolean() const noexcept { return leap() == boolean; }
    constexpr bool is_timestamp() const noexcept { return leap() == timestamp; }
    constexpr bool is_binary() const noexcept { return leap() == binary; }
    constexpr bool is_string() const noexcept { return leap() == string; }
    constexpr bool is_null() const noexcept { return leap() == null; }
    constexpr bool is_integer() const noexcept { return leap() == integer; }
    constexpr bool is_floating_point() const noexcept { return leap() == floating_point; }

    template <typename... Args_> // requires std::is_same_v<Args_, _type>
    constexpr bool is_one_of(Args_... args) const noexcept {
        return ((leap() == args) || ...);
    }

    constexpr etype leap() const noexcept { return _value & _value_mask; }
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
        else if constexpr(is_integral_v<eval_type>) { return integer ; }
        else if constexpr(is_floating_point_v<eval_type>) { return floating_point ; }
        else if constexpr(is_same_v<eval_type, char const*>) { return string; }
        else if constexpr(is_specialization_of<eval_type, basic_string>::value) { return string; }
        else if constexpr(is_specialization_of<eval_type, std::chrono::time_point>::value) { return timestamp; }
        else if constexpr(is_base_of_v<refl::object, eval_type>) { return object; }
        else if constexpr(is_specialization_of<eval_type, std::vector>::value) {
            using value_type = typename eval_type::value_type;
            static_assert(!from_type<value_type>().is_container(), "nested container is not supported.");
            return array | from_type<value_type>();
        }
        else if constexpr(is_specialization_of<eval_type, u8str_map>::value) {
            using mapped_type = typename eval_type::mapped_type;
            static_assert(!from_type<mapped_type>().is_container(), "nested container is not supported.");
            static_assert(is_same_v<u8str, typename eval_type::key_type>, "key of map type must be 'u8str'");
            return map | from_type<mapped_type>();
        }
        else { static_assert(false, "Unsupported type"); return {}; }
        // clang-format on
    }

    template <typename Ty_>
    constexpr static etype from_type_exact() {
        using eval_type = Ty_;
        using namespace templates;
        using namespace std;

        // clang-format off
        if      constexpr(is_same_v<Ty_, nullptr_t>) { return null; }
        else if constexpr(is_same_v<Ty_, boolean_t>) { return boolean; }
        else if constexpr(is_same_v<Ty_, int64_t>) { return integer; }
        else if constexpr(is_same_v<Ty_, double>) { return floating_point; }
        else if constexpr(is_same_v<Ty_, u8str>) { return string; }
        else if constexpr(is_same_v<Ty_, binary_chunk>) { return binary; }
        else if constexpr(is_same_v<Ty_, timestamp_t>) { return timestamp; }
        else if constexpr(is_base_of_v<refl::object, Ty_>) { return object; }
        else if constexpr(is_specialization_of<eval_type, std::vector>::value) {
            using value_type = typename eval_type::value_type;
            static_assert(!from_type_exact<value_type>().is_container(), "nested container is not supported.");
            return array | from_type_exact<value_type>();
        }
        else if constexpr(is_specialization_of<eval_type, u8str_map>::value) {
            using mapped_type = typename eval_type::mapped_type;
            static_assert(is_same_v<u8str, typename eval_type::key_type>, "key of map type must be 'u8str'");
            static_assert(!from_type_exact<mapped_type>().is_container(), "nested container is not supported.");
            return map |   from_type_exact<mapped_type>();
        }
        else { static_assert(false, "Not a valid markup object member type"); return {}; }
        // clang-format on
    }

private:
    template <int V_>
    static constexpr decltype(auto) _deduce_from_exact() {
        // clang-format off
        if constexpr (V_ == null)   { return static_cast<nullptr_t*>(nullptr); }
        if constexpr (V_ == object) { return static_cast<refl::object*>(nullptr); }
        if constexpr (V_ == boolean) { return static_cast<boolean_t*>(nullptr);}
        if constexpr (V_ == string) { return static_cast<u8str*>(nullptr); }
        if constexpr (V_ == integer) { return static_cast<int64_t*>(nullptr); }
        if constexpr (V_ == floating_point) { return static_cast<double*>(nullptr); }
        if constexpr (V_ == binary) { return static_cast<binary_chunk*>(nullptr); }
        if constexpr (V_ == timestamp) { return static_cast<timestamp_t*>(nullptr); }
        // clang-format on
    }

    template <int V_>
    static constexpr decltype(auto) _deduce_from() {
        if constexpr (etype(V_).is_array()) {
            using value_type = std::remove_reference_t<decltype(*_deduce_from_exact<etype(V_).leap()>())>;
            return static_cast<std::vector<value_type>*>(nullptr);
        }
        if constexpr (etype(V_).is_map()) {
            using value_type = std::remove_reference_t<decltype(*_deduce_from_exact<etype(V_).leap()>())>;
            return static_cast<u8str_map<value_type>*>(nullptr);
        }
        if constexpr (!etype(V_).is_container()) {
            return _deduce_from_exact<V_>();
        }
    }

    template <int V_, typename Ty_>
    using _deduced_type_t = std::remove_reference_t<decltype(*_deduce_from<V_>())>*;
    // std::conditional_t<
    //     std::is_const_v<std::remove_reference_t<Ty_>>,
    //     std::remove_reference_t<decltype(*_deduce_from<V_>())> const*,
    //     std::remove_reference_t<decltype(*_deduce_from<V_>())>*>;

    template <typename Ty_>
    using _deduce_result_t = std::remove_pointer_t<_deduced_type_t<from_type<Ty_>(), Ty_>>;

public:
    template <int V_>
    using to_type_t = std::remove_pointer_t<decltype(_deduce_from<V_>())>;

    template <typename Ty_, size_t N>
    static decltype(auto) deduce(Ty_ (&v)[N]) {
        return u8str(reinterpret_cast<const char*>(v));
    }

    template <typename Ty_>
    static decltype(auto) deduce(std::initializer_list<Ty_> v) {
        return std::vector<_deduce_result_t<Ty_>>(v.begin(), v.end());
    }

    template <typename Ty_>
    static decltype(auto) deduce(Ty_&& v) {
        using eval_type = std::remove_reference_t<Ty_>;

        if constexpr (std::is_base_of_v<refl::object, eval_type>) {
            return eval_type(std::forward<Ty_>(v));
        } else if constexpr (std::is_same_v<eval_type, nullptr_t>) {
            return nullptr;
        } else if constexpr (templates::is_specialization_of<eval_type, std::vector>::value) {
            return std::vector<decltype(deduce(typename Ty_::value_type{}))>(v.begin(), v.end());
        } else if constexpr (templates::is_specialization_of<eval_type, u8str_map>::value) {
            return u8str_map<decltype(deduce(typename Ty_::mapped_type{}))>(v.begin(), v.end());
        } else if constexpr (templates::is_specialization_of<eval_type, std::chrono::time_point>::value) {
            return std::chrono::time_point_cast<timestamp_t::duration, timestamp_t::clock>(v);
        } else {
            // return _deduce_result_t<Ty_>(std::forward<eval_type>(v));

            using namespace std;
            using namespace templates;

            // clang-format off
            if      constexpr(is_same_v<eval_type, bool>) { return boolean_t(v); }
            else if constexpr(is_same_v<eval_type, boolean_t>) { return boolean_t(v); }
            else if constexpr(is_same_v<eval_type, nullptr_t>) { return nullptr; }
            else if constexpr(is_same_v<eval_type, binary_chunk>) { return binary_chunk{std::forward<Ty_>(v)}; }
            else if constexpr(is_integral_v<eval_type>) { return integer_t(v); }
            else if constexpr(is_floating_point_v<eval_type>) { return float_t(v); }
            else if constexpr(is_same_v<eval_type, char const*>) { return u8str(v); }
#if __cplusplus >= 202000
            else if constexpr(is_same_v<eval_type, char8_t const*>) { return u8str(reinterpret_cast<char const*>(v)); }
#endif
            else if constexpr(is_specialization_of<eval_type, basic_string>::value) { return u8str(v.begin(), v.end()); }
            else { static_assert("unsupported type"); }
            // clang-format on
        }
    }

private:
    _type _value;
};

} // namespace kangsw::refl

