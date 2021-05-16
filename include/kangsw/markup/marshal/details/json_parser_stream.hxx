#pragma once
#include <variant>

#include "strutils.hxx"
#include "generics.hxx"
#include "trivial_marshal.hxx"
#include "kangsw/markup/reflection/object.hxx"
#include "kangsw/markup/reflection/property.hxx"

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
class json_parser_stream {
public:
    enum result_t {
        ready,
        error,
        done
    };

private:
    struct _nextc {
        enum type : uint8_t {
            opening_brace, // always the first
            closing_brace,
            comma_or_closing_brace,
            closing_bracket,          // met [, looking for ']'
            comma_or_closing_bracket, // met [, looking for ']'

            tag_colon,
            string_escaped_next,                             // after '\'
            string_escape_u_begin,                           // \u:
            string_escape_u_end = string_escape_u_begin + 4, // \u:

            tag_opening_quote,
            string_closing_quote,

            value_begin, // expect any value begin character
            value_end,   // to prevent "abc": 243 142, is treated valid, first searches end of the value.

            begin_comment_next,       // met '/' in non-string context, expecting '/' or '*'
            line_comment_newline,     // skipping all characters untill met newline
            block_comment_0_asterisk, // looking for '*' of */
            block_comment_1_slash,    // after *, looking for '/'
        };
    };

public:
    json_parser_stream(object& root, u8str* buf);

    result_t operator()(char ch);

private:
    void pop() { _st.pop_back(); }
    void push(_nextc::type c) { _st.push_back(c); }
    void replace(_nextc::type c) { pop(), push(c); }
    void apnd(char ch) { _out += ch; }

    void tag_token_begin() {
        _token.begin_index = _out.size();
        _token.has_escape  = false;
    }

    void tag_token_end() {
        auto token = u8str_view{_out}.substr(_token.begin_index);

        // if it has escape, unescape characters into raw.
        if (_token.has_escape) {
            // TODO
        }

        // find property by token name

        // push it to stack.
    }

    void value_token_begin() {
        _token.begin_index = _out.size();
        _token.has_escape  = false;
    }

    void value_token_end() {
        auto token = u8str_view{_out}.substr(_token.begin_index);

        // access to topmost property of _context, then pop it.
    }

private:
    enum class _context_state {
        none, // 루트 오브젝트에만 할당
        property_attributes,
        property_value,
        attribute_value,
        skipping,
    };

    struct _parser_context {
        object_baseaddr_t* addr = {};
        _context_state state    = _context_state::none;
        union {
            property const* prop;
            property::attribute const* attr;
        } ptr = {};
    };

private:
    std::vector<_nextc::type> _st = {};
    object& _dest_root;
    u8str& _out;
    u8str _buf_if_not_specified;

    // used for parsing only for current argument.
    // detailed information will be represented as parser context.
    std::vector<_parser_context> _context;

    struct {
        size_t begin_index = ~size_t{0};
        bool has_escape    = false;
    } _token;
};

inline json_parser_stream::json_parser_stream(object& root, u8str* buf)
  : _dest_root(root), _out(buf ? *buf : _buf_if_not_specified) {
    if (!buf) { _buf_if_not_specified.reserve(64); }
    _st.reserve(32);
    _st.push_back(_nextc::opening_brace);
    _context.reserve(32);
    _context.push_back({root.base(), _context_state::none});
}

