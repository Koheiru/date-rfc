#include <string>
#include <iostream>
#include <sstream>
#include <cwchar>
#include <iomanip>
#include <date/date-rfc.h>
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif // defined(_MSC_VER)

// ----------------------------------------------------------------------------
const char* zone_name(const std::string* zone)
{
    return (!zone || zone->empty() ? "{empty}" : zone->c_str());
}

const wchar_t* zone_name(const std::wstring* zone)
{
    return (!zone || zone->empty() ? L"{empty}" : zone->c_str());
}

// ----------------------------------------------------------------------------
namespace date
{

template <>
struct dt_traits < std::time_t >
{
    template <class CharT, class Traits, class Alloc = std::allocator<CharT>>
    static std::time_t join(const dt_parts& p, std::basic_string<CharT, Traits, Alloc>* zone, int32_t* offset)
    {
        std::wcout
            << "year = " << static_cast<int>(p.year)
            << ", month = " << static_cast<int>(p.month)
            << ", day = " << static_cast<int>(p.day)
            << ", weekday = " << static_cast<int>(p.weekday)
            << ", hour = " << static_cast<int>(p.hour)
            << ", minute = " << static_cast<int>(p.minute)
            << ", second = " << static_cast<int>(p.second)
            << ", nsecond = " << static_cast<int>(p.nanosecond)
            << ", zone = " << zone_name(zone)
            << ", offset = " << (!offset ? 0 : *offset)
            << std::endl;

        std::tm dt;
        dt.tm_year = p.year - 1900;
        dt.tm_mon = p.month;
        dt.tm_mday = p.day;
        dt.tm_hour = p.hour;
        dt.tm_min = p.minute;
        dt.tm_sec = p.second;
        return std::mktime(&dt);
    }
};

} // namespace date

// ----------------------------------------------------------------------------
void check_rfc1123()
{
    const std::string values[] = {
        std::string("Tue, 31 Dec 2010 23:59:59 GMT"),
        std::string("Tue, 31 Dec 2010 23:59:59 EDT"),
        std::string("Tue, 31 Dec 2010 23:59:59 +0430"),
        std::string("31 Dec 2010 23:59:59 GMT"),
        std::string("31 Dec 2010 23:59:59 EDT"),
        std::string("31 Dec 2010 23:59:59 +0430"),
        std::string("31 Dec 2010 23:59 GMT"),
        std::string("31 Dec 2010 23:59 EDT"),
        std::string("31 Dec 2010 23:59 +0430"),
    };

    for (auto value : values)
    {
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Value: " << value << std::endl;

        std::istringstream stream(value);
        std::time_t dt{};
        try 
        { 
            stream >> date::format_rfc1123(dt);
        }
        catch (std::exception& e)
        { 
            std::cout << "Exception: " << e.what() << std::endl; 
        }

        std::cout << "Result: failbit = " << (stream.fail() ? "true" : "false") << ", dt = " << std::put_time(std::gmtime(&dt), "%c") << std::endl;
    }
}

// ----------------------------------------------------------------------------
void check_rfc3339()
{
    const std::string values[] = {
        std::string("1985-04-12T23:20:50.52Z"),
        std::string("1996-12-19T16:39:57-08:00"),
        std::string("1990-12-31T23:59:60Z"),
        std::string("1990-12-31T15:59:60+08:00"),
        std::string("1937-01-01T12:00:27.87+00:20"),
    };

    for (auto value : values)
    {
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Value: " << value << std::endl;

        std::istringstream stream(value);
        std::time_t dt{};
        try 
        {
            stream >> date::format_rfc3339(dt);
        }
        catch (std::exception& e)
        { 
            std::cout << "Exception: " << e.what() << std::endl; 
        }
        
        std::cout << "Result: failbit = " << (stream.fail() ? "true" : "false") << ", dt = " << std::put_time(std::gmtime(&dt), "%c") << std::endl;
    }
}

// ----------------------------------------------------------------------------
void check_wide()
{
    const std::wstring values[] = {
        std::wstring(L"Tue, 31 Dec 2010 23:59:59 GMT"),
        std::wstring(L"31 Dec 2010 23:59:59 +0430"),
    };

    for (auto value : values)
    {
        std::wcout << "-----------------------------------------" << std::endl;
        std::wcout << "Value: " << value << std::endl;

        std::wistringstream stream(value);
        std::time_t dt{};
        try 
        {
            stream >> date::format_rfc1123(dt);
        }
        catch (std::exception& e)
        { 
            std::wcout << "Exception: " << e.what() << std::endl; 
        }
        
        std::wcout << "Result: failbit = " << (stream.fail() ? "true" : "false") << ", dt = " << std::put_time(std::gmtime(&dt), L"%c") << std::endl;
    }
}

// ----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << " Checking RFC1123..." << std::endl;
    check_rfc1123();
    
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << " Checking RFC3339..." << std::endl;
    check_rfc3339();

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << " Checking wide..." << std::endl;
    check_wide();

	return 0;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif // defined(_MSC_VER)
