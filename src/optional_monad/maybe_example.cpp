// Optional monad example demonstrating chaining, cross-TU usage, and exception handling

#include "maybe_example.h"
#include <iostream>
#include <stdexcept>
#include <cassert>

// Cross-TU example: safe_divide implementation
std::optional<int> safe_divide(int numerator, int denominator)
{
    if (denominator == 0)
    {
        return std::nullopt;
    }
    return numerator / denominator;
}

std::optional<int> chained_calculation_no_coro(int a, int b, int c, int reps)
{
    int result = 0;
    for (size_t i = 0; i < reps; ++i)
    {
        auto r1 = safe_divide(a, b);
        if (!r1) return std::nullopt;

        auto r2 = safe_sqrt(c);
        if (!r2) return std::nullopt;

        result += *r1 + *r2;
    }
    return result;
}

std::optional<int> with_exceptions_no_coro(int x)
{
    try
    {
        if (x < 0)
        {
            throw std::invalid_argument("Input must be non-negative");
        }
        return x * 2;
    }
    catch (...)
    {
        return std::nullopt;
    }
}

// Chaining example: chains multiple maybe operations
// Demonstrates short-circuiting when any operation returns nullopt
std::optional<int> chained_calculation(int a, int b, int c, int reps)
{
    using promise_type = maybe_promise<int>;
    struct CoroFrame : stackful_coro_crtp<CoroFrame, promise_type, false>
    {
        using CoroFrameBase = stackless_coro_crtp<CoroFrame, promise_type, false>;
        const int a_;
        const int b_;
        const int c_;
        const int reps_;

        int result1_;
        int result2_;
        int result_;

        // Storage for awaited values and awaiters
        coro_storage<maybe_awaiter<int>&, true> awaiter_storage_;

        // Track construction state
        struct
        {
            bool awaiter_storage_ = false;
        } __constructed;

        CoroFrame(int a, int b, int c, int reps) : a_(a), b_(b), c_(c), reps_(reps)
        {
        }

        static stackless_coroutine_handle<void> doStepImpl(void* seflPtr)
        {
            auto* self = static_cast<CoroFrame*>(seflPtr);
            // As we never resume from a suspended state,
            // we can completely get rid of the switch-goto block.


            // Resume initial suspend
            CO_GET(initial_awaiter_).await_resume();
            self->initial_awaiter_.destroy();

            self->result_ = 0;
            for (size_t i = 0; i < self->reps_; ++i)
            {
                CO_AWAIT(1, awaiter_storage_, safe_divide(self->a_, self->b_), self->result1_=);

                // int r2 = co_await safe_sqrt(c_);
                CO_AWAIT(2, awaiter_storage_, safe_sqrt(self->c_), self->result2_=);
                self->result_ += self->result1_ + self->result2_;
            }
            // co_return r1 + r2;
            CO_RETURN_VALUE(3, final_awaiter_, self->result_);
        }

        void doStep()
        {
            [[maybe_unused]] auto h = doStepImpl(this);
            assert(!h);
        }

        void destroySuspendedCoro(size_t suspendIdx_)
        {
            switch (suspendIdx_)
            {
            case 0:
                this->initial_awaiter_.destroy();
                return;
            case 3:
                return;
            }
        }

        size_t dispatchExceptionHandling(std::exception_ptr eptr)
        {
            try
            {
                std::rethrow_exception(eptr);
            }
            catch (...)
            {
                // Other exceptions: store for later rethrow
                promise().unhandled_exception();
            }
            setDone();
            return 1;
        }
    };

    return CoroFrame::ramp(a, b, c, reps);
}

// Exception handling example
std::optional<int> with_exceptions(int x)
{
    using promise_type = maybe_promise<int>;
    struct CoroFrame : stackless_coro_crtp<CoroFrame, promise_type, false>
    {
        using CoroFrameBase = stackless_coro_crtp<CoroFrame, promise_type, false>;
        int x_;

        CoroFrame(int x) : x_(x)
        {
        }

        static stackless_coroutine_handle<void> doStepImpl(void* selfPtr)
        {
            auto* self = static_cast<CoroFrame*>(selfPtr);
            switch (self->suspendIdx_)
            {
            case 0: break;
            }

            // Resume initial suspend
            CO_GET(initial_awaiter_).await_resume();
            self->initial_awaiter_.destroy();

            // Throw an exception if x is negative
            if (self->x_ < 0)
            {
                throw std::invalid_argument("Input must be non-negative");
            }

            // Otherwise return x * 2
            CO_RETURN_VALUE(1, final_awaiter_, self->x_ * 2);
        }


        void destroySuspendedCoro(size_t suspendIdx_)
        {
            switch (suspendIdx_)
            {
            case 0:
                this->initial_awaiter_.destroy();
                return;
            case 1:
                return;
            }
        }

        size_t dispatchExceptionHandling(std::exception_ptr eptr)
        {
            try
            {
                std::rethrow_exception(eptr);
            }
            catch (...)
            {
                // Other exceptions: store for later rethrow
                promise().unhandled_exception();
            }
            setDone();
            return 1;
        }
    };

    return CoroFrame::ramp(x);
}