inline json_parser_stream::result_t json_parser_stream::operator()(char ch) {
    if (_st.empty()) { return error; }
    using namespace utils;

    switch (auto state = static_cast<std::underlying_type_t<_nextc::type>>(_st.back())) {
            // TODO: 코멘트 처리 추가 ... 모든 섹션에 '/' 처리 루틴
        case _nextc::opening_brace:
            if (ch == '/') {
                return push(_nextc::begin_comment_next), ready;
            } else if (ch == '{') {
                apnd(ch), replace(_nextc::closing_brace);
                return ready;
            } else if (spacechars(ch)) {
                return ready;
            } else {
                return error;
            }

        case _nextc::comma_or_closing_brace:
            if (ch == ',') {
                apnd(ch),
                  push(_nextc::value_begin),
                  push(_nextc::tag_colon),
                  push(_nextc::string_closing_quote),
                  push(_nextc::tag_opening_quote); // tag must follow after comma
                return ready;
            }
            [[fallthrough]];
        case _nextc::closing_brace:
            if (ch == '}') {
                _context.pop_back(); // Finalize parsing context.
                apnd(ch), pop();
                return _st.empty() ? done : ready;
            } else if (ch == '/') {
                return push(_nextc::begin_comment_next), ready;
            } else if (spacechars(ch)) {
                return ready;
            } else if (ch == '"') {
                // beginning of the first-most tag
                tag_token_begin();
                apnd(ch);

                replace(_nextc::comma_or_closing_brace); // can look for next value after all.
                push(_nextc::value_begin);
                push(_nextc::tag_colon);
                push(_nextc::string_closing_quote);
                return ready;
            } else {
                return error;
            }

        case _nextc::string_closing_quote:
            if (ch == '\\') {
                _token.has_escape = true;
                return apnd(ch), push(_nextc::string_escaped_next), ready;
            } else if (ch == '"') {
                pop();

                if (_st.back() == _nextc::tag_colon) {
                    tag_token_end();
                } else {
                    value_token_end();
                }

                return apnd(ch), ready;
            } else if (!iscntrl(ch)) {
                return apnd(ch), ready;
            } else {
                return error;
            }

        case _nextc::tag_opening_quote:
            if (ch == '"') {
                tag_token_begin();
                return apnd(ch), pop(), ready;
            } else if (ch == '/') {
                return push(_nextc::begin_comment_next), ready;
            } else if (spacechars(ch)) {
                return ready;
            }

        case _nextc::tag_colon:
            if (ch == ':') {
                return apnd(ch), pop(), ready;
            } else if (spacechars(ch)) {
                return ready;
            } else {
                return error;
            }

        case _nextc::comma_or_closing_bracket:
            if (ch == ',') {
                return apnd(ch), push(_nextc::value_begin), ready;
            }
            [[fallthrough]];
        case _nextc::closing_bracket:
            if (ch == ']') {
                return apnd(ch), pop(), ready;
            } else if (spacechars(ch)) {
                return ready;
            } else if (ch == '/') {
                return push(_nextc::begin_comment_next), ready;
            } else if (one_of("[\"{tfn", ch) || digit(ch)) {
                replace(_nextc::comma_or_closing_bracket);
                push(_nextc::value_begin);
                return (*this)(ch);
            } else {
                return error;
            }

        case _nextc::value_begin:
            if (spacechars(ch)) {
                return ready;
            } else if (ch == '/') {
                return push(_nextc::begin_comment_next), ready;
            } else if (ch == '{') {
                // TODO: Verify whether current property is object
                return apnd(ch), replace(_nextc::closing_brace), ready;
            } else if (ch == '[') {
                // TODO: Verify whether current property is array
                return apnd(ch), replace(_nextc::closing_bracket), ready;
            } else if (ch == '"') {
                value_token_begin();
                return apnd(ch), replace(_nextc::string_closing_quote), ready;
            } else if (one_of("tfn", ch) || digit(ch)) {
                value_token_begin();
                return apnd(ch), replace(_nextc::value_end), ready;
            } else {
                return error;
            }

        case _nextc::value_end:
            if (ch == '.' || alphanumeric(ch)) {
                return apnd(ch), ready;
            } else if (ch == '/') {
                return push(_nextc::begin_comment_next), ready;
            } else if (spacechars(ch)) {
                value_token_end();
                return pop(), ready;
            } else if (one_of(",}]", ch)) {
                value_token_end();
                return pop(), (*this)(ch);
            } else {
                return error;
            }

        case _nextc::string_escaped_next:
            if (ch == 'u') {
                return apnd(ch), replace(_nextc::string_escape_u_begin), ready;
            } else if (one_of("\\/\"bfrnt", ch)) {
                return apnd(ch), pop(), ready; // finishes escape
            } else {
                return error;
            }

        case _nextc::string_escape_u_begin + 0:
        case _nextc::string_escape_u_begin + 1:
        case _nextc::string_escape_u_begin + 2:
        case _nextc::string_escape_u_begin + 3:
            if (hex(ch)) {
                return apnd(ch), replace(static_cast<_nextc::type>(state + 1)), ready;
            } else {
                return error;
            }

        case _nextc::string_escape_u_end:
            return pop(), (*this)(ch);

        case _nextc::begin_comment_next:
            if (ch == '/') {
                return replace(_nextc::line_comment_newline), ready;
            } else if (ch == '*') {
                return replace(_nextc::block_comment_0_asterisk), ready;
            } else {
                return error;
            }

        case _nextc::line_comment_newline:
            if (ch == '\n') {
                return pop(), ready;
            } else {
                return ready;
            }

        case _nextc::block_comment_0_asterisk:
            if (ch == '*') {
                return replace(_nextc::block_comment_1_slash), ready;
            } else {
                return ready;
            }

        case _nextc::block_comment_1_slash:
            if (ch == '/') {
                return pop(), ready;
            } else {
                return replace(_nextc::block_comment_0_asterisk), ready;
            }
        default:; // nop
    }

    return ready;
}
} // namespace kangsw::refl::marshal