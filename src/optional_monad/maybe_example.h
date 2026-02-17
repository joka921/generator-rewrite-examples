// Optional monad example header
// Demonstrates cross-TU usage and inline coroutine patterns

#ifndef OPTIONAL_MONAD_MAYBE_EXAMPLE_H
#define OPTIONAL_MONAD_MAYBE_EXAMPLE_H

#include "maybe.h"
#include "util/coroutine_frame.h"
#include "util/inline_coroutine_frame.h"
#include <cmath>
#include <cassert>

// Forward declarations - implemented in maybe_example.cpp (cross-TU example)
std::optional<int> safe_divide(int numerator, int denominator);
std::optional<int> chained_calculation(int a, int b, int c, int reps = 1);
std::optional<int> chained_calculation_no_coro(int a, int b, int c, int reps = 1);
std::optional<int> with_exceptions(int x);
std::optional<int> with_exceptions_no_coro(int x);

// Inline implementation - defined in header
inline std::optional<int> safe_sqrt(int x) {
    if (x < 0)
    {
        return std::nullopt;
    }
    return static_cast<int>(std::sqrt(x));
}

// Chaining example: chains multiple maybe operations
// Demonstrates short-circuiting when any operation returns nullopt
inline std::optional<int> chained_calculation_header(int a, int b, int c) {
    using promise_type = maybe_promise<int>;
    struct CoroFrame : InlineCoroImpl<CoroFrame, promise_type, false> {
        using CoroFrameBase = CoroImpl<CoroFrame, promise_type, false>;
        const int a_;
        const int b_;
        const int c_;

        // Storage for awaited values and awaiters
        coro_storage<maybe_awaiter<int>&, true> awaiter_storage_;

        // Track construction state
        struct {
            bool awaiter_storage_ = false;
        } __constructed;

        CoroFrame(int a, int b, int c) : a_(a), b_(b), c_(c) {}

        // The original generic doStepImpl
        /*
        static Handle<void> doStepImpl(void* seflPtr) {
            auto* self = static_cast<CoroFrame*>(seflPtr);
            // As we never resume from a suspended state,
            // we can completely get rid of the switch-goto block.

            // Resume initial suspend
            CO_GET(initial_awaiter_).await_resume();
            self->initial_awaiter_.destroy();

            // int r1 = co_await safe_divide(a_, b_);
            CO_AWAIT(1, awaiter_storage_, safe_divide(self->a_, self->b_), int result1_=);

            // int r2 = co_await safe_sqrt(c_);
            CO_AWAIT(2, awaiter_storage_, safe_sqrt(self->c_), int result2_=);

            // co_return r1 + r2;
            CO_RETURN_VALUE(3, final_awaiter_, result1_ + result2_);
        }
        */
       __attribute__((no_stack_protector)) static Handle<void> doStepImpl(void* seflPtr) {
            auto* self = static_cast<CoroFrame*>(seflPtr);
            // As we never resume from a suspended state,
            // we can completely get rid of the switch-goto block.

            // Resume initial suspend
            CO_GET(initial_awaiter_).await_resume();
            self->initial_awaiter_.destroy();

            // int r1 = co_await safe_divide(a_, b_);
            maybe_awaiter<int> awaiter{safe_divide(self->a_, self->b_)};
            if (!awaiter.await_ready())
            {
                awaiter.await_suspend(self->getHandle());
            }
            int result1 = awaiter.await_resume();
            //CO_AWAIT(1, awaiter_storage_, safe_divide(self->a_, self->b_), int result1_=);

            // int r2 = co_await safe_sqrt(c_);
            //CO_AWAIT(2, awaiter_storage_, safe_sqrt(self->c_), int result2_=);
            maybe_awaiter<int> awaiter2{safe_sqrt(self->c_)};
            if (!awaiter2.await_ready())
            {
                awaiter2.await_suspend(self->getHandle());
            }
            int result2 = awaiter2.await_resume();

            // co_return r1 + r2;
            CO_RETURN_VALUE(3, final_awaiter_, result1 + result2);
        }

        void doStep()
        {
            [[maybe_unused]] auto h = doStepImpl(this);
            assert(!h);
        }

        void destroySuspendedCoro(size_t curState) {
            switch (curState) {
            case 0:
                this->initial_awaiter_.destroy();
                return;
            case 3:
                return;
            }
        }

        size_t dispatchExceptionHandling(std::exception_ptr eptr) {
            try {
                std::rethrow_exception(eptr);
            } catch (...) {
                // Other exceptions: store for later rethrow
                promise().unhandled_exception();
            }
            setDone();
            return 1;
        }
    };

    return CoroFrame::ramp(a, b, c);
}

#endif // OPTIONAL_MONAD_MAYBE_EXAMPLE_H
