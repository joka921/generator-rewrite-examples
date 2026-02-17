// Adapted from toby-allsopp's coroutine_monad library
// Original source: https://github.com/toby-allsopp/coroutine_monad
// Copyright (c) 2017 Toby Allsopp
// MIT License (see LICENSE.txt)

#ifndef OPTIONAL_MONAD_MAYBE_H
#define OPTIONAL_MONAD_MAYBE_H

#include "util/coroutine_frame.h"
#include "util/inline_coroutine_frame.h"
#include "util/suspend.h"
#include "util/coro_storage.h"
#include <optional>
#include "optional_monad/return_object_holder.h"
#include <exception>

// Forward declaration
template<typename T>
class maybe_promise;

// Awaiter for std::optional<T> that implements short-circuit semantics.
// Always "ready" (doesn't suspend), but throws if nullopt to trigger short-circuit.
template<typename T>
class maybe_awaiter {
    std::optional<T> opt_;

public:
    explicit maybe_awaiter(std::optional<T> opt) noexcept
        : opt_(std::move(opt)) {}

    // Always ready (no suspension needed)
    constexpr bool await_ready() const noexcept {
        return opt_.has_value();
    }

    // Never called since await_ready() always returns true
    template<typename Handle>
    void await_suspend(Handle h) noexcept
    {
        h.promise().data_->emplace(std::nullopt);
        h.destroy();
    }

    // Extract the value, or throw to trigger short-circuit
    T await_resume() {
        return std::move(*opt_);
    }
};

// Promise type for maybe monad
template<typename T>
struct maybe_promise {
    static constexpr bool return_object_is_stackless = true;
    return_object_holder<std::optional<T>>* data_;
public:
    maybe_promise() = default;

    // Return type is std::optional<T> directly (no wrapper needed)
    auto get_return_object() noexcept {
        return make_return_object_holder(data_);
    }

    constexpr SuspendNever initial_suspend() const noexcept {
        return {};
    }

    constexpr SuspendNever final_suspend() const noexcept {
        return {};
    }

    // Called when coroutine returns a value
    void return_value(T value) {
        data_->emplace(std::move(value));
    }
    void return_value(std::nullopt_t) {
        data_->emplace(std::nullopt);
    }


    // Convert exceptions to nullopt
    void unhandled_exception() {
        data_->emplace(std::nullopt);
    }
};

// ADL function for getting awaiter from std::optional<T>
// This is the C++17 equivalent of operator co_await
template<typename T>
auto get_awaiter(std::optional<T> opt) {
    return maybe_awaiter<T>{std::move(opt)};
}

#endif // OPTIONAL_MONAD_MAYBE_H
