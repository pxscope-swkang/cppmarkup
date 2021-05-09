#pragma once
#include <functional>
#include <vector>
#include "../types.hxx"
#include "object_container_interface.hxx"

namespace kangsw::refl {

/** */
namespace property_flag {
enum type : uint64_t {
    none
};
}

using property_flag_t = property_flag::type;

/** Thrown when invalid property initialization access */
struct property_already_initialized_exception : std::logic_error {
    using std::logic_error::logic_error;
};
struct invalid_container_interface_setup_exception : std::logic_error {
    using std::logic_error::logic_error;
};

/**
 * 
 */
class property {
public:
    struct memory_t {
        /** */
        etype type;

        /** */
        size_t offset;

        /** */
        size_t size;

        /** initializes memory represented by this property */
        std::function<void(void*)> init_fn;

        void* operator()(void* obj) const { return static_cast<char*>(obj) + offset; }
        void const* operator()(void const* obj) const { return static_cast<char const*>(obj) + offset; }

        void* operator()(void* obj, ptrdiff_t ofst) const { return static_cast<char*>(obj) + offset + ofst; }
        void const* operator()(void const* obj, ptrdiff_t ofst) const { return static_cast<char const*>(obj) + offset + ofst; }
    };

    struct attribute {
        /** */
        u8str name;

        /** */
        memory_t _memory;

        auto& memory() const { return _memory; }
    };

public:
    bool is_valid_property() const { return _is_valid; }
    auto& attributes() const { return _attr; }
    auto& tag() const { return _tag; }
    auto& doc() const { return _doc; }
    auto& memory() const { return _memory; }

    auto ovi() const { return _ovi.get(); }
    auto omi() const { return _omi.get(); }

public:
    property& operator=(property&&) noexcept = default;
    property(property&&) noexcept            = default;
    property(u8str&& tag) noexcept
        : _tag(std::move(tag)) {}

    // for internal usage
    void _set_defaults(
        u8str&& doc,
        property_flag_t flag,
        memory_t&& memory) //
    {
        if (_is_valid) { throw property_already_initialized_exception(_tag); }
        _doc    = std::move(doc);
        _flag   = flag;
        _memory = std::move(memory);

        _is_valid = true;
    }

    void _add_attr(attribute&& attr) {
        // TODO: throw logic error on duplicated attribute registering
        _attr.push_back(std::move(attr));
    }

    // TODO: remove attribute

    // sets ovi or omi
    void _set_ovi(object_vector_interface* new_ovi) {
        if (!_is_valid || !(_memory.type.is_object() && _memory.type.is_array())) {
            throw invalid_container_interface_setup_exception("");
        }
        _ovi.reset(new_ovi);
    }

    void _set_omi(object_map_interface* new_omi) {
        if (!_is_valid || !(_memory.type.is_object() && _memory.type.is_map())) {
            throw invalid_container_interface_setup_exception("");
        }
        _omi.reset(new_omi);
    }

private:
    bool _is_valid = false;

    u8str _tag       = {};
    memory_t _memory = {};

    u8str _doc            = {};
    property_flag_t _flag = {};

    std::vector<attribute> _attr = {};

    std::unique_ptr<object_vector_interface const> _ovi = {};
    std::unique_ptr<object_map_interface const> _omi    = {};
};

} // namespace kangsw::refl
