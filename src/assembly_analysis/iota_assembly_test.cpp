#include "inline_iota.h"

// Prevent compiler from optimizing away the computation
template <typename T>
void DoNotOptimize(T const& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

// Callback-based iota implementation
class CallbackIota {
 public:
  CallbackIota(int start, int end) : start_(start), end_(end) {}

  template <typename Callback>
  void forEach(Callback&& cb) {
    for (int i = start_; i < end_; ++i) {
      cb(i);
    }
  }

 private:
  int start_;
  int end_;
};

// Sum using inline_iota generator
int sum_inline_iota(int start, int end) {
  auto gen = inline_iota(start, end);
  int sum = 0;
  for (int val : gen) {
    DoNotOptimize(sum += val);
  }
  DoNotOptimize(sum);
  return sum;
}

// Sum using callback-based iota
int sum_callback_iota(int start, int end) {
  CallbackIota gen(start, end);
  int sum = 0;
  gen.forEach([&sum](int val) { DoNotOptimize(sum += val); });
  DoNotOptimize(sum);
  return sum;
}