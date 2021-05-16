#pragma once
#include "strutils.hxx"
#include "generics.hxx"
#include "trivial_marshal.hxx"
#include "../../reflection/property_proxy.hxx"

namespace kangsw::refl::marshal {

/** Parse string as the schema of given object. */
class json_parse {
public:
    /** Type of error */
    enum class error_type {
        ok,
        invalid_syntax,
        schema_unmatched,
    };

    /** Information of errors or warnings */
    struct error_summary {
        size_t index;
        error_type type;
    };

    /** Parse input json string into object. Returns nothing when succeeded. */
    std::optional<error_summary> operator()(object& dest, u8str_view i) const;

private:
    struct _visitor {
        template <typename Ty_> error_type operator()(property_proxy<Ty_, false> p);
        u8str_view& i;
    };
};

// //////////////////////////////////// IMPLEMENTATION ////////////////////////////////////  //
// //////////////////////////////////// IMPLEMENTATION ////////////////////////////////////  //

namespace Impl {
template <typename Ty_>
json_parse::error_type _parse(Ty_& dest, u8str_view& in) {
    return json_parse::error_type::ok;
}

template <>
inline json_parse::error_type _parse(object& dest, u8str_view& in) {
    using namespace utils;
    if (auto [syntax_valid, position] =
          find_until_match(in, matcher('{'), spacechars);
        syntax_valid) {
        in = in.substr(position + 1);
    } else {
        return json_parse::error_type::invalid_syntax;
    }

    for (;;) {
        // find tag-begin
        if (auto [quote_found, position] =
              find_until_match(in, matcher('"'), spacechars);
            !quote_found) {
            if (position >= in.size()) {
                return json_parse::error_type::invalid_syntax;
            }
            if (in[position] == '}') {
                // object match done.
                in = in.substr(position + 1);
                return json_parse::error_type::ok;
            }
        } else {
            in = in.substr(position + 1);
        }

        // find tag-end
        auto [tag_end_quote_found, quote_pos] =
          find_until_match(in, escape_handling_matcher('"'));

        if (!tag_end_quote_found) {
            return json_parse::error_type::invalid_syntax;
        }

        // if tag is suffixed with '~@@ATTR@@', remove it and find tag.
        auto tag_raw = in.substr(0, quote_pos);

        // find property by tag name.
    }

    return json_parse::error_type::ok;
}

template <typename Ty_>
json_parse::error_type _parse(property_proxy<std::vector<Ty_>, false>& dest, u8str_view& in) {
    return json_parse::error_type::ok;
}

template <typename Ty_>
json_parse::error_type _parse(property_proxy<u8str_map<Ty_>, false>& dest, u8str_view& in) {
    return json_parse::error_type::ok;
}
} // namespace Impl

template <typename Ty_>
json_parse::error_type json_parse::_visitor::operator()(property_proxy<Ty_, false> p) {
    if constexpr (p.type().is_container()) {
        return Impl::_parse(p, this->i);
    } else {
        return Impl::_parse(*p, this->i);
    }
}

inline std::optional<json_parse::error_summary> json_parse::operator()(object& dest, u8str_view i) const {
    auto const init = i.data();
    if (error_type result = Impl::_parse(dest, i);
        result != error_type::ok) //
    {
        error_summary error;
        error.type  = result;
        error.index = i.data() - init;
        return error;
    } else {
        return {};
    }
}

} // namespace kangsw::refl::marshal