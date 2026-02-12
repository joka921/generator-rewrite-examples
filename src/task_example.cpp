#include <iostream>

#include "./task_example.h"

int main()
{
    auto t = add_values(3, 4);
    t.start();
    std::cout << t.result() << std::endl; // 14
}
