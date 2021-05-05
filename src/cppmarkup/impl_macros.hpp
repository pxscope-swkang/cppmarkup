#pragma once
#include "object.hpp"
#include "template_utils.hxx"

#define INTERNAL_CPPMARKUP_STRINGFY3(X) u8 #X
#define INTERNAL_CPPMARKUP_STRINGFY2(X) INTERNAL_CPPMARKUP_STRINGFY3(X)
#define INTERNAL_CPPMARKUP_STRINGFY(X)  INTERNAL_CPPMARKUP_STRINGFY2(X)

#define INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(tname) \
    struct tname : public ::kangsw::markup::impl::object_base<tname>

#define INTERNAL_CPPMARKUP_INSTANCE_FORMER(varname, tag_name_str, ... /*ATTRIBUTES*/)          \
public:                                                                                        \
    class INTERNAL_TYPE_##varname                                                              \
        : public ::kangsw::markup::impl::element_template_base<INTERNAL_TYPE_##varname> {      \
    private:                                                                                   \
        using self_type               = INTERNAL_TYPE_##varname;                               \
        static constexpr auto _tagstr = tag_name_str;                                          \
                                                                                               \
        static inline struct _internal_description_assignment {                                \
            _internal_description_assignment()                                                 \
            {                                                                                  \
                _description                = INTERNAL_next_description();                     \
                INTERNAL_next_description() = {};                                              \
            }                                                                                  \
        } _description_assignment;                                                             \
                                                                                               \
    private:                                                                                   \
        static auto& attribs() noexcept                                                        \
        {                                                                                      \
            static std::vector<::kangsw::markup::property::attribute_representation> _attribs; \
            return _attribs;                                                                   \
        }                                                                                      \
                                                                                               \
    public:                                                                                    \
        /*ATTRIBUTES will be placed here */ __VA_ARGS__

namespace kangsw::markup::impl {

template <typename Ty_>
decltype(auto) deduce_fn(Ty_&& v)
{
    if constexpr (std::is_same_v<Ty_, binary_chunk>) { return binary_chunk(std::move(v)); }
    if constexpr (std::is_same_v<Ty_, bool>) { return bool(v); }
    if constexpr (std::is_integral_v<Ty_> && !std::is_same_v<Ty_, bool>) { return int64_t{v}; }
    if constexpr (std::is_floating_point_v<Ty_>) { return double{v}; }
    if constexpr (templates::is_specialization<Ty_, std::basic_string>::value) { return u8string{v.begin(), v.end()}; }
    if constexpr (std::is_base_of_v<object, Ty_>) { return Ty_(std::move(v)); }
    if constexpr (std::is_same_v<Ty_, u8string::const_pointer>) { return u8string{v}; }
}

template <typename Ty_, size_t N>
decltype(auto) deduce_fn(Ty_ (&v)[N]) { return u8string{v}; }

template <typename Ty_>
decltype(auto) deduce_fn(std::initializer_list<Ty_>&& v)
{
    return std::vector<decltype(deduce_fn(Ty_{}))>(v.begin(), v.end());
}

template <typename Ty_>
decltype(auto) deduce_fn(std::map<u8string, Ty_>&& v)
{
    return std::map<u8string, decltype(deduce_fn(Ty_{}))>(std::move(v));
}

template <typename KTy_, typename Ty_, typename... Args_>
decltype(auto) deduce_map(KTy_&& a, Ty_&& b, Args_&&... args)
{
    std::map<u8string, decltype(deduce_fn(Ty_{}))> map;

    return map;
}

} // namespace kangsw::markup::impl

#define INTERNAL_CPPMARKUP_ATTRIBUTE(attr_varname, attr_name, default_value)                               \
public:                                                                                                    \
    struct INTERNAL_ATTR_##attr_varname : ::kangsw::markup::impl::attribute_base {                         \
        using attr_value_type = decltype(::kangsw::markup::impl::deduce_fn(default_value));                \
        static_assert(!::kangsw::markup::get_element_type<attr_value_type>().is_array() &&                 \
                      !::kangsw::markup::get_element_type<attr_value_type>().is_map() &&                   \
                      !::kangsw::markup::get_element_type<attr_value_type>().is_object());                 \
                                                                                                           \
        INTERNAL_ATTR_##attr_varname(self_type* base) noexcept                                             \
        {                                                                                                  \
            if (!INTERNAL_is_first_entry) { return; }                                                      \
                                                                                                           \
            INTERNAL_attrbase_init(                                                                        \
                (::kangsw::markup::impl::element_base*)                                                    \
                    base,                                                                                  \
                attr_name,                                                                                 \
                ::kangsw::markup::get_element_type<attr_value_type>(),                                     \
                attribs(),                                                                                 \
                sizeof *this,                                                                              \
                [](void* v) { *(attr_value_type*)v = ::kangsw::markup::impl::deduce_fn(default_value); }); \
        }                                                                                                  \
                                                                                                           \
    private:                                                                                               \
        attr_value_type _value;                                                                            \
                                                                                                           \
    public:                                                                                                \
        INTERNAL_ATTR_##attr_varname(attr_value_type const& v) noexcept                                    \
            : _value(v)                                                                                    \
        {}                                                                                                 \
        INTERNAL_ATTR_##attr_varname(attr_value_type&& v) noexcept                                         \
            : _value(std::move(v))                                                                         \
        {}                                                                                                 \
                                                                                                           \
        auto& value() noexcept { return _value; }                                                          \
        auto& value() const noexcept { return _value; }                                                    \
        auto operator->() noexcept { return &_value; }                                                     \
        auto operator->() const noexcept { return &_value; }                                               \
        template <typename N_> auto& operator[](N_ i) noexcept { return _value[i]; }                       \
        template <typename N_> auto& operator[](N_ i) const noexcept { return _value[i]; }                 \
        operator attr_value_type&() noexcept { return _value; }                                            \
        operator attr_value_type const &() const noexcept { return _value; }                               \
    } attr_varname{this /* 어트리뷰트 오프셋 / 사이즈 계산용, 최초 1회 */};

