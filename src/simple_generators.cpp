#include <iostream>
#include <ostream>

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
generator<int, Handle> iota(int start, int end)
{
    using promise_type = generator<int, Handle>::promise_type;
    struct CoroFrame : CoroImpl<CoroFrame, promise_type, true>
    {
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
            initial_awaiter_.destroy();
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
                // Nothing to destroy at the `co_return` point, as there are no local variables in scope at that point.
                return;
            }
        }
    };
    return CoroFrame::ramp(start, end);
}

int main()
{
    for (auto i : iota(3000, 3042))
    {
        std::cout << i << std::endl;
    }
}
