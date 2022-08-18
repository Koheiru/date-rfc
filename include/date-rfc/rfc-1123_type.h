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
//                                  rfc1123
// ----------------------------------------------------------------------------
struct rfc1123
{
    using year_type     = uint16_t;
    using month_type    = uint8_t;
    using day_type      = uint8_t;
    using week_day_type = uint8_t;
    using hour_type     = uint8_t;
    using minute_type   = uint8_t;
    using second_type   = uint8_t;
    using offset_type   = int16_t;

    struct parts
    {
        year_type     year     = 1970;       //!< [1 .. 2^16-1]
        month_type    month    = 1;          //!< [1 .. 12]
        day_type      day      = 1;          //!< [1 .. 31]
        week_day_type week_day = 2;          //!< [0 .. 7] where 0 is 'value not defined'.
        hour_type     hour     = 0;          //!< [0 .. 23]
        minute_type   minute   = 0;          //!< [0 .. 59]
        second_type   second   = 0;          //!< [0 .. 59]
        offset_type   offset_in_minutes = 0; //!< [-6039 .. 6039] (due to format '(+|-)HHMM')
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
        if (std::abs(dt.offset_in_minutes) > 6039)
            return false;
        if (dt.week_day != 0)
        {
            const auto week_day = calendar_helper::day_of_week(calendar_helper::date{ dt.year, dt.month, dt.day });
            if (dt.week_day != week_day)
                return false;
        }
        return true;
    }

    template <class Char>
    static constexpr std::array<std::pair<static_string<Char, 4>, uint8_t>, 7> weekday_names()
    {
        using string = static_string<Char, 4>;
        return {
            std::make_pair(string("Mon"), uint8_t(1)),
            std::make_pair(string("Tue"), uint8_t(2)),
            std::make_pair(string("Wed"), uint8_t(3)),
            std::make_pair(string("Thu"), uint8_t(4)),
            std::make_pair(string("Fri"), uint8_t(5)),
            std::make_pair(string("Sat"), uint8_t(6)),
            std::make_pair(string("Sun"), uint8_t(7)),
        };
    }

    template <class Char>
    static constexpr std::array<std::pair<static_string<Char, 4>, uint8_t>, 12> month_names()
    {
        using string = static_string<Char, 4>;
        return {
            std::make_pair(string("Jan"), uint8_t(1)),
            std::make_pair(string("Feb"), uint8_t(2)),
            std::make_pair(string("Mar"), uint8_t(3)),
            std::make_pair(string("Apr"), uint8_t(4)),
            std::make_pair(string("May"), uint8_t(5)),
            std::make_pair(string("Jun"), uint8_t(6)),
            std::make_pair(string("Jul"), uint8_t(7)),
            std::make_pair(string("Aug"), uint8_t(8)),
            std::make_pair(string("Sep"), uint8_t(9)),
            std::make_pair(string("Oct"), uint8_t(10)),
            std::make_pair(string("Nov"), uint8_t(11)),
            std::make_pair(string("Dec"), uint8_t(12)),
        };
    }

    template <class Char>
    static constexpr std::array<std::pair<static_string<Char, 4>, int16_t>, 11> zone_names()
    {
        //! Ignore military zones except Z-zone.
        using string = static_string<Char, 4>;
        return {
            std::make_pair(string("Z"),   int16_t(0)),
            std::make_pair(string("GMT"), int16_t(0)),
            std::make_pair(string("UT"),  int16_t(0)),
            std::make_pair(string("EST"), int16_t(-5 * 60)),
            std::make_pair(string("EDT"), int16_t(-4 * 60)),
            std::make_pair(string("CST"), int16_t(-6 * 60)),
            std::make_pair(string("CDT"), int16_t(-5 * 60)),
            std::make_pair(string("MST"), int16_t(-7 * 60)),
            std::make_pair(string("MDT"), int16_t(-6 * 60)),
            std::make_pair(string("PST"), int16_t(-8 * 60)),
            std::make_pair(string("PDT"), int16_t(-7 * 60)),
        };
    }

    template <class Iterator>
    static bool read(Iterator& pos, const Iterator& end, parts& value)
    {
        using char_type = typename iterator_traits<Iterator>::value_type;
        constexpr auto weekday_aliases = weekday_names<char_type>();
        constexpr auto month_aliases = month_names<char_type>();
        constexpr auto zone_aliases = zone_names<char_type>();

        pos = skip_spaces(pos, end);
        if (pos == end)
            return false;

        parts dt{};
        std::memset(static_cast<void*>(&dt), 0, sizeof(parts));

        offset_type offset_hours = 0;
        offset_type offset_minutes = 0;
        auto fmt = format(
            optional(
                aliases(dt.week_day, weekday_aliases), 
                characters<char_type>(", ")),
            unsigned_integer<1, 2>(dt.day),    character<char_type>(' '), 
            aliases(dt.month, month_aliases),  character<char_type>(' '), 
            unsigned_integer<2, 4>(dt.year),   character<char_type>(' '),
            unsigned_integer<2, 2>(dt.hour),   character<char_type>(':'), 
            unsigned_integer<2, 2>(dt.minute), 
            optional(
                character<char_type>(':'),
                unsigned_integer<2, 2>(dt.second)), 
            character<char_type>(' '), 
            cases(
                branch(
                    aliases(dt.offset_in_minutes, zone_aliases)),
                branch(
                    signed_integer<2, 2, SignRequired>(offset_hours),
                    unsigned_integer<2, 2>(offset_minutes))));

        if (!::date::read(pos, end, fmt) || !validate(dt))
            return false;

        if (dt.year < 100)
            dt.year += 1900;

        if (dt.offset_in_minutes == 0)
            dt.offset_in_minutes = offset_hours * 60 + (offset_hours > 0 ? offset_minutes : -offset_minutes);

        if (dt.week_day == 0)
            dt.week_day = calendar_helper::day_of_week(calendar_helper::date{ dt.year, dt.month, dt.day });

        value = dt;
        return true;
    }

    template <class Iterator>
    static bool write(const parts& dt, Iterator& dst)
    {
        if (!validate(dt))
            return false;

        using char_type = typename iterator_traits<Iterator>::value_type;
        constexpr auto weekday_aliases = weekday_names<char_type>();
        constexpr auto month_aliases = month_names<char_type>();
        constexpr auto zone_aliases = zone_names<char_type>();

        characters_writer::write(dst, weekday_aliases[dt.week_day - 1].first);
        characters_writer::write(dst, char_type{ ',' });
        characters_writer::write(dst, char_type{ ' ' });
        number_writer<2>::write(dst, dt.day);
        characters_writer::write(dst, char_type{ ' ' });
        characters_writer::write(dst, month_aliases[dt.month - 1].first);
        characters_writer::write(dst, char_type{ ' ' });
        number_writer<4>::write(dst, dt.year);
        characters_writer::write(dst, char_type{ ' ' });
        number_writer<2>::write(dst, dt.hour);
        characters_writer::write(dst, char_type{ ':' });
        number_writer<2>::write(dst, dt.minute);
        characters_writer::write(dst, char_type{ ':' });
        number_writer<2>::write(dst, dt.second);
        characters_writer::write(dst, char_type{ ' ' });
        characters_writer::write(dst, zone_aliases[1].first);

        return true;
    }
};

} // namespace date
