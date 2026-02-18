#ifndef GENERATOR_REWRITE_EXAMPLES_IOTA_UNIFIED_H
#define GENERATOR_REWRITE_EXAMPLES_IOTA_UNIFIED_H

#include <cassert>

#include "generator/unified_generator.h"
#include "util/coroutine_frame.h"
#include "util/inline_coroutine_frame.h"
#include "util/macros.h"

template <bool isStackless, typename Frame, typename promise, bool isNoexcept>
using FrameCRTP = std::conditional_t<isStackless, stackless_coro_crtp<Frame, promise, isNoexcept>,
                                     stackful_coro_crtp<Frame, promise, isNoexcept>>;
/**
 * A simple `iota` generator using the unified heap_generator alias.
 * Direct replacement of:
 *   generator<int, stackless_coroutine_handle> iota(int start, int end) { ... }
 */
template <bool stackless = true>
auto iota_unified(int start, int end) {
  using promise_type = std::conditional_t<stackless, heap_generator<int>::promise_type,
                                          detail::unified_generator_promise<int>>;
  struct CoroFrame : FrameCRTP<stackless, CoroFrame, promise_type, true> {
    using CoroFrameBase = FrameCRTP<stackless, CoroFrame, promise_type, true>;
    int start_;
    int end_;

    CoroFrame(int s, int e) : start_(s), end_(e) {}

    stackless_coroutine_handle<void> doStepImpl() {
      switch (this->suspendIdx_) {
        case 0:
          break;
        case 1:
          goto label_1;
      }

      CO_GET(initial_awaiter_).await_resume();
      this->initial_awaiter_.destroy();
      while (this->start_ < this->end_) {
        CO_YIELD(1, initial_awaiter_, (this->start_));
        ++this->start_;
      }
      CO_RETURN_VOID(2, final_awaiter_);
    }

    void destroySuspendedCoro(size_t suspendIdx_) {
      switch (suspendIdx_) {
        case 0:
          this->initial_awaiter_.destroy();
          return;
        case 1:
          this->initial_awaiter_.destroy();
        case 2:
          return;
      }
    }
  };
  if constexpr (stackless) {
    return CoroFrame::ramp(start, end);
  } else {
    return inline_gen<int, CoroFrame>{
        stackful_coroutine_handle<CoroFrame>{CoroFrame::ramp(start, end)}};
  }
}
#endif  // GENERATOR_REWRITE_EXAMPLES_IOTA_UNIFIED_H