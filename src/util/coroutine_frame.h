#ifndef GENERATOR_REWRITE_EXAMPLES_COROUTINE_FRAME_H
#define GENERATOR_REWRITE_EXAMPLES_COROUTINE_FRAME_H

#include <cassert>
#include <stdexcept>

#include "./coro_storage.h"
#include "./coroutine_handle.h"
#include "./macros.h"
#include "./type_traits.h"

// Return type for exception handling in the stackless coroutine path.
// Carries both the handle for symmetric transfer and a flag indicating whether
// the frame has already been destroyed (e.g. because the final awaiter didn't
// suspend). When `frame_destroyed` is true, the caller must not access the
// coroutine frame anymore.
struct ExceptionResult {
  stackless_coroutine_handle<void> handle{};
  bool frame_destroyed = false;
};

// A CRTP base class for coroutine frame + state machine. Contains all members and functions that
// only depend on the `PromiseType` of a coroutine type.
template <typename Derived, typename PromiseType, bool isNoexcept>
struct stackless_coro_crtp {
  // Required by the `stackless_coroutine_handle` class: the `promise` has to be declared directly
  // after the `HandleFrame`.
  HandleFrame frame_;
  PromiseType promise_;

  static void CHECK() {
    static_assert(offsetof(stackless_coro_crtp, promise_) - offsetof(stackless_coro_crtp, frame_) ==
                  stackless_coroutine_handle<PromiseType>::promise_offset);
  }

  // index of the current suspension point.
  size_t suspendIdx_ = 0;

  // Needed for exception handling when walking up the stack.
  // stores the currently active try-block, and `CO_NO_TRY_BLOCK` means that there is no active
  // try-block, s.t. an exception needs to call `promise_type::unhandled_exception()` directly.
  static constexpr size_t CO_NO_TRY_BLOCK = static_cast<size_t>(-1);
  size_t currentTryBlock_ = CO_NO_TRY_BLOCK;

  // Buffers for the `initial_suspend()` and `final_suspend()` awaiters.
  [[no_unique_address]] coro_storage<decltype(std::declval<PromiseType&>().initial_suspend())&,
                                     true> initial_awaiter_;
  [[no_unique_address]] coro_storage<decltype(std::declval<PromiseType&>().final_suspend())&, true>
      final_awaiter_;

  using handle_type = stackless_coroutine_handle<PromiseType>;

  handle_type getHandle() { return handle_type::from_promise(promise_); }
  PromiseType& promise() { return promise_; }

  bool done() const { return frame_.resumeFunc == nullptr; }

  void setDone() { frame_.resumeFunc = nullptr; }

  // Helper function to cast the `void*` which is obtained from the `stackless_coroutine_handle`
  // into a `Derived*`. That pointer always points to the `resumeFunc` inside the handle frame.
  static auto fromHandle(void* blubb) -> Derived* {
    auto res = static_cast<Derived*>(reinterpret_cast<stackless_coro_crtp*>(
        reinterpret_cast<char*>(blubb) - offsetof(stackless_coro_crtp, frame_)));
    return res;
  }

  // Type erased `resume` function, needed for the indirect type-erased call through the `handle`.
  // Returns stackless_coroutine_handle<void> for the trampoline in HandleBase::resume().
  static stackless_coroutine_handle<void> resume(void* blubb) {
    auto* d = fromHandle(blubb);
    return d->doStep();
  }

  // Type erased `destroy` function.
  static void destroy(void* blubb) {
    auto* self = fromHandle(blubb);
    self->destroy();
  }

  // Called from dispatchExceptionHandling() when an exception is unhandled.
  // Stores the exception via promise().unhandled_exception(), sets done, constructs and
  // awaits the final awaiter. If the final awaiter suspends, the frame stays
  // alive (frame_destroyed=false). If it doesn't suspend, the frame is cleaned
  // up and deleted (frame_destroyed=true).
  ExceptionResult unhandled_exception()
  {
      this->setDone();
      this->promise().unhandled_exception();
      CO_STORAGE_CONSTRUCT(this->final_awaiter_, (this->promise().final_suspend()));
      auto& awaiter = this->final_awaiter_.get().ref_;
      if (!awaiter.await_ready()) {
        using type = decltype(awaiter.await_suspend(this->getHandle()));
        if constexpr (std::is_void_v<type>) {
          awaiter.await_suspend(this->getHandle());
          return {{}, false};
        } else if constexpr (std::is_same_v<type, bool>) {
          if (awaiter.await_suspend(this->getHandle())) {
            return {{}, false};
          }
          // await_suspend returned false: didn't actually suspend, fall through.
        } else {
          auto next = awaiter.await_suspend(this->getHandle());
          return {stackless_coroutine_handle<void>{next.ptr}, false};
        }
      }
      // Final awaiter didn't suspend. Clean up and delete the frame.
      awaiter.await_resume();
      this->final_awaiter_.destroy();
      deleteFrame();
      // After deleteFrame(), `this` is freed. We must not access any members.
      // The return value is constructed on the caller's stack, so this is safe.
      return {{}, true};
  }

