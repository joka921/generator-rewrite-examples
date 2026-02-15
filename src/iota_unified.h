#ifndef GENERATOR_REWRITE_EXAMPLES_IOTA_UNIFIED_H
#define GENERATOR_REWRITE_EXAMPLES_IOTA_UNIFIED_H

#include "./coroutine_frame.h"
#include "./inline_coroutine_frame.h"
#include "./unified_generator.h"
#include "macros.h"

template <bool isStackless, typename Frame, typename promise, bool isNoexcept>
using FrameCRTP = std::conditional_t<isStackless, CoroImpl<Frame, promise, isNoexcept>, InlineCoroImpl<Frame, promise, isNoexcept>>;
/**
 * A simple `iota` generator using the unified heap_generator alias.
 * Direct replacement of:
 *   generator<int, Handle> iota(int start, int end) { ... }
 */
template <bool stackless = true>
auto iota_unified(int start, int end)
{
    using promise_type = std::conditional_t<stackless, heap_generator<int>::promise_type, detail::unified_generator_promise<int>>;
    struct CoroFrame : FrameCRTP<stackless, CoroFrame, promise_type, true>
    {
        using CoroFrameBase = FrameCRTP<stackless, CoroFrame, promise_type, true>;
        int start_;
        int end_;

        CoroFrame(int s, int e) : start_(s), end_(e)
        {
        }

        void doStepImpl()
        {
            switch (this->curState)
            {
            case 0: break;
            case 1: goto label_1;
            }

            CO_GET(initial_awaiter_).await_resume();
            this->initial_awaiter_.destroy();
            while (start_ < end_)
            {
                CO_YIELD(1, initial_awaiter_, (start_));
                ++start_;
            }
            CO_RETURN_VOID(2, final_awaiter_);
        }

        void destroySuspendedCoro(size_t curState)
        {
            switch (curState)
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
    if constexpr( stackless)
    {
        return CoroFrame::ramp(start, end);
    } else
    {
        return inline_gen<int, CoroFrame>{
            InlineHandle<CoroFrame>{CoroFrame::ramp(start, end)}};
    }
}
#endif //GENERATOR_REWRITE_EXAMPLES_IOTA_UNIFIED_H
