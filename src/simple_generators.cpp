#include <iostream>
#include <array>


#include "./iota.h"
#include "./string_prepend.h"


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
}
