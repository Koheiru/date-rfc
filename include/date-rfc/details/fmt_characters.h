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
#include "static_string.h"
#include "iterator_traits.h"

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
//                                types: characters
// ----------------------------------------------------------------------------
template <class Char, std::size_t Length>
struct characters_t
{
    using value_type = static_string<Char, Length>;

    enum : unsigned { min_length = Length };
    enum : unsigned { max_length = Length };
    enum : bool { need_cache = false };

    characters_t(value_type&& value) : value(std::move(value)) {}
    ~characters_t() = default;

    const value_type value;
};

// ----------------------------------------------------------------------------
template <class Char, class InputChar, std::size_t InputLength>
static characters_t<Char, InputLength> characters(const InputChar (&value)[InputLength])
{
    return characters_t<Char, InputLength>{ static_string<Char, InputLength>(value) };
}

// ----------------------------------------------------------------------------
template <class Iterator, class Char, std::size_t Length, class ...Others>
bool read_impl(Iterator& pos, const Iterator& end, const characters_t<Char, Length>& fmt, Others&&... others)
{
    using char_type = typename iterator_traits<Iterator>::value_type;

    for (std::size_t i = 0; i < fmt.value.length(); ++i, ++pos)
    {
        if (pos == end)
            return false;

        const char_type ch = *pos;
        if (ch != fmt.value[i])
            return false;
    }

    return read_impl(pos, end, std::forward<Others>(others)...);
}

} // namespace date

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
