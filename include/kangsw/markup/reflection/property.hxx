#pragma once
#include <functional>
#include <vector>
#include "../types.hxx"

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

/**
 * 
 */
class property {
public:
    struct memory_layout {
        /** */
        etype type;

        /** */
        size_t offset;

        /** */
        size_t size;

        /** initializes memory represented by this property */
        std::function<void(void*)> init_fn;

    private:
        friend class object;

        void* operator()(void* obj) const { return static_cast<char*>(obj) + offset; }
        void const* operator()(void const* obj) const { return static_cast<char const*>(obj) + offset; }
        void* operator()(void* obj, ptrdiff_t ofst) const { return static_cast<char*>(obj) + offset + ofst; }
        void const* operator()(void const* obj, ptrdiff_t ofst) const { return static_cast<char const*>(obj) + offset + ofst; }

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

        /** */
        ptrdiff_t offset() const { return _owner->attribute_offset(); }

    private:
        friend class property;

        property* _owner;
    };

public:
    property(u8str&& tag)
        : _tag(std::move(tag)) {}

    // for internal usage
    void _set_defaults(
        u8str&& doc,
        property_flag_t flag,
        memory_layout&& memory,
        ptrdiff_t optional_attribute_offset = {}) //
    {
        if (_is_valid) { throw property_already_initialized_exception(_tag); }
        _doc              = std::move(doc);
        _flag             = flag;
        _memory           = std::move(memory);
        _attribute_offset = optional_attribute_offset;

        _is_valid = true;
    }

    void _add_attr(attribute&& attr) {
        // TODO: throw logic error on duplicated attribute registering
        attr._owner = this;
        _attr.push_back(std::move(attr));
    }

public:
    bool is_valid_property() const { return _is_valid; }
    auto& attributes() const { return _attr; }
    auto& tag() const { return _tag; }
    auto& doc() const { return _doc; }
    auto& memory() const { return _memory; }
    ptrdiff_t attribute_offset() const { return _attribute_offset; }

private:
    u8str const _tag             = {};
    u8str _doc                   = {};
    property_flag_t _flag        = {};
    ptrdiff_t _attribute_offset  = {};
    memory_layout _memory        = {};
    std::vector<attribute> _attr = {};
    bool _is_valid               = false;
};

} // namespace kangsw::refl
