#ifndef GENERATOR_REWRITE_EXAMPLES_INLINE_COROUTINE_FRAME_H
#define GENERATOR_REWRITE_EXAMPLES_INLINE_COROUTINE_FRAME_H

#include <stdexcept>
#include "./coroutine_handle.h"
#include "./type_traits.h"
#include "./coro_storage.h"
#include "./macros.h"

// A CRTP base class for inline (stackful) coroutine frames. Nearly identical to `CoroImpl`,
// but `ramp()` returns the frame by value (no heap allocation) and `destroy()` does not
// deallocate the frame.
template <typename Derived, typename PromiseType, bool isNoexcept>
struct InlineCoroImpl
{
    // Required by the `Handle` class: the `promise` has to be declared directly after the `HandleFrame`.
    PromiseType pt;

    static void CHECK()
    {
        /*
        static_assert(offsetof(InlineCoroImpl, pt) -
                      offsetof(InlineCoroImpl, frm) ==
                      Handle<PromiseType>::promise_offset);
                      */
    }

    // index of the current suspension point.
    size_t curState = 0;

    // Needed for exception handling when walking up the stack.
    static constexpr size_t CO_NO_TRY_BLOCK = static_cast<size_t>(-1);
    size_t currentTryBlock_ = CO_NO_TRY_BLOCK;

    // Buffers for the `initial_suspend()` and `final_suspend()` awaiters.
    [[no_unique_address]] coro_storage<decltype(std::declval<PromiseType&>().initial_suspend())&, true>
    initial_awaiter_;
    [[no_unique_address]] coro_storage<decltype(std::declval<PromiseType&>().final_suspend())&, true> final_awaiter_;

    using Hdl = InlineHandle<Derived&>;
    Hdl getHandle() { return Hdl(derived()); }

    PromiseType& promise() { return pt; }

    bool done_ = false;

    bool done() const
    {
        return done_;
    }

    void setDone()
    {
        done_ = true;
    }


    // Type erased `destroy` function. Unlike `CoroImpl::destroy`, does NOT deallocate the frame.
    void destroy()
    {
        if (!done())
        {
            derived().destroySuspendedCoro(curState);
        }
        else
        {
            final_awaiter_.get().ref_.await_resume();
            final_awaiter_.destroy();
        }
    }

    // Constructor, set up the function pointers at the beginning of the frame.
    InlineCoroImpl()
    {
        CHECK();
    }

    Derived& derived() { return *static_cast<Derived*>(this); }

    void doStep() noexcept(isNoexcept)
    {
        if constexpr (isNoexcept)
        {
            derived().doStepImpl();
        }
        else
        {
            try
            {
                derived().doStepImpl();
            }
            catch (...) { handleException(std::current_exception(), curState); }
        }
    }

    // The inline coroutine `ramp` function. Returns the frame by value (NRVO, no heap allocation).
    template <typename... CoroArgs>
    static /*Derived*/ auto ramp(CoroArgs&&... coroArgs)
    {
        Derived frame{std::forward<CoroArgs>(coroArgs)...};

        // For stackful coroutines, the `get_return_object` can sometimes not be constructed from the promise alone,
        // In particular resuming coroutines like generators need to have the coroutine frame embedded in the stack.
        constexpr bool resIsStackless = PromiseType::return_object_is_stackless;

        auto ret = [&]()
        {
            if constexpr (resIsStackless) { return frame.pt.get_return_object(); }
            else { return 0; }
        }();

        CO_STORAGE_CONSTRUCT(frame.initial_awaiter_, (frame.pt.initial_suspend()));
        if constexpr (resIsStackless)
        {
            CO_AWAIT_IMPL_IMPL(frame.initial_awaiter_.get().ref_,
                               frame.getHandle(), ret);
        } else
        {
            CO_AWAIT_IMPL_IMPL(frame.initial_awaiter_.get().ref_,
                               frame.getHandle(), frame);
        }
        frame.doStep();
        if constexpr (resIsStackless) { return ret; } else
        {
            return frame;
        }
    }

    // Function that is called when exception is thrown inside the `doStep()/resume()` function.
    void handleException(std::exception_ptr eptr, size_t& nextState)
    {
        nextState = derived().dispatchExceptionHandling(std::move(eptr));
        if (!done())
        {
            derived().doStep();
        }
    }
};

#endif //GENERATOR_REWRITE_EXAMPLES_INLINE_COROUTINE_FRAME_H
