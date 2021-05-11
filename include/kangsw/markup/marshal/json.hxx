#pragma once
#include "../reflection/property_proxy.hxx"
#include "generics.hxx"

namespace kangsw::refl::marshal {

/**  */
class json_parse_context {
public:
    /** Input parsing status */
    enum status_type {
        status_ok   = 0,
        status_done = 1,

        status_error = std::numeric_limits<std::underlying_type_t<status_type>>::min(),
    };

public:
    /** Initialize json parsing context */
    json_parse_context(object& dest) : _dest(dest) {}

    /** Parses input stream sequentially. Returns true if next input is available */
    bool operator()(char ch);

    /** Get current status of json context */
    status_type status() const;

private:
    object& _dest;
};

/** Json dump can be done in statelessly. */
class json_dump {
public:
    void operator()(object const& obj, string_output& o);

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
}

template <>
inline void _dump<object>(object const& v, string_output& o) {
    o << '{', ++o; // Write value first -> indent later

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
                visit_property(v, attr, json_dump::_visitor{o});

                size_t attr_idx = &attr - prop.attributes().data();
                if (attr_idx + 1 < prop.attributes().size()) { o << ','; }
            }

            --o;
            o << break_indent << "}," << break_indent;
        }

        o.wrap('"', prop.tag()) << ": {";
        visit_property(v, prop, json_dump::_visitor{o});

        size_t prop_idx = &prop - v.properties().data();
        if (prop_idx + 1 < v.properties().size()) { o << ','; }
    }

    --o, o << break_indent << '}';
}

template <typename Ty_>
void _dump(property_proxy<std::vector<Ty_>, true> v, string_output& o) {
}

template <typename Ty_>
void _dump(property_proxy<u8str_map<Ty_>, true> v, string_output& o) {
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

inline void json_dump::operator()(object const& obj, string_output& o) {
    json::_dump(obj, o);
    o << break_indent;
}
} // namespace kangsw::refl::marshal