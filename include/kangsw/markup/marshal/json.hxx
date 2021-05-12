#pragma once
#include "../reflection/property_proxy.hxx"
#include "generics.hxx"

namespace kangsw::refl::marshal {

/**  */
class json_parse {
public:
    /** Input parsing status */
    enum status_type {
        status_ok   = 0,
        status_done = 1,

        status_error = std::numeric_limits<std::underlying_type_t<status_type>>::min(),
    };

public:
    /** Initialize json parsing context */
    json_parse(object& dest) : _dest(dest) {}

    /** Parses input stream sequentially. Returns true if next input is available */
    bool operator()(char ch);

    template <typename It_>
    bool operator()(It_ begin, It_ end) {
        for (; begin != end; ++begin) {
            if (!this->operator()(*begin)) { return false; }
        }
    }

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
    auto constexpr T = etype::from_type<Ty_>();
    using E          = etype::_type;

    if constexpr (T.is_one_of(E::floating_point, E::integer, E::boolean, E::null)) {
        generic_stringfy<Ty_>{}(v, std::back_inserter(o.out));
    } else if constexpr (T.is_one_of(etype::string, etype::timestamp, etype::binary)) {
        o << '"';
        generic_stringfy<Ty_>{}(v, std::back_inserter(o.out));
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

inline void json_dump::operator()(object const& obj, string_output& o) {
    json::_dump(obj, o);
    o << break_indent;
}
} // namespace kangsw::refl::marshal