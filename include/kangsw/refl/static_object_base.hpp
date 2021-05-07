#pragma once
#include "object.hpp"

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
    object_traits_base const& traits() override { return traits_type::get(); }
};

/** Performs element registration */
template <typename ObjTy_, typename ValueTy_, typename HashTy_>
class element_regestration_t {
public:
    element_regestration_t(
        u8str&& tag,
        size_t offset,
        ValueTy_&& initial_value) //
    {
    }
};

/** Performs attribute registration */
template <typename ObjTy_, typename ElemTy_, typename ValueTy_, typename HashTy_>
class attribute_registration_t {
public:
    attribute_registration_t(
        u8str&& name,
        size_t offset,
        ValueTy_&& initial_value) //
    {
    }
};

} // namespace kangsw::refl