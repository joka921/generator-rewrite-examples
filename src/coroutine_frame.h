#ifndef GENERATOR_REWRITE_EXAMPLES_COROUTINE_FRAME_H
#define GENERATOR_REWRITE_EXAMPLES_COROUTINE_FRAME_H

#include <stdexcept>
#include "./coroutine_handle.h"
#include "./type_traits.h"
#include "./coro_storage.h"
#include "./macros.h"

// A CRTP base class for coroutine frame + state machine. Contains all members and functions that only depend on the `PromiseType` of a coroutine type.
template<typename Derived, typename PromiseType, bool isNoexcept>
struct CoroImpl {
    // Required by the `Handle` class:: the `promise` has to be declared directly after the `HandleFrame`.
    HandleFrame frm;
    PromiseType pt;

    static void CHECK() {
        static_assert(offsetof(CoroImpl, pt) -
                      offsetof(CoroImpl, frm) ==
                      Handle<PromiseType>::promise_offset);
    }
    // index of the current suspension point.
    size_t curState = 0;

    // Needed for exception handling when walking up the stack.
    // stores the currently active try-block, and `CO_NO_TRY_BLOCK` means that there is not active try-block,
    // s.t. an exception needs to call `promise_type::unhandled_exception()` directly.
    static constexpr size_t CO_NO_TRY_BLOCK = static_cast<size_t>(-1);
    size_t currentTryBlock_ = CO_NO_TRY_BLOCK;

    // Buffers for the `initial_suspend()` and `final_suspend()` awaiters.
    coro_storage<decltype(std::declval<PromiseType&>().initial_suspend())&, true> initial_awaiter_;
    coro_storage<decltype(std::declval<PromiseType&>().final_suspend())&, true> final_awaiter_;

    using Hdl = Handle<PromiseType>;
    PromiseType &promise() { return pt; }


    bool done() const
    {
        return frm.resumeFunc == nullptr;
    }
    void setDone()
    {
        frm.resumeFunc = nullptr;
    }

    // Helper function to cast the `void*` which is obtained from the `Handle` into a `Derived*`.
    // That pointer always points to the `resumeFunc` inside the handle frame.
    static auto fromHandle(void *blubb) -> Derived* {
        auto res = static_cast<Derived *>(reinterpret_cast<CoroImpl *>(
            reinterpret_cast<char *>(blubb) -
            offsetof(CoroImpl, frm)));
        return res;
    }

    // Type erased `resume` function, needed for the indirect type-erased call through the `handle`.
    static void resume(void *blubb) { [[clang::musttail]] return doStep(fromHandle(blubb)); }

    // Type erased `destroy` function.
    static void destroy(void *blubb) {
        auto *self = fromHandle(blubb);
        // If the coroutine is currently suspended at a suspension point that is not the final suspension point, we first have to destroy the local variables inside
        // the coroutine frame.
        if (!self->done()) {
            self->destroySuspendedCoro(self->curState);
        } else {
            // The coroutine is suspended at its final suspension point, local variables already have been destroyed,
            // but the final_awaiter_ still exists.
            self->final_awaiter_.get().ref_.await_resume();
            self->final_awaiter_.destroy();
        }
        // Delete the `Derived` object itself (it was allocated on the heap via the `ramp()` below.
        if constexpr (coro_detail::has_promise_delete<PromiseType>::value) {
            self->~Derived();
            PromiseType::operator delete(
                static_cast<void *>(self), sizeof(Derived));
        } else if constexpr (coro_detail::has_promise_delete_unsized<PromiseType>::value) {
            self->~Derived();
            PromiseType::operator delete(static_cast<void *>(self));
        } else {
            delete self;
        }
    }

    // Type erased `destroy` function.
    void destroy() {
        auto *self = &derived();
        // If the coroutine is currently suspended at a suspension point that is not the final suspension point, we first have to destroy the local variables inside
        // the coroutine frame.
        if (!self->done()) {
            self->destroySuspendedCoro(self->curState);
        } else {
            // The coroutine is suspended at its final suspension point, local variables already have been destroyed,
            // but the final_awaiter_ still exists.
            self->final_awaiter_.get().ref_.await_resume();
            self->final_awaiter_.destroy();
        }
        // Delete the `Derived` object itself (it was allocated on the heap via the `ramp()` below.
        if constexpr (coro_detail::has_promise_delete<PromiseType>::value) {
            self->~Derived();
            PromiseType::operator delete(
                static_cast<void *>(self), sizeof(Derived));
        } else if constexpr (coro_detail::has_promise_delete_unsized<PromiseType>::value) {
            self->~Derived();
            PromiseType::operator delete(static_cast<void *>(self));
        } else {
            delete self;
        }
    }

    // Constructor, set up the function pointers at the beginning of the frame.
    CoroImpl() {
        CHECK();
        frm.resumeFunc = &CoroImpl::resume;
        frm.destroyFunc = &CoroImpl::destroy;
    }

    Derived& derived() {return *static_cast<Derived*>(this);}

    static void doStep(void* ptrToDerived) /*noexcept(isNoexcept)*/ {
        [[clang::musttail]] return Derived::doStepImpl(ptrToDerived);
        //[[clang::musttail]] return Derived::doStepImpl(static_cast<Derived*>(ptrToDerived));
        /*
        if constexpr (isNoexcept)
        {
            [[clang::musttail]] return Derived::doStepImpl(static_cast<Derived*>(ptrToDerived));
        } else
        {
            try {
                return Derived::doStepImpl(ptrToDerived);
            } catch (...)
            {

                 auto& derived = *static_cast<Derived*>(ptrToDerived);
                derived.handleException(std::current_exception(), derived.curState);
            }
        }
        */
    }

    // The coroutine `ramp` function. Gets the arguments to the coroutine, and returns the coroutine object
    // via `get_return_object`.
    template<typename... CoroArgs>
    static auto ramp(CoroArgs&&... coroArgs) {
        // Allocate space for the frame, and placement new into it.
        void *__coro_mem = coro_detail::promise_allocate<PromiseType>(sizeof(Derived), std::forward<CoroArgs>(coroArgs)...);
        auto *frame = new(__coro_mem) Derived{std::forward<CoroArgs>(coroArgs)...};
        // `get_return_object()` result is temporarily stored on the stack.
        auto ret = frame->pt.get_return_object();
        // Call, store, and `co_await` the `initial_suspend`.
        CO_STORAGE_CONSTRUCT(frame->initial_awaiter_, (frame->pt.initial_suspend()));
        // If the coroutine suspends, return the `ret` to the caller.
        CO_AWAIT_IMPL_IMPL(frame->initial_awaiter_.get().ref_, Handle<PromiseType>::from_promise(frame->pt), ret);
        // If we reach here, the coroutine was not suspended (likely because of `suspend_never`, so we directly
        // run the coroutine and return the `ret` once it first suspends.
        frame->doStep(frame);
        return ret;
    }

    // Function that is called when exception is thrown inside the `doStep()/resume()` function.
    // TODO Do we really need the `nextState` parameter, or is this just always the `currentState` member?
    void handleException(std::exception_ptr eptr, size_t &nextState) {
        // `dispatchExceptionHandling()` must be provided by the `Derived` class.
        nextState = derived().dispatchExceptionHandling(std::move(eptr));

        // Exception was caught inside the class, just resume normal exection.
        if (!done()) {
            derived().doStep();
        }
    }

};


#endif //GENERATOR_REWRITE_EXAMPLES_COROUTINE_FRAME_H