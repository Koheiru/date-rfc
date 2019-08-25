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
#include <array>
#include <utility>
#include "fmt_traits.h"
#include "iterator_traits.h"
#include "static_string.h"

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
//                               types: aliases
// ----------------------------------------------------------------------------
template <class Char, std::size_t Length, class Value, std::size_t Count>
struct aliases_t
{
    using value_type = Value;
    using aliases_type = std::array<std::pair<static_string<Char, Length>, Value>, Count>;

    enum : unsigned { min_length = Length };
    enum : unsigned { max_length = Length };
    enum : bool { need_cache = false };

    aliases_t(value_type& value, const aliases_type& aliases) : value(value), aliases(aliases) {}
    ~aliases_t() = default;

    value_type& value;
    const aliases_type& aliases;
};

// ----------------------------------------------------------------------------
template <class Char, std::size_t Length, class Value, std::size_t Count>
static aliases_t<Char, Length, Value, Count> aliases(Value& value, const std::array<std::pair<static_string<Char, Length>, Value>, Count>& aliases)
{
    return aliases_t<Char, Length, Value, Count>{ value, aliases };
}

// ----------------------------------------------------------------------------
template <class Iterator, class Char, std::size_t Length, class Value, std::size_t Count, class ...Others>
bool read_impl(Iterator& pos, const Iterator& end, aliases_t<Char, Length, Value, Count>& fmt, Others&&... others)
{
    using char_type = typename iterator_traits<Iterator>::value_type;

    std::array<const Char*, Count> str_list;
    std::array<bool, Count> is_match;
    for (std::size_t k = 0; k < Count; ++k)
    {
        const auto& str = fmt.aliases[k].first;
        str_list[k] = str.c_str();
        is_match[k] = !str.empty();
    }

    for (std::size_t i = 0; i < Length; ++i, ++pos)
    {
        if (pos == end)
            return false;

        const char_type ch = *pos;
        for (std::size_t k = 0; k < Count; ++k)
        {
            if (!is_match[k])
                continue;
            
            const Char* str = str_list[k];
            if (*str != ch)
            {
                is_match[k] = false;
                continue;
            }

            ++str;
            if (*str != Char{ '\0' })
            {
                str_list[k] = str;
                continue;
            }

            ++pos;
            fmt.value = fmt.aliases[k].second;
            return read_impl(pos, end, std::forward<Others>(others)...);
        }
    }

    return false;
}

} // namespace date

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
