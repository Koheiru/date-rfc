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
//                             types: cases branch
// ----------------------------------------------------------------------------
template <class ...Formatters>
struct branch_t
{
    enum : unsigned { min_length = args_traits<Formatters...>::min_length };
    enum : unsigned { max_length = args_traits<Formatters...>::max_length };
    enum : bool { need_cache = false };

    branch_t(Formatters&&... formatters) : formatters(std::forward<Formatters>(formatters)...) {}
    ~branch_t() = default;

    std::tuple<Formatters...> formatters;
};

// ----------------------------------------------------------------------------
template <class ...Formatters>
branch_t<Formatters...> branch(Formatters&&... formatters)
{
    return branch_t<Formatters...>(std::forward<Formatters>(formatters)...);
}

// ----------------------------------------------------------------------------
template <class Iterator, class ...Formatters>
bool read_impl(Iterator& pos, const Iterator& end, branch_t<Formatters...>& fmt)
{
    return read_impl(pos, end, fmt.formatters, std_impl::make_index_sequence<sizeof...(Formatters)>{});
}

// ----------------------------------------------------------------------------
//                               types: cases
// ----------------------------------------------------------------------------
template <class ...Branches>
struct cases_t
{
    enum : unsigned { min_length = cases_traits<Branches...>::min_length };
    enum : unsigned { max_length = cases_traits<Branches...>::max_length };
    enum : bool { need_cache = true };

    cases_t(Branches&&... branches) : branches(std::forward<Branches>(branches)...) {}
    ~cases_t() = default;

    std::tuple<Branches...> branches;
};

// ----------------------------------------------------------------------------
template <class ...Branches>
cases_t<Branches...> cases(Branches&&... branches)
{
    return cases_t<Branches...>(std::forward<Branches>(branches)...);
}

// ----------------------------------------------------------------------------
template <std::size_t Count, std::size_t N>
struct cases_reader_t
{
    template <class Iterator, class ...Branches>
    static bool read(Iterator& pos, const Iterator& end, cases_t<Branches...>& fmt)
    {
        Iterator begin = pos;
        if (read_impl(pos, end, std::get<Count - N>(fmt.branches)))
            return true;
        
        pos = std::move(begin);
        return cases_reader_t<Count, N - 1>::read(pos, end, fmt);
    }
};

template <std::size_t Count>
struct cases_reader_t<Count, 0>
{
    template <class Iterator, class ...Branches>
    static bool read(Iterator&, const Iterator&, cases_t<Branches...>&)
    {
        return false;
    }
};

// ----------------------------------------------------------------------------
template <class Iterator, class ...Branches, class ...Others>
bool read_impl(Iterator& pos, const Iterator& end, cases_t<Branches...>& fmt, Others&&... others)
{
    enum : std::size_t { branches_count = sizeof...(Branches) };
    if (!cases_reader_t<branches_count, branches_count>::read(pos, end, fmt))
        return false;
    return read_impl(pos, end, std::forward<Others>(others)...);
}

} // namespace date

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