  void deleteFrame()
  {
      // Destroy and delete the `Derived` object itself (it was allocated on the heap via the `ramp()`
      // below.
      derived().~Derived();
      if constexpr (coro_detail::has_promise_delete<PromiseType>::value) {
          PromiseType::operator delete(static_cast<void*>(this), sizeof(Derived));
      } else if constexpr (coro_detail::has_promise_delete_unsized<PromiseType>::value) {
          PromiseType::operator delete(static_cast<void*>(this));
      } else {
          delete this;
      }

  }
  // Actual implementation of `destroy` function.
  void destroy() {
    // If the coroutine is currently suspended at a suspension point that is not the final
    // suspension point, we first have to destroy the local variables inside the coroutine frame.
    if (!derived().done()) {
      derived().destroySuspendedCoro(derived().suspendIdx_);
    } else {
      // The coroutine is suspended at its final suspension point, local variables already have been
      // destroyed, but the final_awaiter_ still exists.
      derived().final_awaiter_.get().ref_.await_resume();
      derived().final_awaiter_.destroy();
    }
      deleteFrame();
  }

  // Constructor, set up the function pointers at the beginning of the frame.
  stackless_coro_crtp() {
    CHECK();
    frame_.resumeFunc = &stackless_coro_crtp::resume;
    frame_.destroyFunc = &stackless_coro_crtp::destroy;
  }

  Derived& derived() { return *static_cast<Derived*>(this); }

  // The actor function. Calls into `derived().doStepImpl()`.
  stackless_coroutine_handle<void> doStep() noexcept(isNoexcept) {
    if constexpr (isNoexcept) {
      return derived().doStepImpl();
    } else {
      auto& d = derived();
      try {
        return d.doStepImpl();
      } catch (...) {
        auto [handle, frame_destroyed] = d.handleException();
        // If the frame was already destroyed (e.g. final awaiter was
        // suspend_never), we must not touch `d` anymore.
        if (frame_destroyed) {
          return handle;
        }
        // As the `handleException` function only catches exceptions and then returns, we have to do
        // another step, as nobody has actually suspended the coroutine. We don't do another step if
        // the frame is `done` which can happen either for uncaught exceptions, or for exceptions
        // that were caught at the final try-block level.
        if (!d.done())
        {
            assert(!handle);
            return d.doStep();
        }
        return handle;
      }
    }
  }

  // The coroutine `ramp` function. Gets the arguments to the coroutine, and returns the coroutine
  // object via `get_return_object`.
  template <typename... CoroArgs>
  static auto ramp(CoroArgs&&... coroArgs) {
    // Allocate space for the frame, and placement new into it.
    void* coroMem = coro_detail::promise_allocate<PromiseType>(sizeof(Derived),
                                                               std::forward<CoroArgs>(coroArgs)...);
    auto* frame = new (coroMem) Derived{std::forward<CoroArgs>(coroArgs)...};
    // `get_return_object()` result is temporarily stored on the stack.
    auto ret = frame->promise_.get_return_object();
    // Call, store, and `co_await` the `initial_suspend`.
    CO_STORAGE_CONSTRUCT(frame->initial_awaiter_, (frame->promise_.initial_suspend()));
    // If the coroutine suspends, return the `ret` to the caller.
    auto handle = frame->getHandle();
    CO_AWAIT_IMPL_IMPL(frame->initial_awaiter_.get().ref_, handle, ret);
    // If we reach here, the coroutine was not suspended (likely because of `suspend_never`, so we
    // directly run the coroutine and return the `ret` once it first suspends.
    handle.resume();
    return ret;
  }

  // Function that is called when exception is thrown inside the `doStep()/resume()` function.
  // Returns ExceptionResult: the handle for symmetric transfer and whether the frame was destroyed.
  ExceptionResult handleException() {
    // `dispatchExceptionHandling()` must be provided by the `Derived` class.
    return derived().dispatchExceptionHandling();
  }
};

#endif  // GENERATOR_REWRITE_EXAMPLES_COROUTINE_FRAME_H