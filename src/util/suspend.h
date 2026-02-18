#ifndef GENERATOR_REWRITE_EXAMPLES_SUSPEND_H
#define GENERATOR_REWRITE_EXAMPLES_SUSPEND_H

// Drop-in replacement for `std::suspend_always` and `std::suspend_never` from C++20.
// As we will use them with our own `coroutine_handle` types, the `await_suspend` function
// is templated.
struct SuspendAlways {
  static constexpr bool await_ready() noexcept { return false; }

  template <typename Handle>
  static constexpr void await_suspend([[maybe_unused]] Handle handle) noexcept {}

  static constexpr void await_resume() noexcept {}
};

struct SuspendNever {
  static constexpr bool await_ready() noexcept { return true; }

  template <typename Handle>
  static constexpr void await_suspend([[maybe_unused]] Handle handle) noexcept {}

  static constexpr void await_resume() noexcept {}
};

#endif  // GENERATOR_REWRITE_EXAMPLES_SUSPEND_H