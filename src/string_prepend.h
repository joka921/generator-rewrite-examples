#ifndef GENERATOR_REWRITE_EXAMPLES_STRING_PREPEND_H
#define GENERATOR_REWRITE_EXAMPLES_STRING_PREPEND_H

#include "./coroutine_frame.h"
#include "./unified_generator.h"
#include "macros.h"

/**
 * The C++17 rewrite of the following generator:
 *   generator<std::string, std::coroutine_handle> prepend(auto&& rangeOfStrings, std::string prefix) {
 *     for (const auto& s : rangeOfStrings) {
 *       co_yield(prefix + s);
 *     }
 *   }
 *
 */
template <typename RangeOfStrings>
heap_generator<std::string> string_prepend(RangeOfStrings&& rangeOfStrings, std::string prefix)
{
    using promise_type = heap_generator<std::string>::promise_type;
    struct CoroFrame : CoroImpl<CoroFrame, promise_type, true>
    {
        using CoroFrameBase = CoroImpl<CoroFrame, promise_type, true>;
        RangeOfStrings&& rangeOfStrings_;
        std::string prefix_;

        // We will have local variables for `it` and `end`, as well as for a temporary `std::string` object during the
        // `co_yield` exression.
        using It = decltype(std::begin(rangeOfStrings_));
        using End = decltype(std::end(rangeOfStrings_));
        coro_storage<It&, std::is_object_v<It>> it_;
        coro_storage<const End&, std::is_object_v<End>> end_;
        coro_storage<std::string&&, true> tmp_;

        CoroFrame(RangeOfStrings&& ros, std::string pref) : rangeOfStrings_(std::forward<RangeOfStrings>(ros)), prefix_(std::move(pref))
        {
        }

        static Handle<void> doStepImpl(void* selfPtr)
        {
            auto* self = static_cast<CoroFrame*>(selfPtr);
            switch (self->curState)
            {
            case 0: break;
            case 1: goto label_1;
            }

            CO_GET(initial_awaiter_).await_resume();
            self->initial_awaiter_.destroy();
            CO_INIT(it_, (self->rangeOfStrings_.begin()));
            CO_INIT(end_, (self->rangeOfStrings_.end()));
            for (; CO_GET(it_) != CO_GET(end_); ++CO_GET(it_))
            {
                CO_INIT(tmp_, (self->prefix_ + *CO_GET(it_)));
                CO_YIELD(1, initial_awaiter_, CO_GET(tmp_));
                self->tmp_.destroy();
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
                tmp_.destroy();
            case 2:
                // Nothing to destroy at the `co_return` point, as there are no local variables in scope at that point.
                return;
            }
        }
    };
    return CoroFrame::ramp(std::forward<RangeOfStrings>(rangeOfStrings), std::move(prefix));
}

#endif //GENERATOR_REWRITE_EXAMPLES_STRING_PREPEND_H