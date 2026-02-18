#ifndef GENERATOR_REWRITE_EXAMPLES_COROUTINE_FRAME_H
#define GENERATOR_REWRITE_EXAMPLES_COROUTINE_FRAME_H

#include <stdexcept>

#include "./coro_storage.h"
#include "./coroutine_handle.h"
#include "./macros.h"
#include "./type_traits.h"

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
  static stackless_coroutine_handle<void> resume(void* blubb) { return doStep(fromHandle(blubb)); }

  // Type erased `destroy` function.
  static void destroy(void* blubb) {
    auto* self = fromHandle(blubb);
    self->destroy();
  }

    stackless_coroutine_handle<void> await_final_suspend()
  {
      this->setDone();
      this->promise().unhandled_exception();
      auto* self = this;
      CO_RETURN_IMPL_IMPL(final_awaiter_);

      deleteFrame();
      return {};
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
    auto* self = &derived();
    // If the coroutine is currently suspended at a suspension point that is not the final
    // suspension point, we first have to destroy the local variables inside the coroutine frame.
    if (!self->done()) {
      self->destroySuspendedCoro(self->suspendIdx_);
    } else {
      // The coroutine is suspended at its final suspension point, local variables already have been
      // destroyed, but the final_awaiter_ still exists.
      self->final_awaiter_.get().ref_.await_resume();
      self->final_awaiter_.destroy();
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

  // The type-erased actor function. calls into `Derived::doStepImpl`.
  static stackless_coroutine_handle<void> doStep(void* ptrToDerived) noexcept(isNoexcept) {
    if constexpr (isNoexcept) {
      return Derived::doStepImpl(ptrToDerived);
    } else {
      stackless_coroutine_handle<void> ret;
      auto& derived = *static_cast<Derived*>(ptrToDerived);
      try {
        return Derived::doStepImpl(ptrToDerived);
      } catch (...) {
        ret = derived.handleException();
      }
        // As the `handleException` function only catches exceptions and then returns, we have to do
        // another step, as nobody has actually suspended the coroutine. We don't do another step if
        // the frame is `done` which can happen either for uncaught exceptions, or for exp
        if (!derived.done())
        {
            assert(!ret);
            doStep(ptrToDerived);
        }
        return ret;
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
  // Returns stackless_coroutine_handle<void> for the trampoline: if the catch handler does
  // symmetric transfer via `co_return`, the handle is propagated back up.
  stackless_coroutine_handle<void> handleException() {
    // `dispatchExceptionHandling()` must be provided by the `Derived` class.
    return derived().dispatchExceptionHandling();
  }
};

#endif  // GENERATOR_REWRITE_EXAMPLES_COROUTINE_FRAME_H