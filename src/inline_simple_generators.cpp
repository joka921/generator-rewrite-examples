#include <iostream>

#include "./inline_iota.h"

int main()
{
    // Basic iteration test.
    for (auto i : inline_iota(3000, 3042))
    {
        std::cout << i << std::endl;
    }

    // Early destruction test: break mid-iteration to exercise destroySuspendedCoro.
    {
        std::cout << "Early break test: ";
        for (auto i : inline_iota(0, 10))
        {
            std::cout << i << " ";
            if (i == 3) break;
        }
        std::cout << std::endl;
    }
}
