#pragma once
#include <assert.h>
#include <charconv>
#include "types.hxx"
#include "utility/base64.hxx"

namespace kangsw::refl {

template <typename Ty_> struct generic_can_trivially_marshalable {
    constexpr bool operator()() {
        auto constexpr type = etype::from_type<Ty_>();
        return !type.is_container() && !type.is_object();
    }
};

template<typename Ty_>
inline constexpr bool generic_can_trivially_marshalable_v = generic_can_trivially_marshalable<Ty_>{}();

/** Predict required capacity to stringfy given element */
template <typename Ty_>
struct genenric_predict_buffer_length {
    size_t operator()(Ty_ const& i) const {
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
            // yyyy-mm-ddThh:MM:ss.SSSZ
            return 25;
        } else {
            return 0;
        }
    }
};

template <typename Ty_> struct generic_stringfy {
    template <typename OutIt_>
    void operator()(Ty_ const& i, OutIt_ o) const {
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
            char buf[32];
            auto len = snprintf(buf, sizeof buf, "%4d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, frac);
            std::copy(buf, buf + len, o);
        } else {
            static_assert("This type can't be trivially stringfy-ied");
        }
    }
};

template <typename Ty_> struct genenric_parse {
    template <typename It_>
    It_ operator()(It_ begin, It_ end, Ty_&& dest) const {}
};

} // namespace kangsw::refl
