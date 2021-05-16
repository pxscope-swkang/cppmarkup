#pragma once
#include <optional>
#include "../../types.hxx"

namespace kangsw::refl::marshal {

/**
 * Generic string output
 */
struct indent_t {};
constexpr inline static indent_t break_indent;

class string_output {
public:
    string_output(u8str& str, int indent_width = -1, int initial_indent = 0)
      : _out(&str), _indent_width(indent_width), _indent_init(initial_indent) {}

    template <typename Ty_>
    string_output& operator<<(Ty_&& other) { return *_out += std::forward<Ty_>(other), *this; }
    string_output& operator<<(indent_t) { return _break_indent(), *this; }

    template <typename Wrap_, typename... Ty_>
    string_output& wrap(Wrap_&& s, Ty_&&... other) { return *this << s, ((*this << std::forward<Ty_>(other)), ...) << std::forward<Wrap_>(s); }

    auto& str() const { return *_out; }
    auto& str() { return *_out; }

    void operator++() { _conf_indent_f(); }
    void operator--() { _conf_indent_b(); }

private:
    void _break_indent();
    void _conf_indent_f() { _indent_init += _indent_width; }
    void _conf_indent_b() { _indent_init -= _indent_width; }

private:
    u8str* _out;
    int _indent_width = -1;
    int _indent_init  = 0;
};

inline void string_output::_break_indent() {
    if (_indent_width < 0) { return; }

    *_out += '\n';
    if (_indent_width > 0) { _out->append(_indent_init, ' '); }
}

} // namespace kangsw::refl::marshal
