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

#include <ctime>
#include "details/calendar_helper.h"
#include "date_converter.h"
#include "rfc-3339_type.h"

// ----------------------------------------------------------------------------
namespace date
{

// ----------------------------------------------------------------------------
template <>
struct date_converter<rfc3339, std::time_t, void>
{
    static bool from_parts(const rfc3339::parts& parts, std::time_t& timepoint)
    {
        using date_time = calendar_helper::date_time;
        const auto epoch_offset = calendar_helper::to_seconds_count(date_time{ 1970, 1, 1, 0, 0, 0 });
        const auto seconds_count = calendar_helper::to_seconds_count(date_time{ parts.year, parts.month, parts.day, parts.hour, parts.minute, parts.second });
        timepoint = static_cast<std::time_t>(seconds_count - parts.offset_in_minutes * 60 - epoch_offset);
        return true;
    }

    static bool to_parts(std::time_t timepoint, rfc3339::parts& parts)
    {
        using date_time = calendar_helper::date_time;
        const auto epoch_offset = calendar_helper::to_seconds_count(date_time{ 1970, 1, 1, 0, 0, 0 });
        const auto dt = calendar_helper::from_seconds_count(epoch_offset + timepoint);
        parts.year     = dt.year;
        parts.month    = dt.month;
        parts.day      = dt.day;
        parts.hour     = dt.hour;
        parts.minute   = dt.minute;
        parts.second   = dt.second;
        parts.nanosecond        = 0;
        parts.offset_in_minutes = 0;
        return true;
    }
};

} // namespace date
