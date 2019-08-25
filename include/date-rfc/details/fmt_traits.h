//
// The MIT License (MIT)
//
// Copyright (c) 2019 Yury Prostov
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#pragma once
#include <tuple>
#include <type_traits>
#include "index_sequence.h"

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpedantic"
# if __GNUC__ < 5
//! GCC 4.9 Bug 61489 Wrong warning with -Wmissing-field-initializers.
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
# endif
#endif

// ----------------------------------------------------------------------------
namespace date
{

// ----------------------------------------------------------------------------
//                                type traits
// ----------------------------------------------------------------------------
template <class T, class ...Args>
struct args_traits;

template <class T>
struct args_traits<T>
{
    enum : unsigned { min_length = T::min_length };
    enum : unsigned { max_length = T::max_length };
    enum : bool { need_cache = T::need_cache };
};

template <class T, class ...Args>
struct args_traits
{
    enum : unsigned { min_length = T::min_length + args_traits<Args...>::min_length };
    enum : unsigned { max_length = T::max_length + args_traits<Args...>::max_length };
    enum : bool { need_cache = T::need_cache | args_traits<Args...>::need_cache };
};

// ----------------------------------------------------------------------------
enum : bool { SignOptional = false, SignRequired = true };

template <bool SignRequired>
struct sign_traits;

template <>
struct sign_traits<SignOptional>
{
    enum : unsigned { min_length = 0 };
    enum : unsigned { max_length = 1 };
};

template <>
struct sign_traits<SignRequired>
{
    enum : unsigned { min_length = 1 };
    enum : unsigned { max_length = 1 };
};

// ----------------------------------------------------------------------------
#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wenum-compare"
#endif  // __GNUC__

template <class T, class ...Args>
struct cases_traits;

template <class T1, class T2>
struct cases_traits<T1, T2>
{
    using min_type = typename std::conditional<(T1::min_length < T2::min_length), T1, T2>::type;
    using max_type = typename std::conditional<(T1::max_length > T2::max_length), T1, T2>::type;

    enum : unsigned { min_length = min_type::min_length };
    enum : unsigned { max_length = max_type::max_length };
};

template <class T, class ...Args>
struct cases_traits
{
    using _args_min_type = typename cases_traits<Args...>::min_type;
    using _args_max_type = typename cases_traits<Args...>::max_type;
    using min_type = typename std::conditional<(T::min_length < _args_min_type::min_length), T, _args_min_type>::type;
    using max_type = typename std::conditional<(T::max_length > _args_max_type::max_length), T, _args_max_type>::type;

    enum : unsigned { min_length = min_type::min_length };
    enum : unsigned { max_length = max_type::max_length };
};

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif  // __GNUC__

// ----------------------------------------------------------------------------
//                            common functions
// ----------------------------------------------------------------------------
template <class Iterator, class Format, std::size_t... Indexes>
bool read_impl(Iterator& pos, const Iterator& end, Format& format, std_impl::index_sequence<Indexes...>)
{
    return read_impl(pos, end, std::get<Indexes>(format)...);
}

// ----------------------------------------------------------------------------
template <class Iterator>
bool read_impl(Iterator& pos, const Iterator& end)
{
    return true;
}

} // namespace date

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
