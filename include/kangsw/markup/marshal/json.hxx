#pragma once
#include "../reflection/property_proxy.hxx"
#include "generics.hxx"

namespace kangsw::refl::marshal {

/**
 * Finds a complete json object from continuous stream.
 *
 * Use this class when you have continous json object stream, but boundary of each object
 *is unclear, due to various reasons(stream is not devided by null character, or no total
 *length is provided ... etc)
 *
 * Automatically minifies input json stream.
 */
class json_fence {
public:
    enum result_t {
        ready,
        error,
        done
    };

private:
    enum class _nextc : uint8_t {
        opening_brace, // always the first
        closing_brace,
        comma_or_closing_brace,
        closing_bracket,          // met [, looking for ']'
        closing_bracket_or_comma, // met [, looking for ']'

        tag_colon,
        string_escaped_next, // after '\'
        string_escape_u_0,   // \u:
        string_escape_u_1,   // \ux:
        string_escape_u_2,   // \uxx:
        string_escape_u_3,   // \uxxx:

        tag_opening_quote,
        string_closing_quote,

        value_begin, // expect any value begin character
        value_end,   // to prevent "abc": 243 142, is treated valid, first searches end of the value.

        begin_comment_next,       // met '/' in non-string context, expecting '/' or '*'
        line_comment_newline,     // skipping all characters untill met newline
        block_comment_0_asterisk, // looking for '*' of */
        block_comment_1_slash,    // after *, looking for '/'
    };

    template <size_t N_>
    static bool one_of(char const (&s)[N_], char ch) {
        return std::end(s) != std::find(std::begin(s), std::end(s), ch);
    }

public:
    json_fence(u8str& out) : _out(out) {
        _st.reserve(32);
        _st.push_back(_nextc::opening_brace);
        _out.clear();
    }

    result_t operator()(char ch) {
        if (_st.empty()) { return error; }

        switch (_st.back()) {
            // TODO: 内膏飘 贸府 眠啊 ... 葛电 冀记俊 '/' 贸府 风凭
            case _nextc::opening_brace:
                if (ch == '{') {
                    apnd(ch), replace(_nextc::closing_brace);
                    return ready;
                } else if (spacechars(ch)) {
                    return ready;
                } else {
                    return error;
                }

            case _nextc::comma_or_closing_brace:
                if (ch == ',') {
                    push(_nextc::tag_colon);
                    push(_nextc::string_closing_quote);
                    push(_nextc::tag_opening_quote); // tag must follow after comma
                    return ready;
                }
                [[fallthrough]];
            case _nextc::closing_brace:
                if (ch == '}') {
                    apnd(ch), pop();
                    return _st.empty() ? done : ready;
                } else if (spacechars(ch)) {
                    return ready;
                } else if (ch == '"') { // beginning of first-most tag
                    apnd(ch);
                    replace(_nextc::comma_or_closing_brace); // can look for next value after all.
                    push(_nextc::tag_colon);
                    push(_nextc::string_closing_quote);
                    return ready;
                } else {
                    return error;
                }

            case _nextc::string_closing_quote:
                if (ch == '\\') { // escape sequence ... does not append
                    return push(_nextc::string_escaped_next), ready;
                } else if (ch == '"') {
                    return apnd(ch), pop(), ready;
                } else if (!iscntrl(ch)) {
                    return apnd(ch), ready;
                } else {
                    return error;
                }

            case _nextc::tag_opening_quote:
                if (ch == '"') {
                    return apnd(ch), pop(), ready;
                } else if (spacechars(ch)) {
                    return ready;
                }

            case _nextc::tag_colon:
                if (ch == ':') {
                    return apnd(ch), replace(_nextc::value_begin), ready;
                } else if (spacechars(ch)) {
                    return ready;
                } else {
                    return error;
                }
                
            case _nextc::closing_bracket_or_comma:
                if (ch == ',') {
                    return apnd(ch), push(_nextc::value_begin), ready;
                }
                [[fallthrough]];
            case _nextc::closing_bracket:
                if (ch == ']') {
                    return apnd(ch), pop(), ready;
                } 
                [[fallthrough]];
            case _nextc::value_begin:
                if (spacechars(ch)) {
                    return ready;
                } else if (ch == '{') {
                    return apnd(ch), push(_nextc::closing_brace), ready;
                } else if (ch == '[') {
                    return apnd(ch), push(_nextc::closing_bracket), ready;
                } else if (ch == '"') {
                    return apnd(ch), replace(_nextc::string_closing_quote), ready;
                } else if (one_of("tfn", ch) || digit(ch)) {
                    return apnd(ch), replace(_nextc::value_end), ready;
                } else {
                    return error;
                }

            case _nextc::value_end:
                if (ch == '.' || alphanumeric(ch)) {
                    return apnd(ch), ready;
                } else if (spacechars(ch)) {
                    return pop(), ready;
                } else if (one_of(",}]", ch)) {
                    return pop(), (*this)(ch);
                } else {
                    return error;
                }

            case _nextc::string_escaped_next: break;
            case _nextc::begin_comment_next: break;
            case _nextc::line_comment_newline: break;
            case _nextc::block_comment_0_asterisk: break;
            case _nextc::block_comment_1_slash: break;

            case _nextc::string_escape_u_0: break;
            case _nextc::string_escape_u_1: break;
            case _nextc::string_escape_u_2: break;
            case _nextc::string_escape_u_3: break;

            default:;
        }

        return ready;
    }

private:
    static bool spacechars(char const ch) { return one_of(" \t\n\r\f\b", ch); }
    static bool between(char const ch, char a, char b) { return a <= ch && ch <= b; }
    static bool digit(char const ch) { return between(ch, '0', '9'); }
    static bool alphabet(char const ch) { return between(ch, 'a', 'z') | between(ch, 'A', 'Z'); }
    static bool alphanumeric(char const ch) { return digit(ch) | alphabet(ch) | (ch == '_'); }

