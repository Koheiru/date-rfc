//
// The MIT License (MIT)
//
// Copyright (c) 2018 Yury Prostov
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

#include "date-templates.h"

// ----------------------------------------------------------------------------
namespace date
{
    
// ----------------------------------------------------------------------------
struct rfc3339;

template <class Date>
format_rfc<rfc3339, Date> format_rfc3339(Date& value, const rfc3339& format = rfc3339())
{
    return format_rfc<rfc3339, Date>(value, format);
}

// ----------------------------------------------------------------------------
//                                 rfc3339
// ----------------------------------------------------------------------------
struct rfc3339
{
    template <class CharT, class Traits, class Alloc = std::allocator<CharT>>
    static dt_parts read(std::basic_istream<CharT, Traits>& stream, const rfc3339& fmt, std::basic_string<CharT, Traits, Alloc>* zone, int* offset)
    {
        using namespace details;
        using details::read;

        dt_parts  parts{};

        size_t pos = 0;
        skip_spaces(stream, pos);
        if (stream.eof())
            throw std::logic_error(std::string("unexpected eof at ") + std::to_string(pos));
        
        int offset_hour(0);
        int offset_minute(0);

        read(stream, pos, 
            ru<4,4>(parts.year), rc('-'), ru<2,2>(parts.month), rc('-'), ru<2,2>(parts.day), rc('T'), 
            ru<2,2>(parts.hour), rc(':'), ru<2,2>(parts.minute), rc(':'), ru<2,2>(parts.second), optional(rc('.'), ru<1,9>(parts.nanosecond)),
            cases(
                branch(rc('Z')), 
                branch(rs<2,2>(offset_hour), rc(':'), ru<2,2>(offset_minute))));

        if (offset)
            *offset = (offset_hour * 60 + offset_minute) * 60;

        return parts;
    }
};

} // namespace date
