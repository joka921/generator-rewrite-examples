#include <iostream>

#include "./task_example.h"

int main()
{
    auto t = add_values(3, 1'000'000'000);
    t.start();
    std::cout << t.result() << std::endl; // 14
}
