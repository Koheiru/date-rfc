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
#include <tuple>

// ----------------------------------------------------------------------------
namespace date
{

// ----------------------------------------------------------------------------
//! NOTE: code below is based on the following article: 
//  https://www.researchgate.net/publication/316558298_Date_Algorithms#pf2a

// ----------------------------------------------------------------------------
//                              calendar helper
// ----------------------------------------------------------------------------
struct calendar_helper
{
    using year_type     = uint16_t;
    using month_type    = uint8_t;
    using day_type      = uint8_t;
    using hour_type     = uint8_t;
    using minute_type   = uint8_t;
    using second_type   = uint8_t;
    using seconds_count = uint64_t;
    
    enum week_day : uint8_t
    {
        Monday    = 1,
        Tuesday   = 2,
        Wednesday = 3,
        Thursday  = 4,
        Friday    = 5,
        Saturday  = 6,
        Sunday    = 7,
    };

    struct date
    {
        date(year_type year, month_type month, day_type day)
            : year(year), month(month), day(day) {}

        year_type  year  = 1970; //!< [1 .. 2^16-1]
        month_type month = 1;    //!< [1 .. 12]
        day_type   day   = 1;    //!< [1 .. 31]
    };

    struct time
    {
        time(hour_type hour, minute_type minute, second_type second)
            : hour(hour), minute(minute), second(second) {}

        hour_type   hour   = 0;    //!< [0 .. 23]
        minute_type minute = 0;    //!< [0 .. 59]
        second_type second = 0;    //!< [0 .. 59]
    };

    struct date_time : date, time
    {
        date_time(year_type year, month_type month, day_type day, hour_type hour, minute_type minute, second_type second)
            : date(year, month, day), time(hour, minute, second) {}
    };

    static week_day day_of_week(const date& date)
    {
        const auto m = date.month + (date.month < 3 ? 12 : 0);
        const auto y = date.year - (date.month < 3 ? 1 : 0);
        return static_cast<week_day>((date.day + (153 * m - 457) / 5 + 365 * y + y / 4 - y / 100 + y / 400 + 1) % 7 + 1);
    }

    static bool is_leap_year(year_type year)
    {
        return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
    }

    static day_type days_in_month(year_type year, month_type month)
    {
        if (month == 2)
            return is_leap_year(year) ? 29 : 28;
        if (month == 2 ||month == 4 ||month == 6 ||month == 9 ||month == 11)
            return 30;
        return 31;
    }

    static seconds_count to_seconds_count(const date_time& dt)
    {
        const seconds_count k = (dt.month < 3 ? 1 : 0);
        const seconds_count m = dt.month + 12 * k;
        const seconds_count y = dt.year - k;
        const seconds_count d = dt.day + (153 * m - 457) / 5 + 365 * y + (y / 4) - (y / 100) + (y / 400) - 306;
        return static_cast<seconds_count>(d * 24 * 60 * 60 + dt.hour * 60 * 60 + dt.minute * 60 + dt.second);
    }

    static date_time from_seconds_count(seconds_count seconds)
    {
        const seconds_count days_count = seconds / (24 * 60 * 60);
        const seconds_count z = days_count + 306;
        const seconds_count h = 100 * z - 25;
        const seconds_count a = h / 3652425;
        const seconds_count b = a - a / 4;
        const seconds_count y = static_cast<year_type>((100 * b + h) / 36525);
        const seconds_count c = b + z - 365 * y - y / 4;
        const seconds_count m = (5 * c + 456) / 153;
        const auto day   = static_cast<day_type>(c - (153 * m - 457) / 5);
        const auto month = static_cast<month_type>(m - (m > 12 ? 12 : 0));
        const auto year  = static_cast<year_type>(y + (m > 12 ? 1 : 0));
        auto seconds_rest = seconds - days_count * (24 * 60 * 60);
        const auto hour = static_cast<hour_type>(seconds_rest / (60 * 60));
        seconds_rest -= hour * (60 * 60);
        const auto minute = static_cast<minute_type>(seconds_rest / 60);
        const auto second = static_cast<second_type>(seconds_rest - minute * 60);
        return date_time{ year, month, day, hour, minute, second };
    }
};

} // namespace date
