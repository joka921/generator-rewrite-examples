// Optional monad example demonstrating chaining, cross-TU usage, and exception handling

#include "maybe_example.h"
#include <iostream>
#include <stdexcept>
#include <cassert>

// Cross-TU example: safe_divide implementation
std::optional<int> safe_divide(int numerator, int denominator) {
    if (denominator == 0) {
    return std::nullopt;}
    return numerator / denominator;
}

std::optional<int> chained_calculation_no_coro(int a, int b, int c)
{

}

// Chaining example: chains multiple maybe operations
// Demonstrates short-circuiting when any operation returns nullopt
std::optional<int> chained_calculation(int a, int b, int c) {
    using promise_type = maybe_promise<int>;
    struct CoroFrame : InlineCoroImpl<CoroFrame, promise_type, false> {
        using CoroFrameBase = CoroImpl<CoroFrame, promise_type, false>;
        int a_, b_, c_;
        int result1_;
        int result2_;

        // Storage for awaited values and awaiters
        coro_storage<std::optional<int>&, true> maybe_storage_;
        coro_storage<maybe_awaiter<int>&, true> awaiter_storage_;

        // Track construction state
        struct {
            bool maybe_storage_ = false;
            bool awaiter_storage_ = false;
        } __constructed;

        CoroFrame(int a, int b, int c) : a_(a), b_(b), c_(c) {}

        static Handle<void> doStepImpl(void* seflPtr) {
            auto* self = static_cast<CoroFrame*>(seflPtr);
            // As we never resume from a suspended state,
            // we can completely get rid of the switch-goto block.

            // Resume initial suspend
            CO_GET(initial_awaiter_).await_resume();
            self->initial_awaiter_.destroy();

            // int r1 = co_await safe_divide(a_, b_);
            CO_INIT(maybe_storage_, (safe_divide(self->a_, self->b_)));
            CO_AWAIT(1, awaiter_storage_, CO_GET(maybe_storage_), self->result1_=);
            self->maybe_storage_.destroy();

            // int r2 = co_await safe_sqrt(c_);
            CO_INIT(maybe_storage_, (safe_sqrt(self->c_)));
            CO_AWAIT(2, awaiter_storage_, CO_GET(maybe_storage_), self->result2_=);
            self->maybe_storage_.destroy();

            // co_return r1 + r2;
            CO_RETURN_VALUE(3, final_awaiter_, self->result1_ + self->result2_);
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

// Exception handling example
std::optional<int> with_exceptions(int x) {
    using promise_type = maybe_promise<int>;
    struct CoroFrame : CoroImpl<CoroFrame, promise_type, false> {
        using CoroFrameBase = CoroImpl<CoroFrame, promise_type, false>;
        int x_;

        CoroFrame(int x) : x_(x) {}

        static Handle<void> doStepImpl(void* selfPtr) {
            auto* self = static_cast<CoroFrame*>(selfPtr);
            switch (self->curState) {
            case 0: break;
            }

            // Resume initial suspend
            CO_GET(initial_awaiter_).await_resume();
            self->initial_awaiter_.destroy();

            // Throw an exception if x is negative
            if (self->x_ < 0) {
                throw std::invalid_argument("Input must be non-negative");
            }

            // Otherwise return x * 2
            CO_RETURN_VALUE(1, final_awaiter_, self->x_ * 2);
        }


        void destroySuspendedCoro(size_t curState) {
            switch (curState) {
            case 0:
                this->initial_awaiter_.destroy();
                return;
            case 1:
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

    return CoroFrame::ramp(x);
}

int main() {
    std::cout << "=== Optional Monad Examples ===\n\n";

    // Test 1: Successful chain
    std::cout << "Test 1: chained_calculation(10, 2, 16)\n";
    try {
        auto result1 = chained_calculation(10, 2, 16);
        if (result1) {
            std::cout << "  Result: " << *result1 << " (expected: 9)\n";
        } else {
            std::cout << "  Result: nullopt\n";
        }
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
    std::cout << "\n";

    // Test 2: Short-circuit on divide by zero
    std::cout << "Test 2: chained_calculation(10, 0, 16)\n";
    try {
        auto result2 = chained_calculation(10, 0, 16);
        if (result2) {
            std::cout << "  Result: " << *result2 << "\n";
        } else {
            std::cout << "  Result: nullopt (short-circuited on divide by zero)\n";
        }
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
    std::cout << "\n";

    // Test 3: Short-circuit on negative sqrt
    std::cout << "Test 3: chained_calculation(10, 2, -4)\n";
    try {
        auto result3 = chained_calculation(10, 2, -4);
        if (result3) {
            std::cout << "  Result: " << *result3 << "\n";
        } else {
            std::cout << "  Result: nullopt (short-circuited on negative sqrt)\n";
        }
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
    std::cout << "\n";

    // Test 4: Exception handling
    std::cout << "Test 4: with_exceptions(-1)\n";
    try {
        auto result4 = with_exceptions(-1);
        if (result4) {
            std::cout << "  Result: " << *result4 << "\n";
        } else {
            std::cout << "  Result: nullopt\n";
        }
    } catch (const std::exception& e) {
        std::cout << "  Exception caught: " << e.what() << "\n";
    }
    std::cout << "\n";

    // Test 5: Successful exception test (no exception)
    std::cout << "Test 5: with_exceptions(5)\n";
    try {
        auto result5 = with_exceptions(5);
        if (result5) {
            std::cout << "  Result: " << *result5 << " (expected: 10)\n";
        } else {
            std::cout << "  Result: nullopt\n";
        }
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
    std::cout << "\n";

    // Test 6: Cross-TU usage demonstration
    std::cout << "Test 6: Direct safe_divide(15, 3)\n";
    try {
        auto result6 = safe_divide(15, 3);
        if (result6) {
            std::cout << "  Result: " << *result6 << " (expected: 5)\n";
        } else {
            std::cout << "  Result: nullopt\n";
        }
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }
    std::cout << "\n";

    // Test 7: Cross-TU with inline function
    std::cout << "Test 7: Direct safe_sqrt(25)\n";
    try {
        auto result7 = safe_sqrt(25);
        if (result7) {
            std::cout << "  Result: " << *result7 << " (expected: 5)\n";
        } else {
            std::cout << "  Result: nullopt\n";
        }
    } catch (const std::exception& e) {
        std::cout << "  Exception: " << e.what() << "\n";
    }

    std::cout << "\n=== All tests completed ===\n";
    return 0;
}
