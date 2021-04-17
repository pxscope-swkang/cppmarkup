#pragma once
#include "object.hpp"

#define CPPMARKUP_OBJECT_TEMPLATE(template_type_name)                                \
    struct INTERNAL_EZ_SUPER_##template_type_name                                    \
        : public ::cppmarkup::impl::traits<INTERNAL_EZ_SUPER_##template_type_name> { \
        using INTERNAL_EZ_super = INTERNAL_EZ_SUPER_##template_type_name;            \
    };                                                                               \
    struct template_type_name : public INTERNAL_EZ_SUPER_##template_type_name

#ifndef CPPMARKUP_ALIGN
#define CPPMARKUP_ALIGN 8
#endif

#define CPPMARKUP_ADD(varname, tag, default_value, ...)                                                    \
    struct INTERNAL_EZ_INSTANCE_##varname {                                                                \
        using value_type = decltype(default_value);                                                        \
                                                                                                           \
    private:                                                                                               \
        alignas(CPPMARKUP_ALIGN) value_type _value                             = default_value;            \
        static inline std::atomic<::cppmarkup::impl::node_property*> _node_ref = nullptr;                  \
                                                                                                           \
    public:                                                                                                \
        static bool parse(void* r, size_t size, pugi::xml_node const& s)                                   \
        {                                                                                                  \
            assert(sizeof(value_type) == size);                                                            \
            return ::cppmarkup::marshal::parse(*(value_type*)r, s);                                        \
        }                                                                                                  \
                                                                                                           \
        static inline ::cppmarkup::impl::object_inst_init<                                                 \
            INTERNAL_EZ_INSTANCE_##varname, value_type, CPPMARKUP_ALIGN>                                   \
            _init{                                                                                         \
                _node_ref,                                                                                 \
                INTERNAL_EZ_node_list,                                                                     \
                tag,                                                                                       \
                sizeof _value,                                                                             \
                &INTERNAL_EZ_description_str,                                                              \
                &parse};                                                                                   \
                                                                                                           \
        operator value_type() const                                                                        \
        {                                                                                                  \
            return _value;                                                                                 \
        }                                                                                                  \
                                                                                                           \
        INTERNAL_EZ_INSTANCE_##varname(void* owner_base)                                                   \
        { /* Accurate offset is recalculated here. */                                                      \
            if (auto ptr = _node_ref.exchange(nullptr))                                                    \
            {                                                                                              \
                printf("ptr: %p ", ptr);                                                                   \
                ptr->offset      = (intptr_t)this - (intptr_t)owner_base;                                  \
                ptr->next_offset = ptr->offset + sizeof *this;                                             \
                printf("offset: %llu ~ size: %llu\n", ptr->offset, ptr->total_size);                       \
            }                                                                                              \
        }                                                                                                  \
                                                                                                           \
        INTERNAL_EZ_INSTANCE_##varname(const INTERNAL_EZ_INSTANCE_##varname& r) = default;                 \
        INTERNAL_EZ_INSTANCE_##varname(INTERNAL_EZ_INSTANCE_##varname&& r)      = default;                 \
        INTERNAL_EZ_INSTANCE_##varname(value_type const& r = value_type{}) : _value(r) {}                  \
        INTERNAL_EZ_INSTANCE_##varname(value_type&& r) : _value(std::move(r)) {}                           \
        INTERNAL_EZ_INSTANCE_##varname& operator=(const INTERNAL_EZ_INSTANCE_##varname& r) = default;      \
        INTERNAL_EZ_INSTANCE_##varname& operator=(INTERNAL_EZ_INSTANCE_##varname&& r) = default;           \
                                                                                                           \
        INTERNAL_EZ_INSTANCE_##varname& operator=(value_type const& r) { return _value = r, *this; }       \
        INTERNAL_EZ_INSTANCE_##varname& operator=(value_type&& r) { return _value = std::move(r), *this; } \
        auto& operator()() { return _value; }                                                              \
        auto& operator()() const { return _value; }                                                        \
                                                                                                           \
        __VA_ARGS__                                                                                        \
    } varname{this};

#define CPPMARKUP_ADD2(varname, default_value, ...) CPPMARKUP_ADD(varname, u8## #varname, default_value, __VA_ARGS__)

#define CPPMARKUP_ATTR(attr_name, default_value)                          \
    alignas(CPPMARKUP_ALIGN) std::u8string attr_name;                     \
    struct INTERNAL_EZ_ATTR_##attr_name {                                 \
        static inline ::cppmarkup::impl::object_inst_attr_init<           \
            INTERNAL_EZ_ATTR_##attr_name, CPPMARKUP_ALIGN>                \
            _init{INTERNAL_EZ_node_list, u8## #attr_name, default_value}; \
    };

#define CPPMARKUP_ADD_ARRAY(template_type_name)
#define CPPMARKUP_DESCRIPTION_BELOW(description) \
    static inline INTERNAL_EZ_description_replacer DESCRIPTION_##__LINE__{description};

#define CPPMARKUP_NESTED_OBJECT(varname, tag, ...)               \
    CPPMARKUP_OBJECT_TEMPLATE(INTERNAL_EZ_##varname##_TEMPLATE){ \
        __VA_ARGS__};                                            \
    CPPMARKUP_ADD(varname, tag, INTERNAL_EZ_##varname##_TEMPLATE{})
