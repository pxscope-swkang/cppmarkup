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
        void* operator()(object* obj) { return (char*)obj + offset_from_owner_object + offset_from_base; }
    };

    struct attribute {
        /** */
        u8str name;

        /** */
        memory_layout memory;
    };

public:


private:
    u8str _tag;
    u8str _doc;
    memory_layout _memory;
    std::vector<attribute> _attr;
};

} // namespace kangsw::refl
