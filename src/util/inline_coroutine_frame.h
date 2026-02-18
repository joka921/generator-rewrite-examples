#ifndef GENERATOR_REWRITE_EXAMPLES_INLINE_COROUTINE_FRAME_H
#define GENERATOR_REWRITE_EXAMPLES_INLINE_COROUTINE_FRAME_H

#include <cassert>
#include <stdexcept>

#include "./coro_storage.h"
#include "./coroutine_handle.h"
#include "./macros.h"
#include "./type_traits.h"

// A CRTP base class for inline (stackful) coroutine frames. Nearly identical to
// `stackless_coro_crtp`, but `ramp()` returns the frame by value (no heap allocation) and
// `destroy()` does not deallocate the frame.
template <typename Derived, typename PromiseType, bool isNoexcept>
struct stackful_coro_crtp {
  PromiseType promise_;

  // index of the current suspension point.
  size_t suspendIdx_ = 0;

  // Needed for exception handling when walking up the stack.
  static constexpr size_t CO_NO_TRY_BLOCK = static_cast<size_t>(-1);
  size_t currentTryBlock_ = CO_NO_TRY_BLOCK;

  // Buffers for the `initial_suspend()` and `final_suspend()` awaiters.
  [[no_unique_address]] coro_storage<decltype(std::declval<PromiseType&>().initial_suspend())&,
                                     true> initial_awaiter_;
  [[no_unique_address]] coro_storage<decltype(std::declval<PromiseType&>().final_suspend())&, true>
      final_awaiter_;

  using handle_type = stackful_coroutine_handle<Derived&>;
  handle_type getHandle() { return handle_type(derived()); }

  PromiseType& promise() { return promise_; }

  bool done_ = false;

  bool done() const { return done_; }

  void setDone() { done_ = true; }

  // Type erased `destroy` function. Unlike `stackless_coro_crtp::destroy`, does NOT deallocate the
  // frame or call its destructor, this is done automatically once the frame goes out of scope.
  void destroy() {
    if (!done()) {
      derived().destroySuspendedCoro(suspendIdx_);
    } else {
      final_awaiter_.get().ref_.await_resume();
      final_awaiter_.destroy();
    }
  }

  Derived& derived() { return *static_cast<Derived*>(this); }

  void doStep() noexcept(isNoexcept) {
    if constexpr (isNoexcept) {
      [[maybe_unused]] auto h = derived().doStepImpl();
      assert(!h);
    } else {
      try {
        [[maybe_unused]] auto h = derived().doStepImpl();
        assert(!h);
      } catch (...) {
        handleException(std::current_exception(), suspendIdx_);
      }
    }
  }

  // The inline coroutine `ramp` function. The return type depends on the promise:
  // if `PromiseType::return_object_is_stackless` is true (meaning that the return object doesn't
  // depend on the coroutine frame because the frame has run to completion before the return object
  // is returned (used for fully eager coroutines like the ` optional`  coroutine.) Otherwise, the
  // `Derived` object (the actual coroutine frame/ state machine) is returned directly, and it is
  // the response of the caller to construct the actual return object (e.g. a stackful generator)
  // from it.
  template <typename... CoroArgs>
  static auto ramp(CoroArgs&&... coroArgs) {
    Derived frame{std::forward<CoroArgs>(coroArgs)...};

    // For stackful coroutines, the `get_return_object` can sometimes not be constructed from the
    // promise alone, In particular resuming coroutines like generators need to have the coroutine
    // frame embedded in the stack.
    constexpr bool resIsStackless = PromiseType::return_object_is_stackless;

    auto ret = [&]() {
      if constexpr (resIsStackless) {
        return frame.promise_.get_return_object();
      } else {
        return 0;
      }
    }();

    CO_STORAGE_CONSTRUCT(frame.initial_awaiter_, (frame.promise_.initial_suspend()));
    if constexpr (resIsStackless) {
      CO_AWAIT_IMPL_IMPL(frame.initial_awaiter_.get().ref_, frame.getHandle(), ret);
    } else {
      CO_AWAIT_IMPL_IMPL(frame.initial_awaiter_.get().ref_, frame.getHandle(), frame);
    }
    frame.doStep();
    if constexpr (resIsStackless) {
      return ret;
    } else {
      return frame;
    }
  }

  // Function that is called when exception is thrown inside the `doStep()/resume()` function.
  void handleException(std::exception_ptr eptr, size_t& nextState) {
    nextState = derived().dispatchExceptionHandling(std::move(eptr));
    if (!done()) {
      derived().doStep();
    }
  }
};

#endif  // GENERATOR_REWRITE_EXAMPLES_INLINE_COROUTINE_FRAME_H