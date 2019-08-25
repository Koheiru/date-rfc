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
//                              calendar helper
// ----------------------------------------------------------------------------
struct calendar_helper
{
    using week_day = uint8_t;
    using month_days = uint8_t;
    using seconds_count = std::time_t;

    /*
    template <class Year>
    static bool is_leap_year(Year year)
    {
        return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
    }

    template <class Year, class Month>
    static month_days days_in_month(Year year, Month month)
    {
        return (month == 2) ? (is_leap_year(year) ? 29 : 28) :
            ((month == 4 || month == 6 || month == 9 || month == 11) ? 30 : 31);
    }
    */

    template <class Year, class Month, class Day>
    static week_day day_of_week(Year year, Month month, Day day)
    {
        const auto a = (14 - month) / 12;
        const auto y = year - a;
        const auto m = month + 12 * a - 2;
        return static_cast<week_day>((day + y + (y / 4) - (y / 100) + (y / 400) + (31 * m) / 12) % 7);
    }

    template <class Year, class Month, class Day, class Hour, class Minute, class Second>
    static seconds_count to_seconds_count(Year year, Month month, Day day, Hour hour, Minute minute, Second second)
    {
        const auto a = (14 - month) / 12;
        const auto y = year + 4800 - a;
        const auto m = month + 12 * a - 3;
        const auto d = day + ((153 * m + 2) / 5) + 365 * y + (y / 4) - (y / 100) + (y / 400) - 32045;
        return static_cast<seconds_count>(d * 24 * 60 * 60 + hour * 60 * 60 + minute * 60 + second);
    }

    template <class Year, class Month, class Day, class Hour, class Minute, class Second>
    static std::tuple<Year, Month, Day, Hour, Minute, Second> from_seconds_count(seconds_count seconds)
    {
        const auto days_count = seconds / (24 * 60 * 60);
        const auto a = days_count + 32044;
        const auto b = (4 * a + 3) / 146097;
        const auto c = a - ((146097 * b) / 4);
        const auto d = (4 * c + 3) / 1461;
        const auto e = c - (1461 * d) / 4;
        const auto m = (5 * e + 2) / 153;
        const auto year  = static_cast<Year>(100 * b + d - 4800 + (m / 10));
        const auto month = static_cast<Month>(m + 3 - 12 * (m / 10));
        const auto day   = static_cast<Day>(e - ((153 * m + 2) / 5) + 1);

        auto seconds_rest = seconds - days_count * (24 * 60 * 60);
        const auto hour = static_cast<Hour>(seconds_rest / (60 * 60));
        seconds_rest -= hour * (60 * 60);
        const auto minute = static_cast<Minute>(seconds_rest / 60);
        const auto second = static_cast<Second>(seconds_rest - minute * 60);

        return std::make_tuple(year, month, day, hour, minute, second);
    }
};

} // namespace date
