#include "marshal_json.hpp"
#include "object.hpp"
#include <iomanip>
#include <iostream>

using namespace kangsw::markup;

////////////////////////////////////////////////////////////////////////////////////////// DUMPING

namespace {
/** TO_STRING과 유사한 역할 수행 */
template <typename Ty_> void to_json_value(json_dump& to, Ty_ const& v);
template <> void to_json_value(json_dump& to, bool const& v) { to.dest << (v ? u8"true" : u8"false"); }
template <> void to_json_value(json_dump& to, kangsw::markup::u8string const& v) { to.dest << u8'"' << v << u8'"'; }
template <> void to_json_value(json_dump& to, kangsw::markup::object const& v) { dump(to, v); }

enum class indent_dir {
    apply,
    conf_fwd,
    conf_bwd
};

void break_indent(json_dump& to, indent_dir dir)
{
    if (to.indent <= 0) { return; }

    switch (dir) {
        case indent_dir::apply:
            to.dest << u8'\n'
                    << std::setw(to.initial_indent) << u8"";
            break;
        case indent_dir::conf_fwd: to.initial_indent += to.indent; break;
        case indent_dir::conf_bwd: to.initial_indent -= to.indent; break;
        default: throw;
    }
}

template <typename Ty_> void to_json_value(json_dump& to, Ty_ const& v)
{
    if constexpr (kangsw::templates::is_specialization<Ty_, std::vector>::value) {
        to.dest << u8'[';
        break_indent(to, indent_dir::conf_fwd);

        for (size_t i = 0, iend = v.size(); i < iend; ++i) {
            break_indent(to, indent_dir::apply);
            to_json_value(to, v[i]);

            if (i + 1 < iend) { to.dest << u8','; }
        }

        break_indent(to, indent_dir::conf_bwd);
        break_indent(to, indent_dir::apply);
        to.dest << u8']';
    }
    else if constexpr (kangsw::templates::is_specialization<Ty_, std::map>::value) {
        // TODO
    }
    else {
        to.dest << v;
    }
}

void to_json_value_from_object(json_dump& to, void const* m, property const& prop)
{
    if (prop.type.is_array()) {
        auto fn = prop.as_array();
        assert(fn);

        to.dest << u8'[';
        break_indent(to, indent_dir::conf_fwd);

        for (size_t i = 0, iend = fn->size(m); i < iend; ++i) {
            break_indent(to, indent_dir::apply);

            to_json_value(to, *fn->at(m, i));
            if (i + 1 < iend) { to.dest << u8','; }
        }

        break_indent(to, indent_dir::conf_bwd);
        break_indent(to, indent_dir::apply);
        to.dest << u8']';
    }
    else if (prop.type.is_map()) {
        // TODO
    }
    else {
        to_json_value(to, *(object const*)m);
    }
}
} // namespace

#define DO_GENERATE(ty)                                      \
    if (prop.type.is_array())                                \
        to_json_value(to, *(std::vector<ty>*)memory);        \
    else if (prop.type.is_map())                             \
        to_json_value(to, *(std::map<u8string, ty>*)memory); \
    else                                                     \
        to_json_value(to, *(ty*)memory);                     \
    break;

kangsw::markup::marshalerr_t
kangsw::markup::dump(json_dump& to, object const& from)
{
    auto& props = from.props();
    auto& out   = to.dest;
    out << u8'{';
    break_indent(to, indent_dir::conf_fwd);

    for (int i = 0; i < props.size(); ++i) {
        auto& prop         = props[i];
        void const* memory = (char*)&from + prop.memory.elem_offset + prop.memory.value_offset;

        break_indent(to, indent_dir::apply);
        out << u8'"' << prop.tag << u8"\": ";

        // 오브젝트 타입별로 다른 동작
        switch (prop.type.value_type()) {
            case element_type::null: out << u8"null"; break;
            case element_type::boolean: DO_GENERATE(bool);
            case element_type::integer: DO_GENERATE(int64_t);
            case element_type::floating_point: DO_GENERATE(double);
            case element_type::string: DO_GENERATE(u8string);
            case element_type::object: to_json_value_from_object(to, memory, prop); break;
            default:;
        }

        if (i + 1 < props.size()) { out << u8','; }
    }

    break_indent(to, indent_dir::conf_bwd);
    break_indent(to, indent_dir::apply);
    out << u8'}';
    break_indent(to, indent_dir::apply);
    return marshalerr_t::ok;
}

////////////////////////////////////////////////////////////////////////////////////////// PARSING

kangsw::markup::marshalerr_t
kangsw::markup::parse(object& to, json_parse const& from)
{

    return marshalerr_t::fail;
}