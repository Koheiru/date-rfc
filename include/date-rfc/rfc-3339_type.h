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

#include <cctype>
#include <ctime>
#include <cmath>
#include <cstring>
#include <iterator>
#include "details/static_string.h"
#include "details/iterator_traits.h"
#include "details/calendar_helper.h"
#include "details/data_writers.h"
#include "details/fmt_common.h"

// ----------------------------------------------------------------------------
namespace date
{

// ----------------------------------------------------------------------------
//                                  rfc3339
// ----------------------------------------------------------------------------
struct rfc3339
{
    using year_type    = uint16_t;
    using month_type   = uint8_t;
    using day_type     = uint8_t;
    using hour_type    = uint8_t;
    using minute_type  = uint8_t;
    using second_type  = uint8_t;
    using nanosec_type = uint32_t;
    using offset_type  = int16_t;

    struct parts
    {
        year_type    year     = 1970;       //!< [1 .. 2^16-1]
        month_type   month    = 1;          //!< [1 .. 12]
        day_type     day      = 1;          //!< [1 .. 31]
        hour_type    hour     = 0;          //!< [0 .. 23]
        minute_type  minute   = 0;          //!< [0 .. 59]
        second_type  second   = 0;          //!< [0 .. 59]
        nanosec_type nanosecond = 0;        //!< [0 .. 999999999]
        offset_type  offset_in_minutes = 0; //!< [-1439 .. 1439]
    };

    static bool validate(const parts& dt)
    {
        if (dt.year == 0)
            return false;
        if (dt.month == 0 || dt.month > 12)
            return false;
        if (dt.day == 0 || dt.day > calendar_helper::days_in_month(dt.year, dt.month))
            return false;
        if (dt.hour > 23)
            return false;
        if (dt.minute > 59)
            return false;
        if (dt.second > 59)
            return false;
        if (dt.nanosecond > 999999999)
            return false;
        if (std::abs(dt.offset_in_minutes) > 1439)
            return false;
        return true;
    }

    template <class Iterator>
    static bool read(Iterator& pos, const Iterator& end, parts& value)
    {
        using char_type = typename iterator_traits<Iterator>::value_type;

        pos = skip_spaces(pos, end);
        if (pos == end)
            return false;

        parts dt{};
        std::memset(static_cast<void*>(&dt), 0, sizeof(parts));

        offset_type offset_hours = 0;
        offset_type offset_minutes = 0;
        auto fmt = format(
            unsigned_integer<4, 4>(dt.year),   character<char_type>('-'),
            unsigned_integer<2, 2>(dt.month),  character<char_type>('-'),
            unsigned_integer<2, 2>(dt.day),    character<char_type>('T'),
            unsigned_integer<2, 2>(dt.hour),   character<char_type>(':'),
            unsigned_integer<2, 2>(dt.minute), character<char_type>(':'),
            unsigned_integer<2, 2>(dt.second),
            optional(
                character<char_type>('.'),
                fraction<9>(dt.nanosecond)),
            cases(
                branch(
                    character<char_type>('Z')),
                branch(
                    signed_integer<2, 2, SignRequired>(offset_hours),
                    character<char_type>(':'),
                    unsigned_integer<2, 2>(offset_minutes))));

        if (!::date::read(pos, end, fmt) || !validate(dt))
            return false;

        value = dt;
        value.offset_in_minutes = offset_hours * 60 + (offset_hours > 0 ? offset_minutes : -offset_minutes);
        return true;
    }

    template <class Iterator>
    static bool write(const parts& dt, Iterator& dst)
    {
        if (!validate(dt))
            return false;

        using char_type = typename iterator_traits<Iterator>::value_type;

        number_writer<4>::write(dst, dt.year);
        characters_writer::write(dst, char_type{ '-' });
        number_writer<2>::write(dst, dt.month);
        characters_writer::write(dst, char_type{ '-' });
        number_writer<2>::write(dst, dt.day);
        characters_writer::write(dst, char_type{ 'T' });
        number_writer<2>::write(dst, dt.hour);
        characters_writer::write(dst, char_type{ ':' });
        number_writer<2>::write(dst, dt.minute);
        characters_writer::write(dst, char_type{ ':' });
        number_writer<2>::write(dst, dt.second);
        characters_writer::write(dst, char_type{ 'Z' });

        return true;
    }
};

} // namespace date
