#ifndef GENERATOR_REWRITE_EXAMPLES_TASK_EXAMPLE_H
#define GENERATOR_REWRITE_EXAMPLES_TASK_EXAMPLE_H

#include <iostream>

#include "task/task.h"
#include "util/coroutine_frame.h"
#include "util/macros.h"

/**
 * Manually lowered equivalent of:
 *   task<size_t, stackless_coroutine_handle> compute_value(size_t x) { co_return x * 2; }
 */
inline task<size_t, stackless_coroutine_handle> compute_value(size_t x) {
  using promise_type = task<size_t, stackless_coroutine_handle>::promise_type;
  struct CoroFrame : stackless_coro_crtp<CoroFrame, promise_type, true> {
    using CoroFrameBase = stackless_coro_crtp<CoroFrame, promise_type, true>;
    size_t x_;

    CoroFrame(size_t x) : x_(x) {}

    stackless_coroutine_handle<void> doStepImpl() {
      switch (this->suspendIdx_) {
        case 0:
          break;
      }

      CO_GET(initial_awaiter_).await_resume();
      this->initial_awaiter_.destroy();

      // co_return x_ * 2;
      CO_RETURN_VALUE(1, final_awaiter_, (this->x_ * 2));
    }

    void destroySuspendedCoro(size_t suspendIdx_) {
      switch (suspendIdx_) {
        case 0:
          this->initial_awaiter_.destroy();
          return;
        case 1:
          return;
      }
    }
  };
  return CoroFrame::ramp(x);
}

/**
 * Manually lowered equivalent of:
 *   task<size_t, stackless_coroutine_handle> add_values(size_t a, size_t b) {
 *       size_t va = co_await compute_value(a);
 *       size_t vb = co_await compute_value(b);
 *       co_return va + vb;
 *   }
 */
inline task<size_t, stackless_coroutine_handle> add_values(size_t a, size_t b) {
  using promise_type = task<size_t, stackless_coroutine_handle>::promise_type;
  struct CoroFrame : stackless_coro_crtp<CoroFrame, promise_type, false> {
    using CoroFrameBase = stackless_coro_crtp<CoroFrame, promise_type, true>;
    size_t a_;
    size_t b_;
    size_t res_;
    size_t va_;
    size_t vb_;

    // Storage for the inner task and its awaiter.
    coro_storage<task<size_t, stackless_coroutine_handle>&, true> task_storage_;
    coro_storage<detail::task_awaiter<size_t, stackless_coroutine_handle>&, true> awaiter_storage_;

    CoroFrame(size_t a, size_t b) : a_(a), b_(b) {}

    ExceptionResult dispatchExceptionHandling() {
      // not implemented for now, just checking for the symmetric transfer.
      std::terminate();
    }

    stackless_coroutine_handle<void> doStepImpl() {
      switch (this->suspendIdx_) {
        case 0:
          break;
        case 1:
          goto label_1;
        case 2:
          goto label_2;
      }

      CO_GET(initial_awaiter_).await_resume();
      this->initial_awaiter_.destroy();

      this->res_ = 0;
      while (this->a_ < this->b_) {
        // size_t va = co_await compute_value(a_);
        CO_INIT(task_storage_, (compute_value(this->a_)));
        CO_AWAIT(1, awaiter_storage_, CO_GET(task_storage_), this->va_ =);
        this->task_storage_.destroy();

        CO_INIT(task_storage_, (compute_value(this->b_)));
        CO_AWAIT(2, awaiter_storage_, CO_GET(task_storage_), this->vb_ =);
        this->task_storage_.destroy();
        this->res_ += this->va_ + this->vb_;
        ++this->a_;
        --this->b_;
      }

      // co_return va_ + vb_;
      CO_RETURN_VALUE(3, final_awaiter_, (this->res_));
    }

    void destroySuspendedCoro(size_t suspendIdx_) {
      switch (suspendIdx_) {
        case 0:
          this->initial_awaiter_.destroy();
          return;
        case 1:
          awaiter_storage_.destroy();
          task_storage_.destroy();
          return;
        case 2:
          awaiter_storage_.destroy();
          task_storage_.destroy();
          return;
        case 3:
          return;
      }
    }
  };
  return CoroFrame::ramp(a, b);
}

#endif  // GENERATOR_REWRITE_EXAMPLES_TASK_EXAMPLE_H