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
#include "iterator_traits.h"
#include "index_sequence.h"
#include "input_wrapper.h"
#include "fmt_traits.h"
#include "fmt_character.h"
#include "fmt_characters.h"
#include "fmt_unsigned_integer.h"
#include "fmt_signed_integer.h"
#include "fmt_aliases.h"
#include "fmt_optional.h"
#include "fmt_cases.h"

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
template <class Iterator>
Iterator& skip_spaces(Iterator& pos, const Iterator& end)
{
    using char_type = typename iterator_traits<Iterator>::value_type;
    for (; pos != end; ++pos)
    {
        const char_type ch = *pos;
        constexpr auto spaces = make_static_string<char_type>(" \t\n\v\f\r");
        if (!spaces.contains(ch))
            break;
    }
    return pos;
}

// ----------------------------------------------------------------------------
template <class ...Formatters>
std::tuple<Formatters...> format(Formatters&& ...formatters)
{
    return std::make_tuple(std::forward<Formatters>(formatters)...);
}

// ----------------------------------------------------------------------------
template <class Iterator, class ...Formatters>
bool read(Iterator& pos, const Iterator& end, std::tuple<Formatters...>& format)
{
    enum : unsigned { max_length = args_traits<Formatters...>::max_length };
    enum : bool { need_cache = args_traits<Formatters...>::need_cache && input_traits<Iterator>::need_cache };
    
    input_wrapper_t<Iterator, max_length, need_cache> wrapper(pos, end);
    return read_impl(wrapper.begin(), wrapper.end(), format, std_impl::make_index_sequence<sizeof...(Formatters)>{});
}

} // namespace date

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
