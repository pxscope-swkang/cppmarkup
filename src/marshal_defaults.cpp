#include "cppmarkup/marshal_defaults.hpp"

template <> bool kangsw::markup::parse<long long>(int64_t& out, u8string_view in)
{
    return false;
}

template <> bool kangsw::markup::dump<long long>(int64_t const& in, u8string& out)
{
    return false;
}

template <> bool kangsw::markup::parse<double>(double& out, u8string_view in)
{
    return false;
}

template <> bool kangsw::markup::dump<double>(double const& in, u8string& out)
{
    return false;
}

template <> bool kangsw::markup::parse<std::string>(u8string& out, u8string_view in)
{
    return false;
}

template <> bool kangsw::markup::dump<std::string>(u8string const& in, u8string& out)
{
    return false;
}
