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
#include <ostream>
#include <istream>
#include "date_converter.h"

// ----------------------------------------------------------------------------
namespace date
{

// ----------------------------------------------------------------------------
//                                  stream formatter
// ----------------------------------------------------------------------------
template <class Format, class Date, class Converter = date_converter<Format, Date>>
struct format_rfc
{
    using format_type = Format;
    using date_type = Date;

    format_rfc(date_type& value, const format_type& format = format_type())
        : m_format(format), m_value(value)
    {}

    template <class T, class U, class C, class Char, class Traits>
    friend std::basic_istream<Char, Traits>& operator>>(std::basic_istream<Char, Traits>& stream, format_rfc<T, U, C>& formatter);

    template <class T, class U, class C, class Char, class Traits>
    friend std::basic_istream<Char, Traits>& operator>>(std::basic_istream<Char, Traits>& stream, format_rfc<T, U, C>&& formatter);

    template <class T, class U, class C, class Char, class Traits>
    friend std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& stream, const format_rfc<T, U, C>& formatter);

private:
    const format_type m_format;
    date_type& m_value;
};

// ----------------------------------------------------------------------------
template <class Format, class Date, class Converter, class Char, class Traits>
inline std::basic_istream<Char, Traits>& operator>>(std::basic_istream<Char, Traits>& stream, format_rfc<Format, Date, Converter>& formatter)
{
    typename Format::parts parts{};
    std::istreambuf_iterator<Char> end{};
    std::istreambuf_iterator<Char> pos(stream.rdbuf());
    if (!Format::read(pos, end, parts) || !Converter::from_parts(parts, formatter.m_value))
        stream.setstate(std::ios::failbit);
    return stream;
}

// ----------------------------------------------------------------------------
template <class Format, class Date, class Converter, class Char, class Traits>
inline std::basic_istream<Char, Traits>& operator>>(std::basic_istream<Char, Traits>& stream, format_rfc<Format, Date, Converter>&& formatter)
{
    typename Format::parts parts{};
    std::istreambuf_iterator<Char> end{};
    std::istreambuf_iterator<Char> pos(stream.rdbuf());
    if (!Format::read(pos, end, parts) || !Converter::from_parts(parts, formatter.m_value))
        stream.setstate(std::ios::failbit);
    return stream;
}

// ----------------------------------------------------------------------------
template <class Format, class Date, class Converter, class Char, class Traits>
inline std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& stream, const format_rfc<Format, Date, Converter>& formatter)
{
    typename Format::parts parts{};
    std::ostreambuf_iterator<Char> dst(stream);
    if (!Converter::to_parts(formatter.m_value, parts) || !Format::write(parts, dst))
        stream.setstate(std::ios::failbit);
    return stream;
}

} // namespace date
