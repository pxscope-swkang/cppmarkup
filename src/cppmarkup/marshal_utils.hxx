#pragma once
#include "container_proxy.hxx"
#include "object.hpp"

namespace kangsw::markup {
namespace impl {
    template <class Ty_, class Vp_, class FnElem_, class FnArray_, class FnMap_>
    decltype(auto) select_type_handler(
        property const& prop,
        Vp_ memory,
        FnElem_&& fne,
        FnArray_&& fna,
        FnMap_&& fnm)
    {
        enum { is_const = std::is_const_v<std::remove_pointer_t<Vp_>> };
        using value_type = std::conditional_t<is_const, Ty_ const, Ty_>;

        if (prop.type.is_array()) {
            if constexpr (std::is_base_of_v<object, Ty_>) {
                return fna(make_object_array_proxy(prop.as_array(), memory));
            }
            else {
                return fna(make_primitive_array_proxy<Ty_>(memory));
            }
        }
        if (prop.type.is_map()) {
            // TODO
            if constexpr (std::is_base_of_v<object, Ty_>) {
            }
            else {
            }
            assert(false);
        }

        return fne(*(value_type*)memory);
    }

    template <class Ty_, class Vp_, class FnElem_>
    decltype(auto) select_type_handler_attr(Vp_ memory, FnElem_&& fne)
    {
        enum { is_const = std::is_const_v<std::remove_pointer_t<Vp_>> };
        using value_type = std::conditional_t<is_const, Ty_ const, Ty_>;

        return fne(*(value_type*)memory);
    }
} // namespace impl

/** Selects runtime object pointer as concrete template types */
template <class Vp_, class FnElem_, class FnArray_, class FnMap_>
decltype(auto) select_type_handler(
    property const& prop,
    Vp_ memory,
    FnElem_&& fne,
    FnArray_&& fna,
    FnMap_&& fnm)
{
    switch (auto& ty = prop.type; ty.value_type()) {
        case element_type::boolean:
            return impl::select_type_handler<bool>(
                prop, memory,
                std::forward<FnElem_>(fne), std::forward<FnArray_>(fna), std::forward<FnMap_>(fnm));

        case element_type::integer:
            return impl::select_type_handler<int64_t>(
                prop, memory,
                std::forward<FnElem_>(fne), std::forward<FnArray_>(fna), std::forward<FnMap_>(fnm));

        case element_type::floating_point:
            return impl::select_type_handler<double>(
                prop, memory,
                std::forward<FnElem_>(fne), std::forward<FnArray_>(fna), std::forward<FnMap_>(fnm));

        case element_type::string:
            return impl::select_type_handler<u8string>(
                prop, memory,
                std::forward<FnElem_>(fne), std::forward<FnArray_>(fna), std::forward<FnMap_>(fnm));

        case element_type::binary:
            return impl::select_type_handler<binary_chunk>(
                prop, memory,
                std::forward<FnElem_>(fne), std::forward<FnArray_>(fna), std::forward<FnMap_>(fnm));

        case element_type::object:
            return impl::select_type_handler<object>(
                prop, memory,
                std::forward<FnElem_>(fne), std::forward<FnArray_>(fna), std::forward<FnMap_>(fnm));

        default: assert(false && "Should not enter this code");
        case element_type::null:
            return impl::select_type_handler<nullptr_t>(
                prop, memory,
                std::forward<FnElem_>(fne), std::forward<FnArray_>(fna), std::forward<FnMap_>(fnm));
    }
}

/** Attribute version of select_type_handler */
template <class Vp_, class FnElem_>
decltype(auto) select_type_handler_attr(
    element_type const& ty,
    Vp_ memory,
    FnElem_&& fne)
{
    assert(!ty.is_array());
    assert(!ty.is_map());
    assert(!ty.is_object());

    // clang-format off
    switch (ty.value_type()) {
        case element_type::boolean: return impl::select_type_handler_attr<bool>     (memory, std::forward<FnElem_>(fne));
        case element_type::integer: return impl::select_type_handler_attr<int64_t>  (memory, std::forward<FnElem_>(fne));
        case element_type::floating_point: 
            return impl::select_type_handler_attr<double>(memory, std::forward<FnElem_>(fne));
        case element_type::string:  return impl::select_type_handler_attr<u8string> (memory, std::forward<FnElem_>(fne));
        case element_type::binary:  
            return impl::select_type_handler_attr<binary_chunk>(memory, std::forward<FnElem_>(fne));
                                    
        case element_type::object:  
        default: assert(false && "Should not enter this code");
        case element_type::null:    return impl::select_type_handler_attr<nullptr_t>(memory, std::forward<FnElem_>(fne));
    }
    // clang-format on
}

} // namespace kangsw::markup
