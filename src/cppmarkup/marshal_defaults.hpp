#pragma once
#include "template_utils.hxx"
#include "typedefs.hpp"
#include <cstdint>
#include <map>
#include <type_traits>

/**
 * 파싱과 덤프를 위한 기본적인 인터페이스입니다.
 */
namespace kangsw::markup {
template <typename Ty_> bool parse_array(std::vector<Ty_>& out, u8string_view in);
template <typename Ty_> bool dump_array(std::vector<Ty_> const& in, u8string& out);
template <typename Ty_> bool parse_map(std::map<u8string, Ty_>& out, u8string_view in);
template <typename Ty_> bool dump_map(std::map<u8string, Ty_> const& in, u8string& out);

/**
 * JSON 파싱 섹션 ... 
 */
template <typename Ty_> bool parse(Ty_& out, u8string_view in)
{
    if constexpr (std::is_base_of_v<object, Ty_>)
    {
        return parse<object>(out, in);
    }
    else if constexpr (std::is_integral_v<Ty_>)
    {
        int64_t value;
        bool const result = parse(value, in);

        return out = static_cast<Ty_>(value), result;
    }
    else if constexpr (std::is_floating_point_v<Ty_>)
    {
        double value;
        bool const result = parse(value, in);

        return out = static_cast<Ty_>(value), result;
    }
    else if constexpr (templates::is_specialization<Ty_, std::vector>::value)
    {
        return parse_array(out, in);
    }
    else if constexpr (templates::is_specialization<Ty_, std::map>::value)
    {
        return parse_map(out, in);
    }
    else
    {
        static_assert(false);
        return false;
    }
}

template <typename Ty_> bool dump(Ty_ const& in, u8string& out)
{
    if constexpr (std::is_base_of_v<object, Ty_>)
    {
        return dump<object>(in, out);
    }
    else if constexpr (std::is_integral_v<Ty_>)
    {
        return dump((int64_t)in, out);
    }
    else if constexpr (std::is_floating_point_v<Ty_>)
    {
        return dump((double)in, out);
    }
    else if constexpr (templates::is_specialization<Ty_, std::vector>::value)
    {
        return dump_array(in, out);
    }
    else if constexpr (templates::is_specialization<Ty_, std::map>::value)
    {
        return dump_map(in, out);
    }
    else
    {
        static_assert(false);
        return false;
    }
}

template <> bool parse(int64_t& out, u8string_view in);
template <> bool dump(int64_t const& in, u8string& out);
template <> bool parse(double& out, u8string_view in);
template <> bool dump(double const& in, u8string& out);
template <> bool parse(u8string& out, u8string_view in);
template <> bool dump(u8string const& in, u8string& out);

template <typename Ty_> bool parse_array(std::vector<Ty_>& out, u8string_view in)
{
    return false;
}

template <typename Ty_> bool dump_array(std::vector<Ty_> const& in, u8string& out)
{
    return false;
}

template <typename Ty_> bool parse_map(std::map<u8string, Ty_>& out, u8string_view in)
{
    return false;
}

template <typename Ty_> bool dump_map(std::map<u8string, Ty_> const& in, u8string& out)
{
    return false;
}


/**
 * CompactBinary 파싱 섹션 ... 
 */
template <typename Ty_> bool parse(Ty_& o, compact_binary_view i) { throw; }
template <typename Ty_> bool dump(Ty_& o, compact_binary& i) { throw; }

} // namespace kangsw::markup
