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
#include <iterator>
#include "details/static_string.h"
#include "details/iterator_traits.h"
#include "details/calendar_helper.h"
#include "details/fmt_common.h"
#include "date_converter.h"
#include "date_formatter.h"

// ----------------------------------------------------------------------------
namespace date
{

// ----------------------------------------------------------------------------
//                                  rfc1123
// ----------------------------------------------------------------------------
struct rfc1123
{
    struct parts
    {
        uint16_t year     = 1970;       //!< [0 .. 2^16-1]
        uint8_t  month    = 1;          //!< [1 .. 12]
        uint8_t  day      = 1;          //!< [1 .. 31]
        uint8_t  week_day = 1;          //!< [1 .. 7]
        uint8_t  hour     = 0;          //!< [0 .. 23]
        uint8_t  minute   = 0;          //!< [0 .. 59]
        uint8_t  second   = 0;          //!< [0 .. 59]
        int16_t  offset_in_minutes = 0; //!< [-6039 .. 6039] (due to format '(+|-)HHMM')
    };

    template <class Char>
    static constexpr std::array<std::pair<static_string<Char, 4>, uint8_t>, 7> weekday_names()
    {
        using s = static_string<Char, 4>;
        return {
            std::make_pair(s("Sun"), uint8_t(1)),
            std::make_pair(s("Mon"), uint8_t(2)),
            std::make_pair(s("Tue"), uint8_t(3)),
            std::make_pair(s("Wed"), uint8_t(4)),
            std::make_pair(s("Thu"), uint8_t(5)),
            std::make_pair(s("Fri"), uint8_t(6)),
            std::make_pair(s("Sat"), uint8_t(7)),
        };
    }

    template <class Char>
    static constexpr std::array<std::pair<static_string<Char, 4>, uint8_t>, 12> month_names()
    {
        using s = static_string<Char, 4>;
        return {
            std::make_pair(s("Jan"), uint8_t(1)),
            std::make_pair(s("Feb"), uint8_t(2)),
            std::make_pair(s("Mar"), uint8_t(3)),
            std::make_pair(s("Apr"), uint8_t(4)),
            std::make_pair(s("May"), uint8_t(5)),
            std::make_pair(s("Jun"), uint8_t(6)),
            std::make_pair(s("Jul"), uint8_t(7)),
            std::make_pair(s("Aug"), uint8_t(8)),
            std::make_pair(s("Sep"), uint8_t(9)),
            std::make_pair(s("Oct"), uint8_t(10)),
            std::make_pair(s("Nov"), uint8_t(11)),
            std::make_pair(s("Dec"), uint8_t(12)),
        };
    }

    template <class Char>
    static constexpr std::array<std::pair<static_string<Char, 4>, int16_t>, 11> zone_names()
    {
        //! Ignore military zones except Z-zone.
        using s = static_string<Char, 4>;
        return {
            std::make_pair(s("Z"),   int16_t(0)),
            std::make_pair(s("GMT"), int16_t(0)),
            std::make_pair(s("UT"),  int16_t(0)),
            std::make_pair(s("EST"), int16_t(-5 * 60)),
            std::make_pair(s("EDT"), int16_t(-4 * 60)),
            std::make_pair(s("CST"), int16_t(-6 * 60)),
            std::make_pair(s("CDT"), int16_t(-5 * 60)),
            std::make_pair(s("MST"), int16_t(-7 * 60)),
            std::make_pair(s("MDT"), int16_t(-6 * 60)),
            std::make_pair(s("PST"), int16_t(-8 * 60)),
            std::make_pair(s("PDT"), int16_t(-7 * 60)),
        };
    }

    template <class Iterator>
    static bool read(Iterator& pos, const Iterator& end, parts& dt)
    {
        using char_type = typename iterator_traits<Iterator>::value_type;
        constexpr auto weekday_aliases = weekday_names<char_type>();
        constexpr auto month_aliases = month_names<char_type>();
        constexpr auto zone_aliases = zone_names<char_type>();

        pos = skip_spaces(pos, end);
        if (pos == end)
            return false;

        dt = parts{};
        int16_t offset_hours = 0;
        int16_t offset_minutes = 0;
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

        if (!::date::read(pos, end, fmt))
            return false;

        if (dt.year < 100)
            dt.year += 1900;
        if (dt.week_day == 0)
            dt.week_day = calendar_helper::day_of_week(dt.year, dt.month, dt.day);
        if (dt.offset_in_minutes == 0)
            dt.offset_in_minutes = offset_hours * 60 + (offset_hours > 0 ? offset_minutes : -offset_minutes);

        return true;
    }

    template <class Iterator>
    static bool write(const parts& dt, Iterator& dst)
    {
        return true;
    }
};

// ----------------------------------------------------------------------------
template <>
struct date_converter<rfc1123, std::time_t, void>
{
    static bool from_parts(const rfc1123::parts& parts, std::time_t& timepoint)
    {
        const auto epoch_offset = calendar_helper::to_seconds_count(1970, 1, 1, 0, 0, 0);
        const auto seconds_count = calendar_helper::to_seconds_count(parts.year, parts.month, parts.day, parts.hour, parts.minute, parts.second);
        timepoint = static_cast<std::time_t>(seconds_count - parts.offset_in_minutes * 60 - epoch_offset);
        return true;
    }

    static bool to_parts(std::time_t timepoint, rfc1123::parts& parts)
    {
        const auto epoch_offset = calendar_helper::to_seconds_count(1970, 1, 1, 0, 0, 0);
        const auto dt = calendar_helper::from_seconds_count<decltype(parts.year), decltype(parts.month), decltype(parts.day), 
            decltype(parts.hour), decltype(parts.minute), decltype(parts.second)>(epoch_offset + timepoint);
        parts.year     = std::get<0>(dt);
        parts.month    = std::get<1>(dt);
        parts.day      = std::get<2>(dt);
        parts.week_day = calendar_helper::day_of_week(parts.year, parts.month, parts.day);
        parts.hour     = std::get<3>(dt);
        parts.minute   = std::get<4>(dt);
        parts.second   = std::get<5>(dt);
        parts.offset_in_minutes = 0;
        return true;
    }
};

// ----------------------------------------------------------------------------
template <class Date, class Converter = date_converter<rfc1123, Date>>
format_rfc<rfc1123, Date> format_rfc1123(Date& value, const rfc1123& format = rfc1123())
{
    return format_rfc<rfc1123, Date, Converter>(value, format);
}

} // namespace date
