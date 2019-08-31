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
    using week_day = uint8_t;
    using seconds_count = uint64_t;

    template <class Year, class Month, class Day>
    static week_day day_of_week(Year year, Month month, Day day)
    {
        const auto m = month + (month < 3 ? 12 : 0);
        const auto y = year - (month < 3 ? 1 : 0);
        return static_cast<week_day>((day + (153 * m - 457) / 5 + 365 * y + y / 4 - y / 100 + y / 400 + 1) % 7 + 1);
    }

    template <class Year, class Month, class Day, class Hour, class Minute, class Second>
    static seconds_count to_seconds_count(Year year, Month month, Day day, Hour hour, Minute minute, Second second)
    {
        const seconds_count k = (month < 3 ? 1 : 0);
        const seconds_count m = month + 12 * k;
        const seconds_count y = year - k;
        const seconds_count d = day + (153 * m - 457) / 5 + 365 * y + (y / 4) - (y / 100) + (y / 400) - 306;
        return static_cast<seconds_count>(d * 24 * 60 * 60 + hour * 60 * 60 + minute * 60 + second);
    }

    template <class Year, class Month, class Day, class Hour, class Minute, class Second>
    static std::tuple<Year, Month, Day, Hour, Minute, Second> from_seconds_count(seconds_count seconds)
    {
        const seconds_count days_count = seconds / (24 * 60 * 60);
        const seconds_count z = days_count + 306;
        const seconds_count h = 100 * z - 25;
        const seconds_count a = h / 3652425;
        const seconds_count b = a - a / 4;
        const seconds_count y = static_cast<Year>((100 * b + h) / 36525);
        const seconds_count c = b + z - 365 * y - y / 4;
        const seconds_count m = (5 * c + 456) / 153;
        const auto day   = static_cast<Day>(c - (153 * m - 457) / 5);
        const auto month = static_cast<Month>(m - (m > 12 ? 12 : 0));
        const auto year  = static_cast<Year>(y + (m > 12 ? 1 : 0));

        auto seconds_rest = seconds - days_count * (24 * 60 * 60);
        const auto hour = static_cast<Hour>(seconds_rest / (60 * 60));
        seconds_rest -= hour * (60 * 60);
        const auto minute = static_cast<Minute>(seconds_rest / 60);
        const auto second = static_cast<Second>(seconds_rest - minute * 60);

        return std::make_tuple(year, month, day, hour, minute, second);
    }
};

} // namespace date
