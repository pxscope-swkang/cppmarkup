#include "marshal_json.hpp"
#include "base64.hxx"
#include "object.hpp"
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>

using namespace kangsw::markup;

////////////////////////////////////////////////////////////////////////////////////////// DUMPING

namespace {
/** TO_STRING과 유사한 역할 수행 */
template <typename Ty_> void to_json_value(json_dump& to, Ty_ const& v);
template <> void to_json_value(json_dump& to, bool const& v) { to.dest << (v ? "true" : "false"); }
template <> void to_json_value(json_dump& to, kangsw::markup::u8string const& v) { to.dest << '"' << v << '"'; }

template <> void to_json_value(json_dump& to, binary_chunk const& v)
{
    to.dest << '"';
    // base64 encoding
    kangsw::base64::encode(v.data.data(), v.data.size(), std::ostream_iterator<char>{to.dest});
    to.dest << '"';
}

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
            to.dest << '\n'
                    << std::setw(to.initial_indent) << "";
            break;
        case indent_dir::conf_fwd: to.initial_indent += to.indent; break;
        case indent_dir::conf_bwd: to.initial_indent -= to.indent; break;
        default: throw;
    }
}

void to_json_value_from_object(json_dump& to, void const* m, property const& prop);

#define DO_GENERATE(ty)                                      \
    if (prop.type.is_array())                                \
        to_json_value(to, *(std::vector<ty>*)memory);        \
    else if (prop.type.is_map())                             \
        to_json_value(to, *(std::map<u8string, ty>*)memory); \
    else                                                     \
        to_json_value(to, *(ty*)memory);                     \
    break;

template <> void to_json_value(json_dump& to, kangsw::markup::object const& from)
{
    auto& props = from.props();
    auto& out   = to.dest;
    out << '{';
    break_indent(to, indent_dir::conf_fwd);

    for (int i = 0; i < props.size(); ++i) {
        auto& prop         = props[i];
        void const* memory = (char*)&from + prop.memory.elem_offset + prop.memory.value_offset;

        // TODO: attribute는 값보다 먼저 덤프 ...
        for (auto& attr : prop.attributes) {
            auto attr_memory = attr.get(memory);
        }

        break_indent(to, indent_dir::apply);
        out << '"' << prop.tag << "\": ";

        // 오브젝트 타입별로 다른 동작
        switch (prop.type.value_type()) {
            case element_type::null: out << "null"; break;
            case element_type::boolean: DO_GENERATE(bool);
            case element_type::integer: DO_GENERATE(int64_t);
            case element_type::floating_point: DO_GENERATE(double);
            case element_type::string: DO_GENERATE(u8string);
            case element_type::binary: DO_GENERATE(binary_chunk);
            case element_type::object: to_json_value_from_object(to, memory, prop); break;
            default:;
        }

        if (i + 1 < props.size()) { out << ','; }
    }

    break_indent(to, indent_dir::conf_bwd);
    break_indent(to, indent_dir::apply);
    out << '}';
}
#undef DO_GENERATE

template <typename Ty_> void to_json_value(json_dump& to, Ty_ const& v)
{
    if constexpr (kangsw::templates::is_specialization<Ty_, std::vector>::value) {
        to.dest << '[';
        break_indent(to, indent_dir::conf_fwd);

        for (size_t i = 0, iend = v.size(); i < iend; ++i) {
            break_indent(to, indent_dir::apply);
            to_json_value(to, v[i]);

            if (i + 1 < iend) { to.dest << ','; }
        }

        break_indent(to, indent_dir::conf_bwd);
        break_indent(to, indent_dir::apply);
        to.dest << ']';
    }
    else if constexpr (kangsw::templates::is_specialization<Ty_, std::map>::value) {
        // TODO map 처리
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

        to.dest << '[';
        break_indent(to, indent_dir::conf_fwd);

        for (size_t i = 0, iend = fn->size(m); i < iend; ++i) {
            break_indent(to, indent_dir::apply);

            to_json_value(to, *fn->at(m, i));
            if (i + 1 < iend) { to.dest << ','; }
        }

        break_indent(to, indent_dir::conf_bwd);
        break_indent(to, indent_dir::apply);
        to.dest << ']';
    }
    else if (prop.type.is_map()) {
        // TODO: map 처리
    }
    else {
        to_json_value(to, *(object const*)m);
    }
}

} // namespace

kangsw::markup::marshalerr_t
kangsw::markup::dump(json_dump& to, object const& from)
{
    to_json_value(to, from);
    break_indent(to, indent_dir::apply);
    return marshalerr_t::ok;
}

////////////////////////////////////////////////////////////////////////////////////////// PARSING

/**
 * @note. 파싱 로직은 입력 string_view를 상태 머신으로 활용...
 *         입력받은 u8string_view를 문자열을 파싱한 만큼 전진시켜, 새로운 substr로 갱신
 */
