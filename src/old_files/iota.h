#ifndef GENERATOR_REWRITE_EXAMPLES_IOTA_H
#define GENERATOR_REWRITE_EXAMPLES_IOTA_H

#include "./coroutine_frame.h"
#include "./generator.h"
#include "macros.h"

/**
 * A simple `iota` generator, direct replacement of
 *   generator<int, std::coroutine_handle> iota(int start, int end) {
 *     while (start < end) {
 *       co_yield start;
 *       ++start;
 *     }
 *   }
 *
 * @param start
 * @param end
 * @return
 */
generator<int, stackless_coroutine_handle> iota(int start, int end)
{
    using promise_type = generator<int, stackless_coroutine_handle>::promise_type;
    struct CoroFrame : stackless_coro_crtp<CoroFrame, promise_type, true>
    {
        using CoroFrameBase = stackless_coro_crtp<CoroFrame, promise_type, true>;
        int start_;
        int end_;

        CoroFrame(int s, int e) : start_(s), end_(e)
        {
        }

        void doStepImpl()
        {
            switch (this->suspendIdx_)
            {
            case 0: break;
            case 1: goto label_1;
            }

            CO_GET(initial_awaiter_).await_resume();
            initial_awaiter_.destroy();
            while (start_ < end_)
            {
                CO_YIELD(1, initial_awaiter_, (start_));
                ++start_;
            }
            CO_RETURN_VOID(2, final_awaiter_);
        }

        void destroySuspendedCoro(size_t suspendIdx_)
        {
            switch (suspendIdx_)
            {
            case 0:
                this->initial_awaiter_.destroy();
                return;
            case 1:
                this->initial_awaiter_.destroy();
            case 2:
                // Nothing to destroy at the `co_return` point, as there are no local variables in scope at that point.
                return;
            }
        }
    };
    return CoroFrame::ramp(start, end);
}
#endif //GENERATOR_REWRITE_EXAMPLES_IOTA_H