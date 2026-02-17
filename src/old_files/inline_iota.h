#ifndef GENERATOR_REWRITE_EXAMPLES_INLINE_IOTA_H
#define GENERATOR_REWRITE_EXAMPLES_INLINE_IOTA_H

#include "./inline_coroutine_frame.h"
#include "./inline_generator.h"
#include "macros.h"

/**
 * An inline (stack-allocated) version of the `iota` generator.
 * The coroutine body is identical to iota.h, only the base class,
 * promise type, and return type differ.
 */
auto inline_iota(int start, int end)
{
    using promise_type = detail::inline_generator_promise<int>;
    struct CoroFrame : InlineCoroImpl<CoroFrame, promise_type, true>
    {
        using CoroFrameBase = InlineCoroImpl<CoroFrame, promise_type, true>;
        int start_;
        int end_;

        CoroFrame(int s, int e) : start_(s), end_(e)
        {
        }

        void doStepImpl() noexcept
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
                return;
            }
        }
    };
    return inline_generator<int, CoroFrame>{CoroFrame::ramp(start, end)};
}
#endif //GENERATOR_REWRITE_EXAMPLES_INLINE_IOTA_H
