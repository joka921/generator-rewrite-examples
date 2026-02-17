#ifndef GENERATOR_REWRITE_EXAMPLES_THROWING_PARSE_INTS_H
#define GENERATOR_REWRITE_EXAMPLES_THROWING_PARSE_INTS_H

#include <string>
#include <exception>
#include "util/coroutine_frame.h"
#include "generator/unified_generator.h"
#include "util/macros.h"

/**
 * The C++17 rewrite of the following generator:
 *   generator<int, std::coroutine_handle> throwing_parse_ints(
 *       RangeOfStrings&& strings, bool catch_errors) {
 *     for (const auto& s : strings) {
 *       try {
 *         co_yield std::stoi(s);
 *       } catch (...) {
 *         if (!catch_errors) throw;  // escape to caller
 *         co_yield -1;              // fallback
 *       }
 *     }
 *   }
 *
 * This is the first example that uses isNoexcept=false and demonstrates
 * try-catch inside a generator with exception dispatch.
 */
template <typename RangeOfStrings>
heap_generator<int> throwing_parse_ints(RangeOfStrings&& strings, bool catch_errors)
{
    using promise_type = heap_generator<int>::promise_type;
    struct CoroFrame : CoroImpl<CoroFrame, promise_type, false>
    {
        using CoroFrameBase = CoroImpl<CoroFrame, promise_type, false>;
        RangeOfStrings&& strings_;
        bool catch_errors_;

        using It = decltype(std::begin(strings_));
        using End = decltype(std::end(strings_));
        coro_storage<It&, std::is_object_v<It>> it_;
        coro_storage<const End&, std::is_object_v<End>> end_;
        coro_storage<int&&, true> parsed_;
        coro_storage<int&&, true> fallback_;
        std::exception_ptr caught_exception_;

        CoroFrame(RangeOfStrings&& s, bool ce)
            : strings_(std::forward<RangeOfStrings>(s)), catch_errors_(ce)
        {
        }

        // States:
        //   0  = initial_suspend
        //   1  = co_yield inside try (after stoi)
        //   2  = co_yield inside catch (the -1 fallback)
        //   3  = co_return
        //   10 = catch handler entry
        static Handle<void> doStepImpl(void* selfPtr)
        {
            auto* self = static_cast<CoroFrame*>(selfPtr);
            switch (self->curState)
            {
            case 0: break;
            case 1: goto label_1;
            case 2: goto label_2;
            case 10: goto label_catch_0;
            }

            CO_GET(initial_awaiter_).await_resume();
            self->initial_awaiter_.destroy();
            CO_INIT(it_, (self->strings_.begin()));
            CO_INIT(end_, (self->strings_.end()));
            for (; CO_GET(it_) != CO_GET(end_); ++CO_GET(it_))
            {
                // try {
                TRY_BEGIN(0);
                CO_INIT(parsed_, (std::stoi(*CO_GET(it_))));
                CO_YIELD(1, initial_awaiter_, CO_GET(parsed_));
                self->parsed_.destroy();
                TRY_END(CoroFrameBase::CO_NO_TRY_BLOCK, try_end_0);
                continue;
                // } catch (...) {
                label_catch_0:
                if (!self->catch_errors_) {
                    std::rethrow_exception(self->caught_exception_);
                }
                CO_INIT(fallback_, (-1));
                CO_YIELD(2, initial_awaiter_, CO_GET(fallback_));
                self->fallback_.destroy();
                // }
            }
            CO_RETURN_VOID(3, final_awaiter_);
        }

        size_t dispatchExceptionHandling(std::exception_ptr eptr)
        {
            switch (this->currentTryBlock_)
            {
            case CoroFrameBase::CO_NO_TRY_BLOCK:
                // Unhandled exception: store in promise, clean up locals, enter final suspend.
                this->promise().unhandled_exception();
                it_.destroy();
                end_.destroy();
                this->setDone();
                CO_STORAGE_CONSTRUCT(this->final_awaiter_, (this->promise().final_suspend()));
                return CoroFrameBase::CO_NO_TRY_BLOCK;
            case 0:
                // Exception caught by try block 0: store eptr, redirect to catch handler.
                caught_exception_ = std::move(eptr);
                this->curState = 10;
                this->currentTryBlock_ = CoroFrameBase::CO_NO_TRY_BLOCK;
                return 10;
            default:
                __builtin_unreachable();
            }
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
                parsed_.destroy();
                it_.destroy();
                end_.destroy();
                return;
            case 2:
                this->initial_awaiter_.destroy();
                fallback_.destroy();
                it_.destroy();
                end_.destroy();
                return;
            case 3:
                return;
            }
        }
    };
    return CoroFrame::ramp(std::forward<RangeOfStrings>(strings), catch_errors);
}

#endif //GENERATOR_REWRITE_EXAMPLES_THROWING_PARSE_INTS_H
