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
#include "fmt_unsigned_integer.h"
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
//                           types: fraction
// ----------------------------------------------------------------------------
template <class UnsignedInt, unsigned Width>
struct fraction_t
{
    using value_type = UnsignedInt;

    enum : unsigned { min_length = 1 };
    enum : unsigned { max_length = Width };
    enum : bool { need_cache = false };

    fraction_t(value_type& value) : value(value) {}
    ~fraction_t() = default;

    value_type& value;
};

// ----------------------------------------------------------------------------
template <unsigned Width, class UnsignedInt>
fraction_t<UnsignedInt, Width> fraction(UnsignedInt& value)
{
    return fraction_t<UnsignedInt, Width>{ value };
}

// ----------------------------------------------------------------------------
template <class Iterator, class UnsignedInt, unsigned Width, class ...Others>
bool read_impl(Iterator& pos, const Iterator& end, fraction_t<UnsignedInt, Width>& fmt, Others&&... others)
{
    using char_type  = typename iterator_traits<Iterator>::value_type;

    auto x = UnsignedInt{ 0 };
    auto count = std::size_t{ 0 };
    while (count < Width)
    {
        if (pos == end)
            break;

        const char_type ch = *pos;
        if ((ch < char_type{ '0' }) || (ch > char_type{ '9' }))
            break;

        x = 10 * x + static_cast<UnsignedInt>(ch - char_type{ '0' });
        ++count;
        ++pos; 
    }

    if (count == 0)
        return false;

    while (count < Width)
    {
        x = 10 * x;
        ++count;
    }
    
    fmt.value = x;
    return read_impl(pos, end, std::forward<Others>(others)...);
}

} // namespace date

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
