#pragma once
#include "object.hxx"

namespace kangsw::refl {

class property_type_mismatch_exception : public std::logic_error {
public:
    using std::logic_error::logic_error;

    template <typename Ty_>
    static void verify(property::memory_t const& m) {
        if (m.type != etype::from_type_exact<Ty_>()) {
            throw property_type_mismatch_exception{"Type mismatch"};
        }
    }
};

// Supports non-const, const version both.
template <typename Ty_, bool Constant_>
class property_proxy {
public:
    using void_pointer = std::conditional_t<Constant_, void const*, void*>;
    using src_type     = std::conditional_t<Constant_, Ty_ const, Ty_>;
    using pointer      = src_type*;
    using reference    = src_type&;

    property_proxy(property const& m, void_pointer ptr) : _p(static_cast<pointer>(ptr)) {
        property_type_mismatch_exception::verify<Ty_>(m.memory());
    }
    property_proxy(property::attribute const& m, void_pointer ptr) : _p(static_cast<pointer>(ptr)) {
        property_type_mismatch_exception::verify<Ty_>(m.memory);
    }

public:
    reference operator*() const { return *_p; }
    pointer operator->() const { return _p; }

private:
    pointer _p;
};

template <typename ValueTy_, bool Constant_>
class property_proxy<std::vector<ValueTy_>, Constant_> {
public:
    using Ty_ = std::vector<ValueTy_>;

    using void_pointer = std::conditional_t<Constant_, void const*, void*>;
    using src_type     = std::conditional_t<Constant_, Ty_ const, Ty_>;
    using pointer      = src_type*;
    using reference    = src_type&;

    using value_type = typename src_type::value_type;

public:
    property_proxy(property const& m, void_pointer ptr) : _p(static_cast<pointer>(ptr)) {
        property_type_mismatch_exception::verify<Ty_>(m.memory());
    }

    auto size() const { return _p->size(); }
    auto empty() const { return _p->empty(); }
    auto& emplace_back() { return _p->emplace_back(); }

    auto& operator[](size_t i) { return _p->operator[](i); }
    auto& operator[](size_t i) const { return static_cast<Ty_ const*>(_p)->operator[](i); }

    void reserve(size_t c) { _p->reserve(c); }

    void erase(size_t from, size_t to) { _p->erase(_p->begin() + from, _p->begin() + to); }
    void erase(size_t at) { erase(at, at + 1); }

private:
    pointer _p;
};

template <bool Constant_>
class property_proxy<std::vector<object>, Constant_> {
public:
    using Vty_         = object;
    using void_pointer = std::conditional_t<Constant_, void const*, void*>;

public:
    property_proxy(property const& m, void_pointer ptr) : _if(m.ovi()), _p(ptr) {
        assert(_if);
        property_type_mismatch_exception::verify<std::vector<Vty_>>(m.memory());
    }

    auto size() const { return _if->size(_p); }
    auto empty() const { return size() == 0; }
    auto& emplace_back() { return _if->push_back(_p); }

    auto& operator[](size_t i) { return _if->at(_p, i); }
    auto& operator[](size_t i) const { return _if->at(static_cast<void const*>(_p), i); }

    void reserve(size_t c) { _if->reserve(_p, c); }

    void erase(size_t from, size_t to) { _if->erase(_p, from, to); }
    void erase(size_t at) { _if->erase(_p, at, at + 1); }

private:
    object_vector_interface const* _if;
    void_pointer _p;
};

template <typename ValueTy_, bool Constant_>
class property_proxy<u8str_map<ValueTy_>, Constant_> {
public:
    using Ty_ = u8str_map<ValueTy_>;

    using void_pointer  = std::conditional_t<Constant_, void const*, void*>;
    using src_type      = std::conditional_t<Constant_, Ty_ const, Ty_>;
    using pointer       = src_type*;
    using const_pointer = Ty_ const*;
    using reference     = src_type&;

    property_proxy(property const& m, void_pointer ptr) : _p(static_cast<pointer>(ptr)) {
        property_type_mismatch_exception::verify<Ty_>(m.memory());
    }

public:
    auto size() const { return _p->size(); }
    auto& at(u8str_view s) { return _p->at(s); }
    auto& at(u8str_view s) const { return static_cast<const_pointer>(_p)->at(s); }

    auto find(u8str_view s) const {
        auto it = static_cast<const_pointer>(_p)->find(s);
        if (it == static_cast<const_pointer>(_p)->end()) { return nullptr; }
        return &it->second;
    }

    auto find(u8str_view s) {
        auto it = _p->find(s);
        if (it == _p->end()) { return nullptr; }
        return &it->second;
    }

    auto& insert(u8str_view s) { return _p[s]; }
    void erase(u8str_view s) { _p->erase(s); }

    template <typename Fn_>
    void for_each(Fn_&& fn) {
        for (auto& pair : *_p) { fn(pair.first, pair.second); }
    }

    template <typename Fn_>
    void for_each(Fn_&& fn) const {
        for (auto& pair : *static_cast<const_pointer>(_p)) { fn(pair.first, pair.second); }
    }

private:
    pointer _p;
};

template <bool Constant_>
class property_proxy<u8str_map<object>, Constant_> {
public:
    using Vty_          = object;
    using void_pointer  = std::conditional_t<Constant_, void const*, void*>;
    using const_pointer = void const*;

    property_proxy(property const& m, void_pointer ptr) : _if(m.omi()), _p(ptr) {
        assert(_if);
        property_type_mismatch_exception::verify<u8str_map<Vty_>>(m.memory());
    }

public:
    auto size() const { return _if->size(_p); }
    auto& at(u8str_view s) { return _if->at(_p, s); }
    auto& at(u8str_view s) const { return _if->at(static_cast<const_pointer>(_p), s); }
    auto find(u8str_view s) { return _if->find(_p, s); }
    auto find(u8str_view s) const { return _if->find(static_cast<const_pointer>(_p), s); }
    auto& insert(u8str_view s) { return _if->insert(_p); }
    void erase(u8str_view s) { _if->erase(_p, s); }

    template <typename Fn_>
    void for_each(Fn_&& fn) { _if->for_each(_p, std::forward<Fn_>(fn)); }

    template <typename Fn_>
    void for_each(Fn_&& fn) const { _if->for_each(const_cast<const_pointer>(_p), std::forward<Fn_>(fn)); }

private:
    object_map_interface const* _if;
    void_pointer _p;
};

template <typename Ty_, bool Constant_>
using array_proxy = property_proxy<std::vector<Ty_>, Constant_>;

template <typename Ty_, bool Constant_>
using map_proxy = property_proxy<u8str_map<Ty_>, Constant_>;

/** */
template <typename Ty_, typename ObjTy_>
auto make_proxy(ObjTy_* obj, property const& m) {
    enum { is_constant = std::is_const_v<ObjTy_> };
    return property_proxy<Ty_, is_constant>{m, m.memory()(obj->base())};
}
template <typename Ty_, typename ObjTy_>
auto make_proxy(ObjTy_* obj, property::attribute const& m) {
    enum { is_constant = std::is_const_v<ObjTy_> };
    return property_proxy<Ty_, is_constant>{m, m.memory(obj->base())};
}

} // namespace kangsw::refl

void pewpew() {
    using namespace kangsw::refl;
    auto r = make_proxy<object>((object*)nullptr, property::attribute{""});
}