    void pop() { _st.pop_back(); }
    void push(_nextc c) { _st.push_back(c); }
    void replace(_nextc c) { pop(), push(c); }
    void apnd(char ch) { _out += ch; }

private:
    u8str& _out;
    std::vector<_nextc> _st = {};
};

/** Parse string as the schema of given object. */
class json_parse {
public:
    /** Parse input string_view as */
    bool operator()(object& dest, u8str_view i);
};

/** Dump given object as json */
class json_dump {
public:
    void operator()(object const& obj, string_output o);

    struct _visitor {
        template <typename Ty_> void operator()(property_proxy<Ty_, true> p);
        string_output& o;
    };
};

// //////////////////////////////////// IMPLEMENTATION ////////////////////////////////////  //
// //////////////////////////////////// IMPLEMENTATION ////////////////////////////////////  //

namespace json {
template <typename Ty_>
void _dump(Ty_ const& v, string_output& o) {
    auto constexpr T = etype::from_type<Ty_>();
    using E          = etype::_type;

    if constexpr (T.is_one_of(E::floating_point, E::integer, E::boolean, E::null)) {
        generic_stringfy<Ty_>{}(v, std::back_inserter(o.str()));
    } else if constexpr (T.is_one_of(etype::string, etype::timestamp, etype::binary)) {
        o << '"';
        generic_stringfy<Ty_>{}(v, std::back_inserter(o.str()));
        o << '"';
    } else {
        static_assert(false, "Should not enter here.");
    }
}

template <>
inline void _dump<object>(object const& v, string_output& o) {
    o << '{', ++o; // Write value first -> indent later
    auto const baseaddr = v.base();

    for (auto& prop : v.properties()) {
        o << break_indent;

        if (!prop.attributes().empty()) {
            // "PropTag~@@ATTR@@": {
            o.wrap('"', prop.tag(), "~@@ATTR@@") << ": {";
            ++o;

            for (auto& attr : prop.attributes()) {
                o << break_indent;

                // "Attribute tag": value
                o.wrap('"', attr.name) << ": ";
                visit_property(baseaddr, attr, json_dump::_visitor{o});

                size_t attr_idx = &attr - prop.attributes().data();
                if (attr_idx + 1 < prop.attributes().size()) { o << ','; }
            }

            --o;
            o << break_indent << "}," << break_indent;
        }

        o.wrap('"', prop.tag()) << ": ";
        visit_property(baseaddr, prop, json_dump::_visitor{o});

        size_t prop_idx = &prop - v.properties().data();
        if (prop_idx + 1 < v.properties().size()) { o << ','; }
    }

    --o, o << break_indent << '}';
}

template <typename Ty_>
void _dump(property_proxy<std::vector<Ty_>, true> v, string_output& o) {
    o << '[', ++o;

    for (size_t i = 0, end = v.size(); i < end; ++i) {
        o << break_indent;
        auto& elem = v[i];
        _dump(elem, o);

        if (i + 1 < end) { o << ", "; }
    }

    --o, o << break_indent << ']';
}

template <typename Ty_>
void _dump(property_proxy<u8str_map<Ty_>, true> v, string_output& o) {
    o << '{', ++o;
    size_t counter = 0;
    size_t size    = v.size();

    v.for_each([&o, &counter, size](u8str_view s, Ty_ const& v_i) {
        o << break_indent;
        o.wrap('"', s) << ": ";
        _dump(v_i, o);

        if (++counter < size) { o << ", "; }
    });

    --o, o << break_indent << ']';
}

} // namespace json

template <typename Ty_>
void json_dump::_visitor::operator()(property_proxy<Ty_, true> p) {
    auto constexpr T = etype::from_type<Ty_>();
    if constexpr (!T.is_container()) {
        json::_dump(*p, o);
    } else {
        json::_dump(p, o);
    }
}

inline void json_dump::operator()(object const& obj, string_output o) {
    json::_dump(obj, o);
    o << break_indent;
}
} // namespace kangsw::refl::marshal