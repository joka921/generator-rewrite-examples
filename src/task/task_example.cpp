#include "./task_example.h"

#include <iostream>

int main() {
  auto t = add_values(3, 180'000'000);
  t.start();
  std::cout << t.result() << std::endl;  // 14
}