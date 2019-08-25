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
#include <string>
#include "index_sequence.h"

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpedantic"
# if __GNUC__ < 5
//! GCC 4.9 Bug 61489 Wrong warning with -Wmissing-field-initializers.
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
# endif
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4351)
#endif // defined(_MSC_VER)

// ----------------------------------------------------------------------------
namespace date
{

// ---------------------------------------------------------------------------
//                                  helpers
// ---------------------------------------------------------------------------
template <class OutputChar, class InputChar, std::size_t InputLength>
constexpr OutputChar convert_character(const InputChar (&input)[InputLength], std::size_t index)
{
    return (index >= InputLength) ? OutputChar{ '\0' } : static_cast<OutputChar>(input[index]);
}

template <class OutputChar, std::size_t OutputLength, class InputChar, std::size_t InputLength, std::size_t... Indexes>
constexpr std::array<OutputChar, OutputLength> make_literal_array_helper(const InputChar (&input)[InputLength], std_impl::index_sequence<Indexes...>)
{
    return{ { convert_character<OutputChar>(input, Indexes)... } };
}

template <class OutputChar, std::size_t OutputLength, class InputChar, std::size_t InputLength>
constexpr std::array<OutputChar, OutputLength> make_literal_array(const InputChar (&input)[InputLength])
{
    static_assert(OutputLength >= InputLength, "Source string is too long");
    return make_literal_array_helper<OutputChar, OutputLength>(input, std_impl::make_index_sequence<OutputLength>{});
}

// ----------------------------------------------------------------------------
//                               static_string
// ----------------------------------------------------------------------------
template <class Char, std::size_t Capacity>
class static_string
{
public:
    using value_type = Char;

public:
    template <std::size_t InputLength>
    constexpr static_string(const Char (&input)[InputLength])
        : m_data(make_literal_array<Char, Capacity>(input))
        , m_length(InputLength - 1)
    {
        static_assert(InputLength > 0, "Passed string has zero length");
        static_assert(Capacity >= InputLength, "Passed string is too long");
    }

    template <std::size_t InputLength, class U = Char, class = typename std::enable_if<2 <= sizeof(U)>::type>
    constexpr static_string(const char (&input)[InputLength])
        : m_data(make_literal_array<Char, Capacity>(input))
        , m_length(InputLength - 1)
    {
        static_assert(InputLength > 0, "Passed string has zero length");
        static_assert(Capacity >= InputLength, "Passed string is too long");
    }

    constexpr const value_type* c_str() const
    {
        return m_data.data();
    }

    constexpr const value_type& operator[](std::size_t index) const
    {
        return m_data[index];
    }

    constexpr std::size_t length() const
    {
        return m_length;
    }

    constexpr bool empty() const
    {
        return (m_length == 0);
    }

    bool contains(value_type value) const
    {
        for (std::size_t i = 0; i < m_length; ++i)
        {
            if (m_data[i] == value)
                return true;
        }
        return false;
    }

private:
    const std::array<Char, Capacity> m_data;
    const std::size_t m_length;
};

// ----------------------------------------------------------------------------
template <class Char, class InputChar, std::size_t InputLength>
constexpr static_string<Char, InputLength> make_static_string(InputChar (&input)[InputLength])
{
    return static_string<Char, InputLength>(input);
}

// ----------------------------------------------------------------------------
template <class Char, std::size_t Capacity>
bool operator==(const static_string<Char, Capacity>& lhs, const Char* rhs)
{
    if (std::char_traits<Char>::length(rhs) != lhs.length())
        return false;
    return (std::char_traits<Char>::compare(rhs, lhs.c_str(), lhs.length()) == 0);
}

// ----------------------------------------------------------------------------
template <class Char, std::size_t Capacity, std::size_t N>
bool operator==(const static_string<Char, Capacity>& lhs, const Char (&rhs)[N])
{
    if (N < lhs.length())
        return false;
    return (std::char_traits<Char>::compare(static_cast<const Char*>(rhs), lhs.c_str(), lhs.length()) == 0);
}

// ----------------------------------------------------------------------------
template <class Char, std::size_t Capacity, class Traits, class Allocator>
bool operator==(const static_string<Char, Capacity>& lhs, const std::basic_string<Char, Traits, Allocator>& rhs)
{
    if (rhs.length() != lhs.length())
        return false;
    return (rhs.compare(lhs.c_str()) == 0);
}

} // namespace date

#if defined(_MSC_VER)
#pragma warning(pop)
#endif // defined(_MSC_VER)

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
