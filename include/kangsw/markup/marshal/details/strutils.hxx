#pragma once
#include <utility>
#include "kangsw/markup/types.hxx"

namespace kangsw::refl::marshal::utils {

template <size_t N_>
static bool one_of(char const (&s)[N_], char ch) {
    return std::end(s) != std::find(std::begin(s), std::end(s), ch);
}

static bool spacechars(char const ch) { return one_of(" \t\n\r\f\b", ch); }
static bool between(char const ch, char a, char b) { return a <= ch && ch <= b; }
static bool digit(char const ch) { return between(ch, '0', '9'); }
static bool hex(char const ch) { return between(ch, '0', '9') || between(ch, 'a', 'f'); }
static bool alphabet(char const ch) { return between(ch, 'a', 'z') | between(ch, 'A', 'Z'); }
static bool alphanumeric(char const ch) { return digit(ch) | alphabet(ch) | (ch == '_'); }

template <typename Matches_, typename Allows_ = bool (*)(char)>
std::pair<bool, size_t> find_until_match(
  u8str_view in, Matches_&& match, Allows_&& allow = [](char) { return true; }) {
    for (size_t i = 0, end = in.size(); i < end; ++i) {
        char const ch = in[i];

        if (match(ch)) {
            return {true, i};
        } else if (!allow(ch)) {
            break;
        }
    }
    return {false, in.size()};
}

constexpr auto matcher(char const ch) {
    return [ch](char in) { return in == ch; };
}

constexpr auto matcher(u8str_view chset) {
    return [chset](char in) { return chset.find(in) != chset.npos; };
}

constexpr auto escape_handling_matcher(char const to_match) {
    struct _escaped_matcher {
        bool operator()(char const ch) const {
            if (escaping) { // escape whatever next char is.
                escaping = false;
                return false;
            } else if (ch == '\\') {
                escaping = true;
                return false;
            } else if (ch == match_target) {
                return true;
            } else {
                return false;
            }
        }
        char const match_target;
        mutable bool escaping = false;
    };

    return _escaped_matcher{to_match};
}

inline u8str_view remove_suffix_if_found(u8str_view s, u8str_view find) {
    if (s.size() < find.size()) {
        return {};
    } else {
        return s.substr(s.size() - find.size()) == find
                 ? s.substr(0, s.size() - find.size() + 1)
                 : u8str_view{};
    }
}
} // namespace kangsw::refl::marshal::utils
