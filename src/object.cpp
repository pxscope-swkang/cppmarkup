#include "ezdata/object.hpp"

std::u8string* ezdata::impl::node_property::get_attrib(void* element_base, size_t index) const
{
    auto str_base = reinterpret_cast<std::u8string*>(value_size + (char*)element_base);
    assert(index <= (next_offset - offset + value_size) / sizeof(std::u8string));
    assert((next_offset - offset + value_size) % sizeof(std::u8string) == attrib_default.size());
    return str_base + index;
}