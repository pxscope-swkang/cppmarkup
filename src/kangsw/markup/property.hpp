#pragma once
#include "types.hpp"
#include <vector>

namespace kangsw::refl {

/**
 * 
 */
class property {
public:
    struct memory_layout {
        /** */
        element_type type;

        /** */
        size_t offset_from_owner_object;

        /** */
        size_t offset_from_base;

        /** */
        size_t size;

        /** initializes memory represented by this property */
        void (*pinit)(void*);

        /** */
        void* operator()(object* obj) const { return reinterpret_cast<char*>(obj) + offset_from_owner_object + offset_from_base; }
        void const* operator()(object const* obj) const { return reinterpret_cast<char const*>(obj) + offset_from_owner_object + offset_from_base; }
        void* base(object* obj) const { return reinterpret_cast<char*>(obj) + offset_from_owner_object; }
        void const* base(object const* obj) const { return reinterpret_cast<char const*>(obj) + offset_from_owner_object; }

        template<typename Ty_>
        Ty_ const* as(object* obj) const
        {
            
        }
    };

    struct attribute {
        /** */
        u8str name;

        /** */
        memory_layout memory;
    };

public:
    property(u8str&& tag, u8str&& doc, memory_layout&& value)
        : _tag(std::move(tag))
        , _doc(std::move(doc))
        , _memory(std::move(value))
    {}

    // for internal usage
    void _add_attr(attribute&& attr) { _attr.push_back(std::move(attr)); }

public:
    auto& attr() const { return _attr; }
    auto& tag() const { return _tag; }
    auto& doc() const { return _doc; }
    auto& memory() const {}

private:
    u8str _tag;
    u8str _doc;
    memory_layout _memory;
    std::vector<attribute> _attr;
};

} // namespace kangsw::refl
