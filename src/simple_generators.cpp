#include <iostream>
#include <array>
#include <string>


#include "./iota.h"
#include "./string_prepend.h"
#include "./throwing_parse_ints.h"


int main()
{
    for (auto i : iota(3000, 3042))
    {
        std::cout << i << std::endl;
    }

    for (const auto& s : string_prepend(std::array{"strange", "awesome", "stackless", "complicated"}, "very "))
    {
        std::cout << s << std::endl;
    }

    // throwing_parse_ints with catch_errors=true: invalid strings yield -1
    {
        std::array<std::string, 5> input = {"42", "hello", "7", "", "100"};
        std::cout << "throwing_parse_ints (catch_errors=true): ";
        for (auto val : throwing_parse_ints(input, true))
        {
            std::cout << val << ", ";
        }
        std::cout << std::endl;
    }

    // throwing_parse_ints with catch_errors=false: exception escapes to caller
    {
        std::array<std::string, 5> input = {"42", "hello", "7", "", "100"};
        std::cout << "throwing_parse_ints (catch_errors=false): ";
        try
        {
            for (auto val : throwing_parse_ints(input, false))
            {
                std::cout << val << ", ";
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "\nCaught exception: " << e.what() << std::endl;
        }
    }

    // Early destruction test: break mid-iteration to exercise destroySuspendedCoro
    {
        std::array<std::string, 5> input = {"10", "20", "bad", "40", "50"};
        std::cout << "throwing_parse_ints (early break): ";
        for (auto val : throwing_parse_ints(input, true))
        {
            std::cout << val << ", ";
            if (val == 20) break;
        }
        std::cout << std::endl;
    }
}
