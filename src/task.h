#ifndef GENERATOR_REWRITE_EXAMPLES_TASK_H
#define GENERATOR_REWRITE_EXAMPLES_TASK_H

#include <optional>
#include <exception>
#include "./coroutine_handle.h"
#include "./suspend.h"

// Forward declaration of task (at global scope).
template<typename T, template <typename...> typename TaskHandle>
class task;

namespace detail {

// Symmetric transfer back to caller on task completion.
// await_suspend returns the continuation handle (or noop if none), and the
// macro calls .resume() on it.
template<template <typename...> typename TaskHandle>
struct task_final_awaiter {
    static constexpr bool await_ready() noexcept { return false; }

    template<typename Promise>
    TaskHandle<void> await_suspend(TaskHandle<Promise> h) noexcept {
        if (h.promise().continuation_) {
            return h.promise().continuation_;
        }
        return noop_coroutine();
    }

    static constexpr void await_resume() noexcept {}
};

// Value-returning promise.
template<typename T, template <typename...> typename TaskHandle>
class task_promise {
public:
    using handle_type = TaskHandle<task_promise>;

    task_promise() = default;

    ::task<T, TaskHandle> get_return_object() noexcept;

    constexpr SuspendAlways initial_suspend() const noexcept { return {}; }

    task_final_awaiter<TaskHandle> final_suspend() const noexcept { return {}; }

    void return_value(T value) {
        value_ = std::move(value);
    }

    void unhandled_exception() {
        exception_ = std::current_exception();
    }

    T& result() & {
        if (exception_) {
            std::rethrow_exception(exception_);
        }
        return *value_;
    }

    T&& result() && {
        if (exception_) {
            std::rethrow_exception(exception_);
        }
        return std::move(*value_);
    }

    TaskHandle<void> continuation_;

private:
    std::optional<T> value_;
    std::exception_ptr exception_;
};

// Void partial specialization.
template<template <typename...> typename TaskHandle>
class task_promise<void, TaskHandle> {
public:
    using handle_type = TaskHandle<task_promise>;

    task_promise() = default;

    ::task<void, TaskHandle> get_return_object() noexcept;

    constexpr SuspendAlways initial_suspend() const noexcept { return {}; }

    task_final_awaiter<TaskHandle> final_suspend() const noexcept { return {}; }

    void return_void() noexcept {}

    void unhandled_exception() {
        exception_ = std::current_exception();
    }

    void result() {
        if (exception_) {
            std::rethrow_exception(exception_);
        }
    }

    TaskHandle<void> continuation_;

private:
    std::exception_ptr exception_;
};

// Awaiter returned by task::get_awaiter() — the C++17 equivalent of operator co_await.
template<typename T, template <typename...> typename TaskHandle>
class task_awaiter {
    using promise_type = task_promise<T, TaskHandle>;
    using handle_type = TaskHandle<promise_type>;

public:
    explicit task_awaiter(handle_type coro) noexcept : coro_(coro) {}

    bool await_ready() const noexcept { return coro_.done(); }

    // Symmetric transfer: stores caller as continuation, returns inner task's handle.
    // The macro calls .resume() on the returned handle, starting the inner task.
    template<typename CallerPromise>
    handle_type await_suspend(TaskHandle<CallerPromise> caller) noexcept {
        coro_.promise().continuation_ = caller;
        return coro_;
    }

    decltype(auto) await_resume() {
        return coro_.promise().result();
    }

private:
    handle_type coro_;
};

} // namespace detail

template<typename T, template <typename...> typename TaskHandle>
class [[nodiscard]] task {
public:
    using promise_type = detail::task_promise<T, TaskHandle>;
    using handle_type = TaskHandle<promise_type>;

    task() noexcept : coro_(nullptr) {}

    explicit task(handle_type h) noexcept : coro_(h) {}

    task(task&& other) noexcept : coro_(other.coro_) {
        other.coro_ = nullptr;
    }

    task(const task&) = delete;
    task& operator=(const task&) = delete;

    task& operator=(task&& other) noexcept {
        if (this != &other) {
            if (coro_) {
                coro_.destroy();
            }
            coro_ = other.coro_;
            other.coro_ = nullptr;
        }
        return *this;
    }

    ~task() {
        if (coro_) {
            coro_.destroy();
        }
    }

    // Resume from initial_suspend — for top-level use.
    void start() {
        coro_.resume();
    }

    bool done() const {
        return coro_.done();
    }

    decltype(auto) result() {
        return coro_.promise().result();
    }

    // C++17 equivalent of operator co_await.
    friend  auto get_awaiter(const task& t) {
        return detail::task_awaiter<T, TaskHandle>{t.coro_};
    }

private:
    handle_type coro_;
};

namespace detail {
    template<typename T, template <typename...> typename H>
    ::task<T, H> task_promise<T, H>::get_return_object() noexcept {
        return ::task<T, H>{handle_type::from_promise(*this)};
    }

    template<template <typename...> typename H>
    ::task<void, H> task_promise<void, H>::get_return_object() noexcept {
        return ::task<void, H>{handle_type::from_promise(*this)};
    }
} // namespace detail

#endif //GENERATOR_REWRITE_EXAMPLES_TASK_H
