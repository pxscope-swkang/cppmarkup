#include "marshal_json.hpp"
#include "object.hpp"
#include "base64.hxx"
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>

using namespace kangsw::markup;

////////////////////////////////////////////////////////////////////////////////////////// DUMPING

namespace {
/** TO_STRING과 유사한 역할 수행 */
template <typename Ty_> void to_json_value(json_dump& to, Ty_ const& v);
template <> void to_json_value(json_dump& to, bool const& v) { to.dest << (v ? u8"true" : u8"false"); }
template <> void to_json_value(json_dump& to, kangsw::markup::u8string const& v) { to.dest << u8'"' << v << u8'"'; }

template <> void to_json_value(json_dump& to, binary_chunk const& v)
{
    to.dest << u8'"';
    // base64 encoding
    kangsw::base64::encode(v.data.data(), v.data.size(), std::ostream_iterator<char>{to.dest});
    to.dest << u8'"';
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
            to.dest << u8'\n'
                    << std::setw(to.initial_indent) << u8"";
            break;
        case indent_dir::conf_fwd: to.initial_indent += to.indent; break;
        case indent_dir::conf_bwd: to.initial_indent -= to.indent; break;
        default: throw;
    }
}

#define DO_GENERATE(ty)                                      \
    if (prop.type.is_array())                                \
        to_json_value(to, *(std::vector<ty>*)memory);        \
    else if (prop.type.is_map())                             \
        to_json_value(to, *(std::map<u8string, ty>*)memory); \
    else                                                     \
        to_json_value(to, *(ty*)memory);                     \
    break;

void to_json_value_from_object(json_dump& to, void const* m, property const& prop);

template <> void to_json_value(json_dump& to, kangsw::markup::object const& from)
{
    auto& props = from.props();
    auto& out   = to.dest;
    out << u8'{';
    break_indent(to, indent_dir::conf_fwd);

    for (int i = 0; i < props.size(); ++i) {
        auto& prop         = props[i];
        void const* memory = (char*)&from + prop.memory.elem_offset + prop.memory.value_offset;

        // TODO: attribute는 값보다 먼저 덤프 ...
        for (auto& attr : prop.attributes) {
            auto attr_memory = attr.get(memory);
        }

        break_indent(to, indent_dir::apply);
        out << u8'"' << prop.tag << u8"\": ";

        // 오브젝트 타입별로 다른 동작
        switch (prop.type.value_type()) {
            case element_type::null: out << u8"null"; break;
            case element_type::boolean: DO_GENERATE(bool);
            case element_type::integer: DO_GENERATE(int64_t);
            case element_type::floating_point: DO_GENERATE(double);
            case element_type::string: DO_GENERATE(u8string);
            case element_type::binary: DO_GENERATE(binary_chunk);
            case element_type::object: to_json_value_from_object(to, memory, prop); break;
            default:;
        }

        if (i + 1 < props.size()) { out << u8','; }
    }

    break_indent(to, indent_dir::conf_bwd);
    break_indent(to, indent_dir::apply);
    out << u8'}';
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
u8string_view constexpr spacer_chars = u8" \t\n\r";

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

#undef DO_GENERATE
#define DO_GENERATE(Ty_)

template <>
marshalerr_t parse_value(parser_context& context, object& to, u8string_view& ss)
{ // object 버전 특수화 ...
    auto constexpr npos = u8string_view::npos;

    // 1. 여는 중괄호
    if (auto init_br_pos = find_initial_char(ss, spacer_chars, u8"{");
        init_br_pos != ss.npos) //
    {
        ss = ss.substr(init_br_pos);
    }
    else {
        return marshalerr_t::invalid_format;
    }

    for (bool do_continue = true; do_continue;) {
        static const std::regex regex_tag{u8R"_(\s*"((?:[^"\\]|\\.)*)"\s*:)_"};
        auto& result = context._match_result_memory;

        // 2. 태그 찾기
        if (!std::regex_search(ss.begin(), ss.end(), result, regex_tag)) { break; }

        // 태그 발견 시 원본 문자열 밀기
        ss       = ss.substr(result.length());
        auto tag = u8string_view{&*result[1].first, (size_t)result[1].length()}; // 1번 그룹에 문자열 저장됨

        // 콤마 찾기 ...
        auto comma_pos = find_initial_char(ss, spacer_chars, u8",");
        if (comma_pos == npos) {
            // 콤마 없으면 다음 엘리먼트에 이탈
            do_continue = false;
        }
        else {
            // 콤마 있으면 계속 ...
            ss = ss.substr(comma_pos + 1);
        }

        // 3. 태그에 대응되는 프로퍼티 찾기
        auto pprop = to.find_property(tag);
        if (pprop == nullptr) { continue; } // 없으면 그냥 무시

        // 4. 해당 엘리먼트에 대해 파서 돌리기
        switch (pprop->type.value_type()) {
            case element_type::null: break;
            case element_type::boolean: break;
            case element_type::integer: break;
            case element_type::floating_point: break;
            case element_type::string: break;
            case element_type::object: break;
            default:;
        }
    }

    // 5. 닫는 중괄호 찾기
    return marshalerr_t::ok;
}

} // namespace

kangsw::markup::marshalerr_t
kangsw::markup::parse(object& to, json_parse const& from)
{
    // TODO: 파서 기능 구현하기
    // 앞에서부터 캐릭터 하나씩 iterate ...
    // 푸시다운 오토마타로 구현 ...
    auto view = from.source;
    parser_context context;
    return parse_value(context, to, view);
}