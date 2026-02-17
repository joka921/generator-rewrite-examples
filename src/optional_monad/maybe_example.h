// Optional monad example header
// Demonstrates cross-TU usage and inline coroutine patterns

#ifndef OPTIONAL_MONAD_MAYBE_EXAMPLE_H
#define OPTIONAL_MONAD_MAYBE_EXAMPLE_H

#include "maybe.h"
#include "../src/coroutine_frame.h"
#include "../src/inline_coroutine_frame.h"
#include <cmath>
#include <cassert>

// Forward declaration - implemented in maybe_example.cpp (cross-TU example)
std::optional<int> safe_divide(int numerator, int denominator);
std::optional<int> chained_calculation(int a, int b, int c);

// Inline implementation - defined in header
inline std::optional<int> safe_sqrt(int x) {
    if (x < 0)
    {
        return std::nullopt;
    }
    return static_cast<int>(std::sqrt(x));
}

#endif // OPTIONAL_MONAD_MAYBE_EXAMPLE_H
