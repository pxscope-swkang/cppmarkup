#pragma once
#include "object.hpp"
#include "template_utils.hxx"

#define INTERNAL_CPPMARKUP_STRINGFY3(X) u8 #X
#define INTERNAL_CPPMARKUP_STRINGFY2(X) INTERNAL_CPPMARKUP_STRINGFY3(X)
#define INTERNAL_CPPMARKUP_STRINGFY(X)  INTERNAL_CPPMARKUP_STRINGFY2(X)

#define INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(tname) \
    struct tname : ::kangsw::markup::impl::object_base<tname>

#define INTERNAL_CPPMARKUP_OBJECT_TEMPLATE_BODY(tname)                     \
private:                                                                   \
    void should_declare_CPPMARKUP_OBJECT_TEMPLATE_BODY_first() override {} \
                                                                           \
    static inline struct INTERNAL_BODY_##tname {                           \
        INTERNAL_BODY_##tname()                                            \
        {                                                                  \
            tname autogen = {};                                            \
        }                                                                  \
    } INTERNAL_BODY_INIT_##tname;

#define INTERNAL_CPPMARKUP_INSTANCE_FORMER(varname, tag_name_str, ... /*ATTRIBUTES*/)                      \
public:                                                                                                    \
    class INTERNAL_TYPE_##varname : ::kangsw::markup::impl::element_template_base<INTERNAL_TYPE_##varname> \
    {                                                                                                      \
    private:                                                                                               \
        using self_type               = INTERNAL_TYPE_##varname;                                           \
        static constexpr auto _tagstr = tag_name_str;                                                      \
                                                                                                           \
        static inline struct _internal_description_assignment {                                            \
            _internal_description_assignment()                                                             \
            {                                                                                              \
                _description              = INTERNAL_next_description;                                     \
                INTERNAL_next_description = {};                                                            \
            }                                                                                              \
        } _description_assignment;                                                                         \
                                                                                                           \
    private:                                                                                               \
        static inline std::vector<::kangsw::markup::property::attribute_representation> _attribs;          \
                                                                                                           \
    public:                                                                                                \
        /*ATTRIBUTES will be placed here */ __VA_ARGS__

namespace kangsw::markup::impl {

template <typename Ty_>
decltype(auto) deduce_fn(Ty_&&) { return Ty_{}; }

template <typename Ty_, size_t N>
decltype(auto) deduce_fn(Ty_ (&)[N]) { return std::basic_string<std::remove_const_t<Ty_>>{}; }

template <typename Ty_>
decltype(auto) deduce_fn(std::initializer_list<Ty_>&&) { return std::vector<Ty_>{}; }

} // namespace kangsw::markup::impl

/* TODO decltype(default_value) -> deduce_type<decltype(default_value)>::type으로 변경
 * initializer_list -> vector로 인식하게 */
#define INTERNAL_CPPMARKUP_ATTRIBUTE(attr_varname, attr_name, default_value)                 \
public:                                                                                      \
    struct INTERNAL_ATTR_##attr_varname : ::kangsw::markup::impl::attribute_base {           \
        using attr_value_type = decltype(::kangsw::markup::impl::deduce_fn(default_value));  \
        static inline ::kangsw::markup::impl::marshaller_instance<attr_value_type> _marshal; \
                                                                                             \
        INTERNAL_ATTR_##attr_varname(self_type* base)                                        \
        {                                                                                    \
            if (!INTERNAL_is_first_entry) { return; }                                        \
                                                                                             \
            INTERNAL_attrbase_init(                                                          \
                base, attr_name, _attribs,                                                   \
                sizeof *this, &_marshal,                                                     \
                [](void* v) { *(attr_value_type*)v = default_value; });                      \
        }                                                                                    \
                                                                                             \
    private:                                                                                 \
        attr_value_type _value;                                                              \
                                                                                             \
    public:                                                                                  \
        auto& operator()() { return _value; }                                                \
        auto& operator()() const { return _value; }                                          \
    } attr_varname{this /* 어트리뷰트 오프셋 / 사이즈 계산용, 최초 1회 */};

#define INTERNAL_CPPMARKUP_INSTANCE_LATER(varname, default_value)                   \
private:                                                                            \
    static inline ::kangsw::markup::impl::marshaller_instance<value_type> _marshal; \
    value_type _value;                                                              \
                                                                                    \
public:                                                                             \
    INTERNAL_TYPE_##varname(::kangsw::markup::object* base)                         \
    {                                                                               \
        if (!INTERNAL_is_first_entry) { return; }                                   \
                                                                                    \
        INTERNAL_elembase_init(                                                     \
            ::kangsw::markup::get_element_type<value_type>(),                       \
            base, _tagstr, _description,                                            \
            offsetof(INTERNAL_TYPE_##varname, _value),                              \
            sizeof _value, sizeof *this, &_marshal,                                 \
            [](void* v) { *(value_type*)v = default_value; },                       \
            _attribs);                                                              \
    }                                                                               \
                                                                                    \
    auto& operator()() { return _value; }                                           \
    auto& operator()() const { return _value; }                                     \
    }                                                                               \
    varname { this }

#define INTERNAL_CPPMARKUP_ADD(varname, tag_name, default_value, ...)              \
    INTERNAL_CPPMARKUP_INSTANCE_FORMER(varname, tag_name, ##__VA_ARGS__);          \
    using value_type = decltype(::kangsw::markup::impl::deduce_fn(default_value)); \
    INTERNAL_CPPMARKUP_INSTANCE_LATER(varname, default_value)

#define INTERNAL_CPPMARKUP_EMBED_OBJECT_begin(varname, tag_name, ...)     \
    INTERNAL_CPPMARKUP_INSTANCE_FORMER(varname, tag_name, ##__VA_ARGS__); \
    INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(TEMPLATE_##varname)                \
    {                                                                     \
        INTERNAL_CPPMARKUP_OBJECT_TEMPLATE_BODY(TEMPLATE_##varname);

#define INTERNAL_CPPMARKUP_EMBED_OBJECT_end(varname) \
    INTERNAL_CPPMARKUP_INSTANCE_LATER(varname, TEMPLATE_##varname{});

#define INTERNAL_CPPMARKUP_WRAPPED_OBJECT_TEMPLATE(wrapper_type, body_type, varname, tag, ...) \
    INTERNAL_CPPMARKUP_OBJECT_TEMPLATE(wrapper_type)                                           \
    {                                                                                          \
        INTERNAL_CPPMARKUP_OBJECT_TEMPLATE_BODY(wrapper_type);                                 \
        INTERNAL_CPPMARKUP_ADD(varname, tag, body_type(), ##__VA_ARGS__);                      \
        auto& operator()() { return varname; }                                                 \
        auto& operator()() const { return varname; }                                           \
    }

#ifdef CPPMARKUP_BUILD_WITH_DESCRIPTION
#define INTERNAL_CPPMARKUP_DESCRIPTION(description)                                                   \
private:                                                                                              \
    static inline struct INTERNAL_description_assignment_type_##__LINE__ {                            \
        INTERNAL_description_assignment_type_##__LINE__() { INTERNAL_next_description = description; } \
    } INTERNAL_description_assignment;                                                                \
                                                                                                      \
public:

#else
#define INTERNAL_CPPMARKUP_DESCRIPTION(...)
#endif