#pragma once
#include "object.hpp"

/**
 * Since we can't get actual class of each object class instance in compile time,
 *we can not have any precise reference for any object container.
 * Thus below classes introduce kind of proxy methods, which simply wraps all
 *container access operations interanlly, with same interface of stl containers.
 */
namespace kangsw::markup {

/**
 * TODO: object vector와 일반 값 타입 vector이 공통된 템플릿 인터페이스를 제공할 수 있게 합니다.
 */
template <typename Ty_, typename Vp_>
class array_proxy {
    static_assert(std::is_same_v<void, std::remove_const_t<std::remove_pointer_t<Vp_>>>);
    enum { _is_const_array = std::is_const_v<std::remove_pointer_t<Vp_>> };

public:
    using container_type  = std::conditional_t<_is_const_array, const std::vector<Ty_>, std::vector<Ty_>>;
    using const_reference = typename container_type::const_reference;
    using reference       = std::conditional_t<_is_const_array, const_reference, typename container_type::reference>;

public:
    array_proxy(Vp_ vp)
        : _m(reinterpret_cast<container_type*>(vp))
    {}

    auto empty() const { return _m->empty(); }
    auto size() const { return _m->size(); }

    const_reference operator[](size_t i) const { return _m->operator[](i); }
    reference operator[](size_t i) { return _m->operator[](i); }
    const_reference at(size_t i) const { return _m->at(i); }
    reference at(size_t i) { return _m->at(i); }

    reference append() { return _m->emplace_back(); }
    auto reserve(size_t count) { return _m->reserve(count); }
    size_t erase(size_t from, size_t to) { return _m->erase(_m->begin() + from, _m->begin() + to) - _m->begin(); }
    size_t erase(size_t pos) { return _m->erase(_m->begin() + pos) - _m->begin(); }

    template <class Fn_>
    auto for_each(Fn_&& fn)
    {
        for (size_t i = 0, end = size(); i < end; ++i) {
            if constexpr (std::is_same_v<decltype(fn(at(i))), void>) {
                fn(at(i));
            }
            else {
                if (!fn(at(i))) {
                    return;
                }
            }
        }
    }

    template <class Fn_>
    auto for_each(Fn_&& fn) const
    {
        for (size_t i = 0, end = size(); i < end; ++i) {
            if constexpr (std::is_same_v<decltype(fn(at(i))), void>) {
                fn(at(i));
            }
            else {
                if (!fn(at(i))) {
                    return;
                }
            }
        }
    }

private:
    container_type* _m;
};

template <>
class array_proxy<object, void const*> {
public:
    using const_reference = object const&;
    using reference       = object&;

public:
    array_proxy(object_vector_manip const* api, void const* objarr)
        : _f(api)
        , _vp(objarr)
    {}

    auto size() const { return _f->size(_vp); }
    auto empty() const { return size() == 0; }

    auto& operator[](size_t i) const { return *_f->at(_vp, i); }
    auto& at(size_t i) const { return *_f->at(_vp, i); }

    template <class Fn_>
    auto for_each(Fn_&& fn) const
    {
        for (size_t i = 0, end = size(); i < end; ++i) {
            if constexpr (std::is_same_v<decltype(fn(at(i))), void>) {
                fn(at(i));
            }
            else {
                if (!fn(at(i))) {
                    return;
                }
            }
        }
    }

protected:
    object_vector_manip const* _f;
    void const* _vp;
};

/**
 * 해당 클래스는 const 버전의 기능을 확장하는 식으로 구현됩니다.
 */
template <>
class array_proxy<object, void*> : public array_proxy<object, void const*> {
public:
    array_proxy(object_vector_manip const* api, void* objarr)
        : array_proxy<object, void const*>(api, objarr)
    {}

    auto& operator[](size_t i) { return *_f->at(vp(), i); }
    auto& at(size_t i) { return *_f->at(vp(), i); }

    auto& append()
    {
        object* o;
        _f->append(vp(), &o);
        return o;
    }

    auto reserve(size_t n) { _f->reserve(vp(), n); }
    auto erase(size_t from, size_t to) { return _f->erase(vp(), from, to); }
    auto erase(size_t pos) { return erase(pos, pos + 1); }

    template <class Fn_>
    auto for_each(Fn_&& fn)
    {
        for (size_t i = 0, end = size(); i < end; ++i) {
            if constexpr (std::is_same_v<decltype(fn(at(i))), void>) {
                fn(at(i));
            }
            else {
                if (!fn(at(i))) {
                    return;
                }
            }
        }
    }

private:
    void* vp() { return (void*)_vp; }
};

template <typename Ty_, typename Vp_>
decltype(auto) make_primitive_array_proxy(Vp_ ptr) { return array_proxy<Ty_, Vp_>(ptr); }

template <typename Vp_>
decltype(auto) make_object_array_proxy(object_vector_manip const* api, Vp_ ptr) { return array_proxy<object, Vp_>(api, ptr); }

/**
 * TODO: std::map<u8string, object&> 와 같은 함수 시그네쳐
 */
} // namespace kangsw::markup