#define INTERNAL_CPPMARKUP_INSTANCE_LATER(varname, default_value)                                \
private:                                                                                         \
    value_type _value;                                                                           \
                                                                                                 \
public:                                                                                          \
    INTERNAL_TYPE_##varname(::kangsw::markup::object* base) noexcept                             \
    {                                                                                            \
        if (!INTERNAL_is_first_entry) { return; }                                                \
                                                                                                 \
        INTERNAL_elembase_init(                                                                  \
            ::kangsw::markup::get_element_type<value_type>(),                                    \
            base, _tagstr, _description,                                                         \
            offsetof(INTERNAL_TYPE_##varname, _value),                                           \
            sizeof _value, sizeof *this,                                                         \
            [](void* v) { *(value_type*)v = ::kangsw::markup::impl::deduce_fn(default_value); }, \
            attribs(),                                                                           \
            ::kangsw::markup::impl::object_array_instance<value_type>::get(),                    \
            ::kangsw::markup::impl::object_map_instance<value_type>::get());                     \
    }                                                                                            \
                                                                                                 \
    INTERNAL_TYPE_##varname(value_type const& v) noexcept                                        \
        : _value(v)                                                                              \
    {}                                                                                           \
    INTERNAL_TYPE_##varname(value_type&& v) noexcept                                             \
        : _value(std::move(v))                                                                   \
    {}                                                                                           \
    auto& operator=(value_type&& v) noexcept { return _value = (std::move(v)), *this; }          \
    auto& operator=(value_type const& v) noexcept { return _value = (v), *this; }                \
                                                                                                 \
    INTERNAL_TYPE_##varname(INTERNAL_TYPE_##varname const&) noexcept = default;                  \
    INTERNAL_TYPE_##varname(INTERNAL_TYPE_##varname&&) noexcept      = default;                  \
    INTERNAL_TYPE_##varname& operator=(INTERNAL_TYPE_##varname const& v) noexcept = default;     \
    INTERNAL_TYPE_##varname& operator=(INTERNAL_TYPE_##varname&& v) noexcept = default;          \
                                                                                                 \
    auto& value() noexcept { return _value; }                                                    \
    auto& value() const noexcept { return _value; }                                              \
    auto operator->() noexcept { return &_value; }                                               \
    auto operator->() const noexcept { return &_value; }                                         \
    operator value_type&() noexcept { return _value; }                                           \
    operator value_type const &() const noexcept { return _value; }                              \
    template <typename N_> auto& operator[](N_ i) noexcept { return _value[i]; }                 \
    template <typename N_> auto& operator[](N_ i) const noexcept { return _value[i]; }           \
    }                                                                                            \
    varname{this};

#define INTERNAL_CPPMARKUP_ADD(varname, tag_name, default_value, ...)              \
    INTERNAL_CPPMARKUP_INSTANCE_FORMER(varname, tag_name, ##__VA_ARGS__);          \
    using value_type = decltype(::kangsw::markup::impl::deduce_fn(default_value)); \
    INTERNAL_CPPMARKUP_INSTANCE_LATER(varname, default_value)

#define INTERNAL_CPPMARKUP_EMBED_OBJECT_begin(varname, tag_name, ...)     \
    INTERNAL_CPPMARKUP_INSTANCE_FORMER(varname, tag_name, ##__VA_ARGS__); \
    INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(TEMPLATE_##varname)

#define INTERNAL_CPPMARKUP_EMBED_OBJECT_end(varname) \
    ;                                                \
    using value_type = TEMPLATE_##varname;           \
    INTERNAL_CPPMARKUP_INSTANCE_LATER(varname, value_type::get_default())

#define INTERNAL_CPPMARKUP_WRAPPED_OBJECT_TEMPLATE(wrapper_type, body_type, varname, tag, ...) \
    INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(wrapper_type)                                           \
    {                                                                                          \
        INTERNAL_CPPMARKUP_ADD(varname, tag, body_type::get_default(), ##__VA_ARGS__);         \
        auto& operator()() { return varname; }                                                 \
        auto& operator()() const { return varname; }                                           \
    }

#ifdef CPPMARKUP_BUILD_WITH_DESCRIPTION
#define INTERNAL_CPPMARKUP_DESCRIPTION(description)                                                      \
private:                                                                                                 \
    static inline struct INTERNAL_description_assignment_type_##__LINE__ {                               \
        INTERNAL_description_assignment_type_##__LINE__() { INTERNAL_next_description() = description; } \
    } INTERNAL_description_assignment;                                                                   \
                                                                                                         \
public:

#else
#define INTERNAL_CPPMARKUP_DESCRIPTION(...)
#endif