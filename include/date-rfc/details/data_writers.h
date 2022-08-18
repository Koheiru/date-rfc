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

#include "iterator_traits.h"
#include "static_string.h"

// ----------------------------------------------------------------------------
namespace date
{

// ----------------------------------------------------------------------------
struct characters_writer
{
    template <class Iterator, class Char, std::size_t Length>
    static void write(Iterator& dst, const static_string<Char, Length>& value)
    {
        for (const Char* str = value.c_str(); *str != Char{ '\0' }; ++str)
            *(dst++) = *str;
    }

    template <class Iterator, class Char>
    static void write(Iterator& dst, const Char* value)
    {
        write(dst, make_static_string(value));
    }

    template <class Iterator, class Char>
    static void write(Iterator& dst, Char value)
    {
        *(dst++) = value;
    }
};

// ----------------------------------------------------------------------------
template <int Width>
struct number_writer;

template <>
struct number_writer<0>
{
    template <class Char>
    static constexpr static_string<Char, 11> digits()
    {
        return make_static_string<Char>("0123456789");
    }

    template <class Iterator, class Number>
    static void write(Iterator& dst, Number value)
    {
        using char_type = typename iterator_traits<Iterator>::value_type;
        constexpr auto digits = number_writer<0>::digits<char_type>();
        if (value == 0)
            return;
        number_writer<0>::write(dst, value / 10);
        *(dst++) = digits[value % 10];
    }
};

template <int Width>
struct number_writer
{
    template <class Iterator, class Number>
    static void write(Iterator& dst, Number value)
    {
        using char_type = typename iterator_traits<Iterator>::value_type;
        constexpr auto digits = number_writer<0>::digits<char_type>();
        if (value == 0)
        {
            for (int i = 0; i < Width - 1; ++i)
                *(dst++) = digits[0];
            *(dst++) = digits[0];
            return;
        }
        number_writer<Width - 1>::write(dst, value / 10);
        *(dst++) = digits[value % 10];
    }
};

} // namespace date
