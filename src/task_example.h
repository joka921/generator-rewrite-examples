#ifndef GENERATOR_REWRITE_EXAMPLES_TASK_EXAMPLE_H
#define GENERATOR_REWRITE_EXAMPLES_TASK_EXAMPLE_H

#include "./coroutine_frame.h"
#include "./task.h"
#include "./macros.h"

/**
 * Manually lowered equivalent of:
 *   task<int, Handle> compute_value(int x) { co_return x * 2; }
 */
task<int, Handle> compute_value(int x)
{
    using promise_type = task<int, Handle>::promise_type;
    struct CoroFrame : CoroImpl<CoroFrame, promise_type, true>
    {
        using CoroFrameBase = CoroImpl<CoroFrame, promise_type, true>;
        int x_;

        CoroFrame(int x) : x_(x) {}

        void doStepImpl()
        {
            switch (this->curState)
            {
            case 0: break;
            }

            CO_GET(initial_awaiter_).await_resume();
            initial_awaiter_.destroy();

            // co_return x_ * 2;
            CO_RETURN_VALUE(1, final_awaiter_, (x_ * 2));
        }

        void destroySuspendedCoro(size_t curState)
        {
            switch (curState)
            {
            case 0:
                this->initial_awaiter_.destroy();
                return;
            case 1:
                return;
            }
        }
    };
    return CoroFrame::ramp(x);
}

/**
 * Manually lowered equivalent of:
 *   task<int, Handle> add_values(int a, int b) {
 *       int va = co_await compute_value(a);
 *       int vb = co_await compute_value(b);
 *       co_return va + vb;
 *   }
 */
task<int, Handle> add_values(int a, int b)
{
    using promise_type = task<int, Handle>::promise_type;
    struct CoroFrame : CoroImpl<CoroFrame, promise_type, true>
    {
        using CoroFrameBase = CoroImpl<CoroFrame, promise_type, true>;
        int a_;
        int b_;
        int va_;
        int vb_;

        // Storage for the inner task and its awaiter.
        coro_storage<task<int, Handle>&, true> task_storage_;
        coro_storage<detail::task_awaiter<int, Handle>&, true> awaiter_storage_;

        CoroFrame(int a, int b) : a_(a), b_(b) {}

        void doStepImpl()
        {
            switch (this->curState)
            {
            case 0: break;
            case 1: goto label_1;
            case 2: goto label_2;
            }

            CO_GET(initial_awaiter_).await_resume();
            initial_awaiter_.destroy();

            // int va = co_await compute_value(a_);
            CO_STORAGE_CONSTRUCT(task_storage_, (compute_value(a_)));
            CO_STORAGE_CONSTRUCT(awaiter_storage_, (CO_GET(task_storage_).get_awaiter()));
            this->curState = 1;
            CO_AWAIT_IMPL(awaiter_storage_);
            label_1:
            va_ = CO_GET(awaiter_storage_).await_resume();
            awaiter_storage_.destroy();
            task_storage_.destroy();

            // int vb = co_await compute_value(b_);
            CO_STORAGE_CONSTRUCT(task_storage_, (compute_value(b_)));
            CO_STORAGE_CONSTRUCT(awaiter_storage_, (CO_GET(task_storage_).get_awaiter()));
            this->curState = 2;
            CO_AWAIT_IMPL(awaiter_storage_);
            label_2:
            vb_ = CO_GET(awaiter_storage_).await_resume();
            awaiter_storage_.destroy();
            task_storage_.destroy();

            // co_return va_ + vb_;
            CO_RETURN_VALUE(3, final_awaiter_, (va_ + vb_));
        }

        void destroySuspendedCoro(size_t curState)
        {
            switch (curState)
            {
            case 0:
                this->initial_awaiter_.destroy();
                return;
            case 1:
                awaiter_storage_.destroy();
                task_storage_.destroy();
                return;
            case 2:
                awaiter_storage_.destroy();
                task_storage_.destroy();
                return;
            case 3:
                return;
            }
        }
    };
    return CoroFrame::ramp(a, b);
}

#endif //GENERATOR_REWRITE_EXAMPLES_TASK_EXAMPLE_H
