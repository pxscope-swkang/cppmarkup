#pragma once
#include "../types.hxx"

namespace kangsw::refl {

/** Object container support - vector */
class object_vector_interface {
public:
    virtual ~object_vector_interface() = default;

    virtual size_t size(void const* p) const                = 0;
    virtual object const& at(void const* p, size_t i) const = 0;
    virtual object& at(void* p, size_t i) const             = 0;

    virtual object& push_back(void* p) const                  = 0;
    virtual void erase(void* p, size_t from, size_t to) const = 0;
    virtual void reserve(void* p, size_t new_size)            = 0;
};

/** Object container support - map */
class object_map_interface {
public:
    virtual ~object_map_interface() = default;

    virtual size_t size(void const* p) const                      = 0;
    virtual object const& at(void const* p, u8str_view s) const   = 0;
    virtual object& at(void* p, u8str_view s) const               = 0;
    virtual object* find(void* p, u8str_view s) const             = 0;
    virtual object const* find(void const* p, u8str_view s) const = 0;

    virtual object& insert(void* p, u8str_view s) const = 0;
    virtual void erase(void* p, u8str_view s) const     = 0;

    virtual void for_each(void* p, std::function<void(u8str_view, object&)> const&) const             = 0;
    virtual void for_each(void const* p, std::function<void(u8str_view, object const&)> const&) const = 0;
};

} // namespace kangsw::refl