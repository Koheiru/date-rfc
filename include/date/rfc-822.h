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
struct rfc822;

template <class Date>
format_rfc<rfc822, Date> format_rfc822(Date& value, const rfc822& format = rfc822())
{
    return format_rfc<rfc822, Date>(value, format);
}

// ----------------------------------------------------------------------------
//                                  rfc822
// ----------------------------------------------------------------------------
struct rfc822
{
    template <class CharT, class Traits, class Alloc = std::allocator<CharT>>
    static dt_parts read(std::basic_istream<CharT, Traits>& stream, const rfc822& fmt, std::basic_string<CharT, Traits, Alloc>* zone, int* offset)
    {
        using namespace details;
        using details::read;
        
        typedef std::remove_pointer<decltype(offset)>::type dt_offset;
        typedef typename std::remove_pointer<decltype(zone)>::type   dt_zone;

        dt_parts  parts{};

        size_t pos = 0;
        skip_spaces(stream, pos);
        if (stream.eof())
            throw std::logic_error(std::string("unexpected eof at ") + std::to_string(pos));
        
        int offset_hour{};
        int offset_minute{};
        size_t zone_index(-1);
        
        const auto weekday_names = weekday_names_short();
        const auto month_names = month_names_short();
        const auto zone_names = zone_names_rfc822();

        read(stream, pos, 
            optional(ra<3,3>(parts.weekday, weekday_names), rc(','), rc(' ')),
            ru<1,2>(parts.day), rc(' '), ra<3,3>(parts.month, month_names), rc(' '), ru<2,4>(parts.year), rc(' '), 
            ru<2,2>(parts.hour), rc(':'), ru<2,2>(parts.minute), optional(rc(':'), ru<2,2>(parts.second)), rc(' '),
            cases(
                branch(rs<2,2>(offset_hour), ru<2,2>(offset_minute)),
                branch(ra<2,3>(zone_index, zone_names))));

        if (offset)
            *offset = (offset_hour * 60 + offset_minute) * 60;
        if (zone && zone_index != -1)
            *zone = std::move(dt_zone(*(zone_names.first + zone_index)));

        return parts;
    }
};

} // namespace date
