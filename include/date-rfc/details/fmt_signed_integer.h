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
//                            types: signed integer
// ----------------------------------------------------------------------------
template <class SignedInt, unsigned MinLength, unsigned MaxLength, bool SignMandatory>
struct signed_integer_t
{
    using value_type = SignedInt;

    enum : unsigned { min_length = MinLength + sign_traits<SignMandatory>::min_length };
    enum : unsigned { max_length = MaxLength + sign_traits<SignMandatory>::max_length };
    enum : bool { need_cache = false };
    enum : bool { sign_mandatory = SignMandatory };

    signed_integer_t(value_type& value) : value(value) {}
    ~signed_integer_t() = default;

    value_type& value;
};

// ----------------------------------------------------------------------------
template <unsigned MinLength, unsigned MaxLength, bool SignMandatory, class SignedInt>
signed_integer_t<SignedInt, MinLength, MaxLength, SignMandatory> signed_integer(SignedInt& value)
{
    return signed_integer_t<SignedInt, MinLength, MaxLength, SignMandatory>{ value };
}

// ----------------------------------------------------------------------------
template <class Iterator, class SignedInt, unsigned MinLength, unsigned MaxLength, bool SignMandatory, class ...Others>
bool read_impl(Iterator& pos, const Iterator& end, signed_integer_t<SignedInt, MinLength, MaxLength, SignMandatory>& fmt, Others&&... others)
{
    using char_type  = typename iterator_traits<Iterator>::value_type;

    if (pos == end)
    {
        if (MinLength > 0)
            return false;

        fmt.value = SignedInt{ 0 };
        return read_impl(pos, end, std::forward<Others>(others)...);
    }

    const char_type ch = *pos;
    const bool sign_specified = (ch == char_type{ '-' } || ch == char_type{ '+' });
    if (SignMandatory && !sign_specified)
        return false;

    if (sign_specified)
        ++pos;

    auto x = SignedInt{ 0 };
    auto unsignedFmt = unsigned_integer<MinLength, MaxLength>(x);
    if (!read_impl(pos, end, unsignedFmt))
        return false;

    const bool is_negative = (ch == char_type{ '-' });
    fmt.value = is_negative ? -x : x;

    return read_impl(pos, end, std::forward<Others>(others)...);
}

} // namespace date

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