namespace {
u8string_view constexpr spacer_chars = " \t\n\r";

struct parser_context {
    std::match_results<u8string_view::const_iterator> _match_result_memory;
};

size_t find_initial_char(u8string_view const& src, u8string_view const& allowed, u8string_view const& find_any)
{
    for (auto& c : src) {
        if (allowed.find(c) != allowed.npos) { continue; }
        if (find_any.find(c) != find_any.npos) { return static_cast<size_t>(&c - &src.front()); }
        break;
    }

    return src.npos;
}

template <typename Ty_>
marshalerr_t parse_array(parser_context& context, Ty_& to, u8string_view& from)
{
    return {};
}

template <typename Ty_>
marshalerr_t parse_value(parser_context& context, Ty_& to, u8string_view& from)
{ // 모든 파서는 원본 스트링 뷰를 갱신함.
    return {};
}

template <>
marshalerr_t parse_value(parser_context& context, u8string& to, u8string_view& from)
{
    return {};
}

marshalerr_t parse_memory(parser_context& context, element_type const& ty, void* memory, u8string_view& ss)
{
#define PEWPEW(type)                                           \
    if (ty.is_array())                                         \
        parse_array(context, *(std::vector<type>*)memory, ss); \
    else                                                       \
        parse_value(context, *(type*)memory, ss);              \
    break

    // clang-format off
    switch (ty.value_type()) {
        case element_type::null: break;
        case element_type::boolean:         PEWPEW(bool);
        case element_type::integer:         PEWPEW(int64_t);
        case element_type::floating_point:  PEWPEW(double);
        case element_type::string:          PEWPEW(u8string);
        case element_type::binary:          PEWPEW(binary_chunk);
        case element_type::object:
            // TODO: object는 따로 처리 
        default:;
    }
    // clang-format on

    return marshalerr_t::ok;
}

template <>
marshalerr_t parse_value(parser_context& context, object& to, u8string_view& ss)
{ // object 버전 특수화 ...
    auto constexpr npos = u8string_view::npos;

    // 1. 여는 중괄호
    if (auto init_br_pos = find_initial_char(ss, spacer_chars, "{");
        init_br_pos != ss.npos) //
    {
        ss = ss.substr(init_br_pos);
    }
    else {
        return marshalerr_t::invalid_format;
    }

    for (bool do_continue = true; do_continue;) {
        static const std::regex regex_tag{R"_(\s*"((?:[^"\\]|\\.)*)"\s*:)_"};
        auto& result = context._match_result_memory;

        // 2. 태그 찾기
        if (!std::regex_search(ss.begin(), ss.end(), result, regex_tag)) { break; }

        // 태그 발견 시 원본 문자열 밀기
        ss       = ss.substr(result.length());
        auto tag = u8string_view{&*result[1].first, (size_t)result[1].length()}; // 1번 그룹에 문자열 저장됨

        // 콤마 찾기 ...
        auto comma_pos = find_initial_char(ss, spacer_chars, ",");
        if (comma_pos == npos) {
            // 콤마 없으면 다음 엘리먼트에 이탈
            do_continue = false;
        }
        else {
            // 콤마 있으면 계속 ...
            ss = ss.substr(comma_pos + 1);
        }

        // 2.5. 태그가 Attribute인지 판별, Attribute이면 ~@@ATTRS@@ suffix 떼고 태그명 검색 후 어트리뷰트 파싱
        auto attr_suffix_pos = tag.rfind("~@@ATTRS@@");
        if (attr_suffix_pos != npos) {
            tag        = tag.substr(attr_suffix_pos);
            auto pprop = to.find_property(tag);
            if (pprop == nullptr) { continue; } // 없으면 그냥 무시

            auto prop_memory = pprop->get(&to);
            auto& attrs      = pprop->attributes;

            // 2.5.1 각 어트리뷰트에 대해 파서 돌리기
            for (auto& attr : attrs) {
                auto memory = attr.get(prop_memory);

                auto parse_result = parse_memory(context, pprop->type, memory, ss);
                if (parse_result != marshalerr_t::ok) { return parse_result; }
            }
        }
        else { // attr suffix 못 찾으면 그냥 태그임 ...
            // 3. 태그에 대응되는 프로퍼티 찾기
            auto pprop = to.find_property(tag);
            if (pprop == nullptr) { continue; } // 없으면 그냥 무시
            auto memory = pprop->get(&to);

            auto parse_result = parse_memory(context, pprop->type, memory, ss);
            if (parse_result != marshalerr_t::ok) { return parse_result; }
        }
    }

    // 5. 닫는 중괄호 찾기
    return marshalerr_t::ok;
}

} // namespace

kangsw::markup::marshalerr_t
kangsw::markup::parse(object& to, json_parse const& from)
{
    // 앞에서부터 캐릭터 하나씩 iterate ...
    // 푸시다운 오토마타로 구현 ...
    auto view = from.source;
    parser_context context;
    return parse_value(context, to, view);
}