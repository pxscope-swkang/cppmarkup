#pragma once
#include "strutils.hxx"
#include "generics.hxx"
#include "trivial_marshal.hxx"

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
    json_parser_stream(u8str& out) : _out(out) {
        _st.reserve(32);
        _st.push_back(_nextc::opening_brace);
        _out.clear();
    }

    result_t operator()(char ch);

private:
    void pop() { _st.pop_back(); }
    void push(_nextc::type c) { _st.push_back(c); }
    void replace(_nextc::type c) { pop(), push(c); }
    void apnd(char ch) { _out += ch; }

private:
    u8str& _out;
    std::vector<_nextc::type> _st = {};
};

inline json_parser_stream::result_t json_parser_stream::operator()(char ch) {
    if (_st.empty()) { return error; }
    using namespace utils;

    switch (auto state = static_cast<std::underlying_type_t<_nextc::type>>(_st.back())) {
            // TODO: 内膏飘 贸府 眠啊 ... 葛电 冀记俊 '/' 贸府 风凭
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
                  push(_nextc::tag_colon),
                  push(_nextc::string_closing_quote),
                  push(_nextc::tag_opening_quote); // tag must follow after comma
                return ready;
            }
            [[fallthrough]];
        case _nextc::closing_brace:
            if (ch == '}') {
                apnd(ch), pop();
                return _st.empty() ? done : ready;
            } else if (ch == '/') {
                return push(_nextc::begin_comment_next), ready;
            } else if (spacechars(ch)) {
                return ready;
            } else if (ch == '"') {
                // beginning of first-most tag
                apnd(ch);
                replace(_nextc::comma_or_closing_brace); // can look for next value after all.
                push(_nextc::tag_colon);
                push(_nextc::string_closing_quote);
                return ready;
            } else {
                return error;
            }

        case _nextc::string_closing_quote:
            if (ch == '\\') {
                // escape sequence ... does not append
                return apnd(ch), push(_nextc::string_escaped_next), ready;
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
            } else if (ch == '/') {
                return push(_nextc::begin_comment_next), ready;
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
            } else if (ch == '{') {
                apnd(ch),
                  replace(_nextc::comma_or_closing_bracket),
                  push(_nextc::closing_brace);
                return ready;
            } else if (ch == '[') {
                apnd(ch),
                  replace(_nextc::comma_or_closing_bracket),
                  push(_nextc::closing_bracket);
                return ready;
            } else if (ch == '"') {
                apnd(ch),
                  replace(_nextc::comma_or_closing_bracket),
                  push(_nextc::string_closing_quote);
                return ready;
            } else if (one_of("tfn", ch) || digit(ch)) {
                apnd(ch),
                  replace(_nextc::comma_or_closing_bracket),
                  push(_nextc::value_end);
                return ready;
            } else {
                return error;
            }

        case _nextc::value_begin:
            if (spacechars(ch)) {
                return ready;
            } else if (ch == '/') {
                return push(_nextc::begin_comment_next), ready;
            } else if (ch == '{') {
                return apnd(ch), replace(_nextc::closing_brace), ready;
            } else if (ch == '[') {
                return apnd(ch), replace(_nextc::closing_bracket), ready;
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
            } else if (ch == '/') {
                return push(_nextc::begin_comment_next), ready;
            } else if (spacechars(ch)) {
                return pop(), ready;
            } else if (one_of(",}]", ch)) {
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
        default: ; // nop
    }

    return ready;
}
} // namespace kangsw::refl::marshal