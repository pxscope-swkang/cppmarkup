#pragma once
#include <type_traits>

namespace kangsw::templates
{
// https://stackoverflow.com/questions/31762958/check-if-class-is-a-template-specialization
template <class T, template <class...> class Template>
struct is_specialization : std::false_type {
};

template <template <class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : std::true_type {
};
}