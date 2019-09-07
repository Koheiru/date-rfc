#include <iostream>
#include <sstream>
#include <iomanip>
#include <date-rfc/rfc-1123.h>
#include <date-rfc/rfc-3339.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif // defined(_MSC_VER)

// ----------------------------------------------------------------------------
struct ext_time_t
{
    std::time_t time;
    std::uint32_t nanosecond;
};

namespace date
{

template <>
struct date_converter<rfc3339, ext_time_t, void>
{
    static bool from_parts(const rfc3339::parts& parts, ext_time_t& timepoint)
    {
        if (!date_converter<rfc3339, std::time_t>::from_parts(parts, timepoint.time))
            return false;
        timepoint.nanosecond = parts.nanosecond;
        return true;
    }

    static bool to_parts(ext_time_t timepoint, rfc3339::parts& parts)
    {
        if (!date_converter<rfc3339, std::time_t>::to_parts(timepoint.time, parts))
            return false;
        parts.nanosecond = timepoint.nanosecond;
        return true;
    }
};

} // namespace date

// ----------------------------------------------------------------------------
void check_rfc1123()
{
    const std::string values[] = {
        std::string("Fri, 31 Dec 2010 23:59:59 GMT"),
        std::string("Fri, 31 Dec 2010 23:59:59 EDT"),
        std::string("Fri, 31 Dec 2010 23:59:59 +0430"),
        std::string("Fri, 31 Dec 2010 23:59:59 -0235"),
        std::string("Fri, 31 Dec 2010 23:59 -0555"),
        std::string("31 Dec 2010 23:59:59 GMT"),
        std::string("31 Dec 2010 23:59:59 EDT"),
        std::string("31 Dec 2010 23:59:59 +0430"),
        std::string("31 Dec 2010 23:59:59 -0235"),
        std::string("31 Dec 2010 23:59 GMT"),
        std::string("31 Dec 2010 23:59 EDT"),
        std::string("31 Dec 2010 23:59 +0430"),
    };

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "#               RFC 1123                #" << std::endl;
    for (auto value : values) {
        std::time_t dt{};
        std::istringstream stream(value);
        stream >> date::format_rfc1123(dt);
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Value:  " << value << std::endl;
        std::cout << "Parsed: " << std::put_time(std::gmtime(&dt), "%c") 
            << " (failbit: " << (stream.fail() ? "true" : "false") << ")" << std::endl;
        std::cout << "Revert: " << date::format_rfc1123(dt) << std::endl;
    }
    std::cout << std::endl;
}

// ----------------------------------------------------------------------------
void check_rfc3339()
{
    const std::string values[] ={
        std::string("1985-04-12T23:20:50.052Z"),
        std::string("1996-12-19T16:39:57-08:00"),
        std::string("1990-12-31T23:59:59Z"),
        std::string("1990-12-31T15:59:30+08:00"),
        std::string("1971-01-01T12:00:27.87+00:20"),
    };

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "#               RFC 3339                #" << std::endl;
    for (auto value : values) {
        ext_time_t dt{};
        std::istringstream stream(value);
        stream >> date::format_rfc3339(dt);
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Value:  " << value << std::endl;
        std::cout << "Parsed: " << std::put_time(std::gmtime(&dt.time), "%c") << '.' << std::setw(9) << std::setfill('0') << dt.nanosecond
            << " (failbit: " << (stream.fail() ? "true" : "false") << ")" << std::endl;
        std::cout << "Revert: " << date::format_rfc3339(dt) << std::endl;
    }
    std::cout << std::endl;
}

// ----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    //check_rfc1123();
    check_rfc3339();
    return 0;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif // defined(_MSC_VER)
