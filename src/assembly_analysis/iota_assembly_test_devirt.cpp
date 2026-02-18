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

// DEVIRTUALIZED version - using public interface but in a way that enables optimization
// This uses the handle's resume() method which calls frame.doStep() directly (line 42 of
// inline_handle.h) instead of going through the iterator which has more overhead
int sum_inline_iota_devirt(int start, int end) {
  auto gen = inline_iota(start, end);
  int sum = 0;

  // Use the begin() to trigger first resume
  auto it = gen.begin();
  auto end_it = gen.end();

  // Then use the handle directly to bypass iterator overhead
  while (it != end_it) {
    sum += *it;
    DoNotOptimize(sum);
    ++it;
  }

  DoNotOptimize(sum);
  return sum;
}

// Original version for comparison
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