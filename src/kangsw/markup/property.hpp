#pragma once
#include <vector>
#include "types.hpp"

namespace kangsw::refl {
/**
 * 
 */
class property {
public:
    struct memory_layout {
        /** */
        etype type;

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

        template <typename Ty_>
        auto as(object const* obj) const {
            if (etype::from_type<Ty_>() != type) { return nullptr; }
            return static_cast<etype::deduce_result_t<Ty_>*>((*this)(obj));
        }

        template <typename Ty_>
        auto as(object* obj) const {
            if (etype::from_type<Ty_>() != type) { return nullptr; }
            return static_cast<etype::deduce_result_t<Ty_>*>((*this)(obj));
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
        , _memory(std::move(value)) {}

    // for internal usage
    void _add_attr(attribute&& attr) { _attr.push_back(std::move(attr)); }

public:
    auto& attr() const { return _attr; }
    auto& tag() const { return _tag; }
    auto& doc() const { return _doc; }
    auto& memory() const { return _memory; }

private:
    u8str const _tag;
    u8str const _doc;
    memory_layout const _memory;
    std::vector<attribute> _attr;
};

} // namespace kangsw::refl
