#pragma once
#include <type_traits>
#include <map>

namespace kangsw::templates {
// https://stackoverflow.com/questions/31762958/check-if-class-is-a-template-specialization
template <class T, template <class...> class Template>
struct is_specialization_of : std::false_type {
};

template <template <class...> class Template, class... Args>
struct is_specialization_of<Template<Args...>, Template> : std::true_type {
};

// https://stackoverflow.com/questions/60113615/how-to-check-if-a-variable-is-a-map-in-c
template <typename T>
struct is_map : std::false_type {};

template <typename Key, typename Value, typename Order, typename Allocator>
struct is_map<std::map<Key, Value, Order, Allocator>> : std::true_type {};

struct singleton_common_t {};

template <typename Type_, typename HashTy_ = singleton_common_t>
struct singleton {
    static auto& get() {
        static Type_ v;
        return v;
    }
};

template <typename Ty_>
class invoke_here {
    static inline struct _invoke_t {
        _invoke_t() {
            Ty_{}();
        }
    } _invoke;
};

} // namespace kangsw::templates