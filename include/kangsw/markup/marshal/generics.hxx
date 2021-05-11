#pragma once
#include <cassert>
#include <charconv>
#include <optional>
#include "../types.hxx"
#include "../utility/base64.hxx"

namespace kangsw::refl {
namespace impl {
template <typename Ty_> struct _generic_can_trivially_marshalable {
    constexpr bool operator()() {
        auto constexpr type = etype::from_type<Ty_>();
        return !type.is_container() && !type.is_object() && !type.is_string();
    }
};
} // namespace impl

/**
 * Indent options
 */

/**
 * Checks given type can be marshaled via generic marshaling methods
 */
template <typename Ty_>
inline constexpr bool generic_can_trivially_marshalable_v = impl::_generic_can_trivially_marshalable<Ty_>{}();

/**
 * Predict required capacity to stringfy given element
 */
template <typename Ty_>
struct genenric_predict_buffer_length {
    size_t operator()(Ty_ const& i) const;
};

/**
 * Stringfy trivial types into string
 */
template <typename Ty_> struct generic_stringfy {
    template <typename OutIt_> void operator()(Ty_ const& i, OutIt_ o) const;
};

/**
 * Parse trivial types from string
 */
template <typename Ty_> struct generic_parse {

    char const* _impl(char const* begin, char const* end, Ty_& dest) const;

    template <typename It_>
    char const* operator()(It_ begin, It_ end, Ty_& dest) {
        return this->_impl(
          static_cast<char const*>(&*begin),
          static_cast<char const*>(&*begin) + (end - begin), dest);
    }
};

template <typename Ty_> size_t genenric_predict_buffer_length<Ty_>::operator()(Ty_ const& i) const {
    auto constexpr type = etype::from_type<Ty_>();
    if constexpr (type.is_number()) {
        return 32;
    } else if constexpr (type.is_boolean()) {
        return 5;
    } else if constexpr (type.is_binary()) {
        binary_chunk const& v = i;
        return base64::encoded_size(v.size());
    } else if constexpr (type.is_string()) {
        return i.size();
    } else if constexpr (type.is_null()) {
        return 4;
    } else if constexpr (type.is_timestamp()) {
        char const fmt[] = "YYYY-MM-DDThh:mm:ss.SSSZ";
        return sizeof fmt;
    } else {
        return 0;
    }
}

template <typename Ty_> template <typename OutIt_> void generic_stringfy<Ty_>::operator()(Ty_ const& i, OutIt_ o) const {
    auto constexpr type = etype::from_type<Ty_>();
    if constexpr (type.is_number()) {
        char buf[128];
        std::to_chars_result result = std::to_chars(std::begin(buf), std::end(buf), i);
        assert(result.ec == std::errc{});
        auto len = result.ptr - buf;
        std::copy(buf, buf + len, o);
    } else if constexpr (type.is_boolean()) {
        auto const str = i ? "true" : "false";
        std::copy(str, str + (i ? 4 : 5), o);
    } else if constexpr (type.is_binary()) {
        binary_chunk const& v = i;
        base64::encode(i.data(), i.size(), o);
    } else if constexpr (type.is_string()) {
        std::copy(i.begin(), i.end(), o);
    } else if constexpr (type.is_null()) {
        char constexpr str[] = "null";
        std::copy(str, str + 4, o);
    } else if constexpr (type.is_timestamp()) {
        // yyyy-mm-ddThh:MM:ss+hh:MM
        timestamp_t const& t = i;
        using namespace std::chrono;
        auto time = timestamp_t::clock::to_time_t(t);
        auto tm   = *gmtime(&time);
        int frac  = duration_cast<milliseconds>(t.time_since_epoch()).count() % 1000;
        char buf[25];
        auto len = snprintf(buf, sizeof buf, "%4d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                            tm.tm_hour, tm.tm_min, tm.tm_sec, frac);
        std::copy(buf, buf + len, o);
    } else {
        static_assert("This type can't be trivially stringfy-ied");
    }
}

#if _WIN32 // Cross-platform support
static time_t timegm(tm* t) { return _mkgmtime(t); }
#endif

template <typename Ty_> char const* generic_parse<Ty_>::_impl(char const* begin, char const* end, Ty_& dest) const {
    auto constexpr type = etype::from_type<Ty_>();
    auto const maxlen   = end - begin;
    if constexpr (type.is_number()) {
        std::from_chars_result result = std::from_chars(begin, end, dest);
        if (result.ec == std::errc{}) {
            return result.ptr;
        } else {
            return result.ptr;
        }
    } else if constexpr (type.is_boolean()) {
        if (maxlen >= 4 && memcmp(begin, "true", 4) == 0) {
            return dest = true, begin + 4;
        }
        if (maxlen >= 5 && memcmp(begin, "false", 5) == 0) {
            return dest = false, begin + 5;
        }
        return nullptr;
    } else if constexpr (type.is_binary()) {
        binary_chunk& out = dest;
        out.reserve(out.size() + base64::decoded_size(maxlen));
        if (base64::decode(begin, end, std::back_inserter(out))) {
            return end;
        } else {
            return nullptr;
        }
    } else if constexpr (type.is_string()) {
        static_assert("string may not be appropriate to be built by generic_parse");
        return nullptr;
    } else if constexpr (type.is_null()) {
        if (maxlen >= 4 && memcmp(begin, "null", 4) == 0) {
            return begin + 4;
        } else {
            return nullptr;
        }
    } else if constexpr (type.is_timestamp()) {
        //                             0    5  8  11 14 17 20 23
        static char constexpr fmt[] = "YYYY-MM-DDThh:mm:ss.SSSZ";

        auto const get_int = [begin](size_t from, size_t to) -> std::optional<int> {
            int value;
            std::from_chars_result c = std::from_chars(begin + from, begin + to, value);
            if (c.ec == std::errc{}) {
                return value;
            } else {
                return {};
            }
        };

        tm tm;
        const std::pair<int*, std::pair<size_t /*from*/, size_t /*to*/>> assigns[] = {
          {&tm.tm_year, {0, 4}},
          {&tm.tm_mon, {5, 7}},
          {&tm.tm_mday, {8, 10}},
          {&tm.tm_hour, {11, 13}},
          {&tm.tm_min, {14, 16}},
          {&tm.tm_sec, {17, 19}}};

        for (auto [target, pair] : assigns) {
            auto [from, to] = pair;
            auto ovalue     = get_int(from, to);
            if (ovalue) {
                *target = *ovalue;
            } else {
                return nullptr;
            }
        }

        tm.tm_year -= 1900, tm.tm_mon -= 1;

        timestamp_t& out = dest;
        time_t time      = timegm(&tm);
        out              = timestamp_t::clock::from_time_t(time);

        // Milliseconds should treated specially.
        if (begin[20] != 'Z') {
            if (auto ovalue = get_int(20, 23)) {
                std::chrono::milliseconds milli{*ovalue};
                out += milli;
            }

            return begin[23] == 'Z' ? begin + 24 : nullptr;
        } else {
            return begin[19] == 'Z' ? begin + 20 : nullptr;
        }
    } else {
        static_assert("This type can't be trivially stringfy-ied");
        return nullptr;
    }
}
} // namespace kangsw::refl
