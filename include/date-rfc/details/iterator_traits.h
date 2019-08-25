#pragma once
#include <algorithm>
#include <iterator>
#include <type_traits>

// ----------------------------------------------------------------------------
namespace date
{

// ----------------------------------------------------------------------------
template <class T, class = void>
struct iterator_traits
{
    using value_type = typename std::iterator_traits<T>::value_type;
};

// ----------------------------------------------------------------------------
template <class Container>
struct iterator_traits<std::back_insert_iterator<Container>, void>
{
    using value_type = typename Container::value_type;
};

// ----------------------------------------------------------------------------
template <class Container>
struct iterator_traits<std::front_insert_iterator<Container>, void>
{
    using value_type = typename Container::value_type;
};

// ----------------------------------------------------------------------------
template <class Container>
struct iterator_traits<std::insert_iterator<Container>, void>
{
    using value_type = typename Container::value_type;
};

// ----------------------------------------------------------------------------
template <class ...Args>
struct iterator_traits<std::istream_iterator<Args...>>
{
    using value_type = typename std::istream_iterator<Args...>::char_type;
};

// ----------------------------------------------------------------------------
template <class ...Args>
struct iterator_traits<std::ostream_iterator<Args...>>
{
    using value_type = typename std::ostream_iterator<Args...>::char_type;
};

// ----------------------------------------------------------------------------
template <class ...Args>
struct iterator_traits<std::istreambuf_iterator<Args...>>
{
    using value_type = typename std::istreambuf_iterator<Args...>::char_type;
};

// ----------------------------------------------------------------------------
template <class ...Args>
struct iterator_traits<std::ostreambuf_iterator<Args...>>
{
    using value_type = typename std::ostreambuf_iterator<Args...>::char_type;
};

} // namespace date
