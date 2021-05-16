#pragma once
#include "strutils.hxx"
#include "generics.hxx"
#include "trivial_marshal.hxx"
#include "../../reflection/property_proxy.hxx"

namespace kangsw::refl::marshal {
/** Dump given object as json */
class json_dump {
public:
    void operator()(object const& obj, string_output o);

    struct _visitor {
        template <typename Ty_> void operator()(property_proxy<Ty_, true> p);
        string_output& o;
    };
};


namespace Impl {
template <typename Ty_>
void _dump(Ty_ const& v, string_output& o) {
    auto constexpr T = etype::from_type<Ty_>();
    using E          = etype::_type;

    if constexpr (T.is_one_of(E::floating_point, E::integer, E::boolean, E::null)) {
        generic_stringfy<Ty_>{}(v, std::back_inserter(o.str()));
    } else if constexpr (T.is_one_of(etype::timestamp, etype::binary)) {
        o << '"';
        generic_stringfy<Ty_>{}(v, std::back_inserter(o.str()));
        o << '"';
    } else if constexpr (T.is_string()) {
        o << '"';
        for (char ch : v) {
            // Handle escape
            if (iscntrl(ch)) {
                switch (ch) {
                    case '"': o << "\\\""; break;
                    case '\\': o << "\\\\"; break;
                    case '\b': o << "\\b"; break;
                    case '\f': o << "\\f"; break;
                    case '\n': o << "\\n"; break;
                    case '\r': o << "\\r"; break;
                    case '\t': o << "\\t"; break;
                    default:
                        if ('\x00' <= ch && ch <= '\x1f') {
                            o << "\\u";
                            char buf[5];
                            snprintf(buf, sizeof buf, "%04x", ch), buf[4] = 0;
                            o << buf;
                        } else {
                            o << ch;
                        }
                }
            } else {
                o.str() += ch;
            }
        }
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

    --o, o << break_indent << '}';
}

} // namespace Impl

template <typename Ty_>
void json_dump::_visitor::operator()(property_proxy<Ty_, true> p) {
    auto constexpr T = etype::from_type<Ty_>();
    if constexpr (!T.is_container()) {
        Impl::_dump(*p, o);
    } else {
        Impl::_dump(p, o);
    }
}

inline void json_dump::operator()(object const& obj, string_output o) {
    Impl::_dump(obj, o);
    o << break_indent;
}

} // namespace kangsw::refl::marshal