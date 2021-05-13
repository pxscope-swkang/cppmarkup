#pragma once
#include "object.hxx"

/**
 * Static object interface ...
 *
 * 
 */
namespace kangsw::refl {

using static_object_list = templates::singleton<std::vector<object_traits const*>>;

template <typename ObjTy_>
class static_object_traits : public object_traits {
private:
    friend struct templates::singleton<static_object_traits, ObjTy_>;
    static_object_traits() = default;

    static inline struct register_traits {
        register_traits() {
            static_object_list::get().push_back(&get());
        }
    } _reg;

public:
    static auto& get() {
        return templates::singleton<static_object_traits, ObjTy_>::get();
    }

    std::unique_ptr<object> create_empty_object() override {
        return std::make_unique<ObjTy_>();
    }
};

/**  */
template <typename Ty_>
class static_object_base : public object {
public:
    using self_type   = Ty_;
    using traits_type = static_object_traits<Ty_>;

public:
    object_traits const& traits() const override { return traits_type::get(); }
    static Ty_ get_default() {
        Ty_ o;
        return o.reset(), o;
    }

    void* _base() override { return this; }
    void const* _base() const override { return this; }
};

/** Container proxies */
template <typename ObjTy_>
class static_object_vector_iface : public object_vector_interface {
    static_assert(std::is_base_of_v<object, ObjTy_>);
    using ptr  = std::vector<ObjTy_>*;
    using cptr = std::vector<ObjTy_> const*;

public:
    size_t size(void const* p) const override {
        return static_cast<cptr>(p)->size();
    }

    object const& at(void const* p, size_t i) const override {
        return static_cast<cptr>(p)->operator[](i);
    }

    object& at(void* p, size_t i) const override {
        return static_cast<ptr>(p)->operator[](i);
    }

    object& push_back(void* p) const override {
        return static_cast<ptr>(p)->emplace_back();
    }

    void erase(void* p, size_t from, size_t to) const override {
        auto& vec = *static_cast<ptr>(p);
        vec.erase(vec.begin() + from, vec.begin() + to);
    }

    void reserve(void* p, size_t new_size) override {
        static_cast<ptr>(p)->reserve(new_size);
    }
};

template <typename ObjTy_>
class static_object_map_iface : public object_map_interface {
    static_assert(std::is_base_of_v<object, ObjTy_>);
    using ptr  = u8str_map<ObjTy_>*;
    using cptr = u8str_map<ObjTy_> const*;

public:
    size_t size(void const* p) const override {
        return static_cast<cptr>(p)->size();
    }

    object const& at(void const* p, u8str_view s) const override {
        if (auto found = find(p, s)) { return *found; }
        throw std::out_of_range{""};
    }

    object& at(void* p, u8str_view s) const override {
        if (auto found = find(p, s)) { return *found; }
        throw std::out_of_range{""};
    }

    object const* find(void const* p, u8str_view s) const override {
        auto& map = *static_cast<cptr>(p);
        auto it   = map.find(s);
        if (it == map.end()) { return nullptr; }
        return &it->second;
    }

    object* find(void* p, u8str_view s) const override {
        return const_cast<object*>(find(static_cast<void const*>(p), s));
    }

    object& insert(void* p, u8str_view s) const override {
        if (auto found = find(p, s)) { return *found; }
        return (*static_cast<ptr>(p)).try_emplace(u8str(s)).first->second;
    }

    void erase(void* p, u8str_view s) const override {
        // static_cast<ptr>(p)->erase(s);
    }

    void for_each(void* p, std::function<void(u8str_view, object&)> const& fn) const override {
        for (auto& pair : *static_cast<ptr>(p)) { fn(pair.first, pair.second); }
    }

    void for_each(void const* p, std::function<void(u8str_view, object const&)> const& fn) const override {
        for (auto& pair : *static_cast<cptr>(p)) { fn(pair.first, pair.second); }
    }
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
        using traits_type = static_object_traits<ObjTy_>;
        auto& prop        = traits_type::get().find_or_add_property(tag);

        constexpr auto type = etype::from_type<ValueTy_>();
        property::memory_t m;
        m.type   = type;
        m.size   = sizeof(ValueTy_);
        m.offset = offset;

        m.init_fn = [_v = std::move(initial_value)](void* pv) {
            *(ValueTy_*)pv = _v;
        };

        prop._set_defaults("" /*TODO ...*/, (property_flag_t)elem_flags, std::move(m));

        if constexpr (type.is_object()) {
            if constexpr (type.is_array()) {
                prop._set_ovi(new static_object_vector_iface<typename ValueTy_::value_type>);
            }
            if constexpr (type.is_map()) {
                prop._set_omi(new static_object_map_iface<typename ValueTy_::mapped_type>);
            }

            prop._set_traits(&traits_type::get());
        }
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
        using traits_type = static_object_traits<ObjTy_>;
        auto& prop        = traits_type::get().find_or_add_property(tag);

        property::attribute attr;
        attr.name           = std::move(name);
        attr._memory.size   = sizeof(ValueTy_);
        attr._memory.offset = offset;
        attr._memory.type   = etype::from_type<ValueTy_>();

        constexpr auto type = etype::from_type<ValueTy_>();
        static_assert(!type.is_container());
        static_assert(!type.is_object());

        attr._memory.init_fn = [_v = std::move(initial_value)](void* pv) {
            *(ValueTy_*)pv = _v;
        };

        prop._add_attr(std::move(attr));
    }
};

} // namespace kangsw::refl