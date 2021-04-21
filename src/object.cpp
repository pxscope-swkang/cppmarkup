#include "cppmarkup/object.hpp"

template <> bool kangsw::markup::parse<kangsw::markup::object>(object& o, u8string_view i)
{
    throw;
}

template <> bool kangsw::markup::dump<kangsw::markup::object>(object const& o, u8string& i)
{
    throw;
}
