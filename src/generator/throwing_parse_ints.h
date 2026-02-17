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
    struct CoroFrame : stackless_coro_crtp<CoroFrame, promise_type, false>
    {
        using CoroFrameBase = stackless_coro_crtp<CoroFrame, promise_type, false>;
        RangeOfStrings&& strings_;
        bool catch_errors_;

        using It = decltype(std::begin(strings_));
        using End = decltype(std::end(strings_));
        coro_storage<It&, std::is_object_v<It>> it_;
        coro_storage<const End&, std::is_object_v<End>> end_;
        coro_storage<int&, true> parsed_;


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
        static stackless_coroutine_handle<void> doStepImpl(void* selfPtr)
        {
            auto* self = static_cast<CoroFrame*>(selfPtr);
            switch (self->suspendIdx_)
            {
            case 0: break;
            case 1: goto label_1;
            case 10: goto label_catch_0;
            case 11: goto label_catch_1;
            case 12: goto label_try_end_0;
            }

            CO_GET(initial_awaiter_).await_resume();
            self->initial_awaiter_.destroy();
            CO_INIT(it_, (self->strings_.begin()));
            CO_INIT(end_, (self->strings_.end()));
            for (; CO_GET(it_) != CO_GET(end_); ++CO_GET(it_))
            {
                // try {
                CO_INIT(parsed_, (-1));
                TRY_BEGIN(0);
                CO_GET(parsed_) = std::stoi(*CO_GET(it_));
                goto label_try_end_0;
                // } catch (...) {
                label_catch_0:
                    {
                        if (!self->catch_errors_)
                        {
                            throw;
                        }
                        self->suspendIdx_ = 12;
                        return {};
                    }
                label_catch_1:
                    {
                        if (!self->catch_errors_)
                        {
                            throw;
                        }
                        CO_GET(parsed_) = -2;
                        self->suspendIdx_ = 12;
                        return {};
                    }

                // End of catch clause. We deliberately `return`, because we
                // have to end the scope of the exception stack.
                TRY_END(CoroFrameBase::CO_NO_TRY_BLOCK, try_end_0);
                std::cout << " parsedll" << CO_GET(parsed_) << std::endl;
                CO_YIELD(1, initial_awaiter_, CO_GET(parsed_));
                // }
            }
            self->parsed_.destroy();
            CO_RETURN_VOID(3, final_awaiter_);
        }

        stackless_coroutine_handle<void> dispatchExceptionHandling()
        {
            switch (this->currentTryBlock_)
            {
            case CoroFrameBase::CO_NO_TRY_BLOCK:
                // Unhandled exception: store in promise, clean up locals, enter final suspend.
                it_.destroy();
                end_.destroy();
                this->setDone();
                this->promise().unhandled_exception();
                {
                    auto* self = this;
                CO_RETURN_IMPL_IMPL(final_awaiter_);
                }
            case 0:
                handleCatchClause_0();
            default:
                __builtin_unreachable();
            }
        }

        stackless_coroutine_handle<void> handleCatchClause_0()
        {
            this->currentTryBlock_ = CoroFrameBase::CO_NO_TRY_BLOCK;
            try {
            try
            {
               throw;
            } catch (const std::invalid_argument&)
            {
                this->suspendIdx_ = 10;
            } catch (const std::out_of_range&)
            {
                this->suspendIdx_ = 11;
            }
                auto handled = this->doStepImpl(this);
                return this->doStepImpl(this);
            } catch (...)
            {
                return dispatchExceptionHandling();
            }
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
                it_.destroy();
                end_.destroy();
                return;
            case 2:
                this->initial_awaiter_.destroy();
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
