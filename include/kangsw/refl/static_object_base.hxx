#pragma once
#include "object.hxx"

/**
 * Static object interface ...
 *
 * 
 */
namespace kangsw::refl {

/**  */
template <typename Ty_>
class static_object_base : public object {
public:
    using self_type   = Ty_;
    using traits_type = object_traits<Ty_>;

public:
    object_traits_base const& traits() const override { return traits_type::get(); }

protected:
    void* base() override { return this; }
    void const* base() const override { return this; }
};

/** Performs element registration */
template <typename ObjTy_, typename ValueTy_, typename HashTy_>
class element_regestration_t {
public:
    element_regestration_t(
        u8str&& tag,
        size_t offset,
        ValueTy_&& initial_value,
        int elem_flags) //
    {
        using traits_type = object_traits<ObjTy_>;
        auto& prop        = traits_type::get().find_or_add_property(tag);

        property::memory_layout m;
        m.type                     = etype::from_type<ValueTy_>();
        m.size                     = sizeof(ValueTy_);
        m.offset = offset;

        m.init_fn = [_v = std::move(initial_value)](void* pv) {
            *(ValueTy_*)pv = _v;
        };

        prop._set_defaults("" /*TODO ...*/, (property_flag_t)elem_flags, std::move(m));
    }
};

/** Performs attribute registration */
template <typename ObjTy_, typename ValueTy_, typename HashTy_>
class attribute_registration_t {
public:
    attribute_registration_t(
        u8str_view tag,
        u8str&& name,
        size_t offset,
        ValueTy_&& initial_value) //
    {
        using traits_type = object_traits<ObjTy_>;
        auto& prop        = traits_type::get().find_or_add_property(tag);

        property::attribute attr;
        attr.name                            = std::move(name);
        attr.memory.size                     = sizeof(ValueTy_);
        attr.memory.offset = offset;
        attr.memory.type                     = etype::from_type<ValueTy_>();

        attr.memory.init_fn = [_v = std::move(initial_value)](void* pv) {
            *(ValueTy_*)pv = _v;
        };

        prop._add_attr(std::move(attr));
    }
};

} // namespace kangsw::refl