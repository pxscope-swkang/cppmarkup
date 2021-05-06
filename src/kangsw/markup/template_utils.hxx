#pragma once
#include <type_traits>
#include <map>

namespace kangsw::templates
{
// https://stackoverflow.com/questions/31762958/check-if-class-is-a-template-specialization
template <class T, template <class...> class Template>
struct is_specialization : std::false_type {
};

template <template <class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : std::true_type {
};

// https://stackoverflow.com/questions/60113615/how-to-check-if-a-variable-is-a-map-in-c
template <typename T>
struct is_map : std::false_type {};

template <typename Key, typename Value, typename Order, typename Allocator>
struct is_map<std::map<Key, Value, Order, Allocator>> : std::true_type {};

}