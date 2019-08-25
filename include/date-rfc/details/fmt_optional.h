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
#include <utility>
#include "fmt_traits.h"
#include "iterator_traits.h"
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
//                               types: optional
// ----------------------------------------------------------------------------
template <class ...Formatters>
struct optional_t
{
    enum : unsigned { min_length = 0 };
    enum : unsigned { max_length = args_traits<Formatters...>::max_length };
    enum : bool { need_cache = true };

    optional_t(Formatters&&... formatters) : formatters(std::forward<Formatters>(formatters)...) {}
    ~optional_t() = default;

    std::tuple<Formatters...> formatters;
};

// ----------------------------------------------------------------------------
template <class ...Formatters>
optional_t<Formatters...> optional(Formatters&&... formatters)
{
    return optional_t<Formatters...>(std::forward<Formatters>(formatters)...);
}

// ----------------------------------------------------------------------------
template <class Iterator, class ...Formatters, class ...Others>
bool read_impl(Iterator& pos, const Iterator& end, optional_t<Formatters...>& fmt, Others&&... others)
{
    Iterator begin = pos;
    if (!read_impl(pos, end, fmt.formatters, std_impl::make_index_sequence<sizeof...(Formatters)>{}))
    {
        //! TODO: unset values of the optional formatters.
        pos = std::move(begin);
    }
    return read_impl(pos, end, std::forward<Others>(others)...);
}

} // namespace date

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
