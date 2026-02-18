#ifndef GENERATOR_REWRITE_EXAMPLES_THROWING_PARSE_INTS_H
#define GENERATOR_REWRITE_EXAMPLES_THROWING_PARSE_INTS_H

#include <exception>
#include <string>

#include "generator/unified_generator.h"
#include "util/coroutine_frame.h"
#include "util/macros.h"

/**
 * The C++17 rewrite of the following generator:
 *   generator<int, std::coroutine_handle> throwing_parse_ints(
 *       RangeOfStrings&& strings, bool catch_errors) {
 *     for (const auto& s : strings) {
 *       try {
 *         co_yield std::stoi(s);
 *       } catch (const std::invalid_argument&) {
 *         if (!catch_errors) throw;  // escape to caller
 *         continue;
 *       } catch (const std::out_of_range&) {
 *       if (!catch_errors) throw;
 *         break;
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

        // For each of the local variables in our functions we have to keep track of whether it was
        // already constructed or not, because we might encounter an exception at any point and have
        // to correctly destroy them.
        struct
        {
            bool initial_awaiter_ = true;
            bool it_;
            bool end_;
            bool parsed_;
        } __constructed;

        CoroFrame(RangeOfStrings&& s, bool catch_errors)
            : strings_(std::forward<RangeOfStrings>(s)), catch_errors_(catch_errors)
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
            // initial suspension point;
            case 0:
                break;
            // co_yield label.
            case 1:
                goto label_1;
            case 12:
                goto try_end;
            // The end of the loop body, and the position after the loop body each get a label. That way we can
            // implement `continue` and `break` inside the catch handlers, which are defined out of line.
            case 20:
                goto for_continue;
            case 21:
                goto for_break;
            }

            CO_GET(initial_awaiter_).await_resume();
            DESTROY_UNCONDITIONALLY(initial_awaiter_);

            // Manually (roughly) rewrite range-for-loop.
            CO_INIT_EX(it_, (self->strings_.begin()));
            CO_INIT_EX(end_, (self->strings_.end()));
            for (; CO_GET(it_) != CO_GET(end_); ++CO_GET(it_))
            {
                // try {
                self->currentTryBlock_ = 0;
                CO_INIT_EX(parsed_, (std::stoi(*CO_GET(it_))));
                CO_YIELD(1, initial_awaiter_, CO_GET(parsed_));
                DESTROY_UNCONDITIONALLY(parsed_);
                // } catch { (the catch handling is defined below,
                // but we are no longer inside try-block 0, but in its parent, which is no try-block at all.
                // if we pass this line, then we can just pop from the try-block stack.
                self->currentTryBlock_ = CoroFrameBase::CO_NO_TRY_BLOCK;
            try_end:
                // }
            for_continue:
                ;
            }
        for_break:
            DESTROY_UNCONDITIONALLY(end_);
            DESTROY_UNCONDITIONALLY(it_);
            CO_RETURN_VOID(2, final_awaiter_);
        }

        // This function is expected by the CRTP base class. It is called from inside a generic catch(...)
        // clause, so we have access to the current exception.
        stackless_coroutine_handle<void> dispatchExceptionHandling()
        {
            switch (this->currentTryBlock_)
            {
            case CoroFrameBase::CO_NO_TRY_BLOCK:
                {
                    // An exception has occured outside any explicit try-catch block,
                    // but we don't have any further information. We thus have to possibly destroy all
                    // members except for `parsed_` (which is defined inside the try-block, and
                    // therefore in all cases already has been destroyed.
                    auto* self = this;
                    DESTROY_IF_CONSTRUCTED(end_);
                    DESTROY_IF_CONSTRUCTED(it_);
                    DESTROY_IF_CONSTRUCTED(initial_awaiter_);
                    return this->await_final_suspend();
                }
            case 0:
                return handleCatchClause_0();
            default:
                __builtin_unreachable();
            }
        }

        // Implementation for the catch clauses of the first (and only) try-catch block.
        stackless_coroutine_handle<void> handleCatchClause_0()
        {
            // No more parent try-catch block from here on.
            this->currentTryBlock_ = CoroFrameBase::CO_NO_TRY_BLOCK;
            try
            {
                auto* self = this;
                DESTROY_IF_CONSTRUCTED(parsed_);
                // If none of the catch clauses contains a `break/continue/co_return`, then
                // we resume after the try-catch block.
                this->suspendIdx_ = 12;
                try
                {
                    // Rethrow the exception and use the original catch clauses with `continue`
                    // and `break` rewritten.
                    throw;
                }
                catch (const std::invalid_argument&)
                {
                    if (!catch_errors_)
                    {
                        throw;
                    }
                    this->suspendIdx_ = 20;
                }
                catch (const std::out_of_range&)
                {
                    if (!catch_errors_)
                    {
                        throw;
                    }
                    this->suspendIdx_ = 21;
                }
                // If we reach here, then we have caught the exception and correctly
                // set up the `suspendIdx_`, so we can return to the outer `doStep` function which will then
                // resume the coroutine from outside any catch handlers.
                return {};
            }
            catch (...)
            {
                // None of the catch-clauses applied, or they rethrew, propagate to the next level of exception handling.
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
                parsed_.destroy();
                it_.destroy();
                end_.destroy();
                return;
            case 2:
                return;
            }
        }
    };
    return CoroFrame::ramp(std::forward<RangeOfStrings>(strings), catch_errors);
}

#endif  // GENERATOR_REWRITE_EXAMPLES_THROWING_PARSE_INTS_H
