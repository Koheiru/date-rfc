#include <iostream>
#include <sstream>
#include <iomanip>
#include <date-rfc/rfc-1123.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif // defined(_MSC_VER)

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

    for (auto value : values)
    {
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Value: " << value << std::endl;

        std::time_t dt{};
        std::istringstream stream(value);
        stream >> date::format_rfc1123(dt);
        std::cout << "Result: failbit = " << (stream.fail() ? "true" : "false") << ", dt = " << std::put_time(std::gmtime(&dt), "%c") << std::endl;
        std::cout << "Revert: " << date::format_rfc1123(dt) << std::endl;
    }
}

// ----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    check_rfc1123();
    return 0;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif // defined(_MSC_VER)
