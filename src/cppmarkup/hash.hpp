#pragma once
#include <cstdint>
#include <string_view>

namespace cppmarkup {

template <typename Char_>
constexpr uint64_t get_hash(std::basic_string_view<Char_> str, uint64_t seed = 0)
{
    auto hash = seed;
    for (auto ch : str)
    {
        // multiplies 31 to hash, then add character value
        hash = (hash << 5) - hash + ch;
    }
    return hash;
}

} // namespace cppmarkup
