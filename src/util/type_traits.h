#ifndef GENERATOR_REWRITE_EXAMPLES_TYPE_TRAITS_H
#define GENERATOR_REWRITE_EXAMPLES_TYPE_TRAITS_H

// Helper type traits used to analyze promise types and awaitables.
namespace coro_detail {
// A class that is implicitly convertible to anything, but the conversion operator is not defined.
// Useful for type traits.
struct ConvertibleToAnything {
  template <typename T>
  operator T();
};

// `has_await_transform<Promise>::value` is true iff. `Promise` has a member function
// `await_transform` that can be called with a single argument.
template <typename Promise, typename = void>
struct has_await_transform : std::false_type {};

template <typename Promise>
struct has_await_transform<Promise, std::void_t<decltype(std::declval<Promise&>().await_transform(
                                        std::declval<ConvertibleToAnything>()))>> : std::true_type {
};

// Type traits that checks whether a function `get_awaiter(awaitable)` can be found via ADL.
// This is our proxy for `operator co_await` which is not available in C++17.
template <typename Awaitable, typename = void>
struct has_get_awaiter : std::false_type {};

template <typename Awaitable>
struct has_get_awaiter<Awaitable, std::void_t<decltype(get_awaiter(std::declval<Awaitable>()))>>
    : std::true_type {};

// Given a `promise` and a `co_await`ed expression, get the actual `awaiter`, possibly through the
// `await_transform` and `get_awaiter` mechanisms.
template <typename Promise, typename Expr>
decltype(auto) get_awaiter(Promise& promise, Expr&& expr) {
  decltype(auto) awaitable = [&]() -> decltype(auto) {
    if constexpr (has_await_transform<Promise>::value) {
      return promise.await_transform(std::forward<Expr>(expr));
    } else {
      return std::forward<Expr>(expr);
    }
  }();
  if constexpr (has_get_awaiter<decltype(awaitable)>::value) {
    return get_awaiter(std::forward<decltype(awaitable)>(awaitable));
  } else {
    return std::forward<decltype(awaitable)>(awaitable);
  }
}

// Type trait to check whether a given `Promise` has a `return_void` member function.
template <typename Promise, typename = void>
struct has_return_void : std::false_type {};

template <typename Promise>
struct has_return_void<Promise, std::void_t<decltype(std::declval<Promise&>().return_void())>>
    : std::true_type {};

// Type trait that checks whether a given promise type has an overloaded `operator new`.
// Note: This only supports the simple form that only takes a `size_t`.
template <typename P, typename = void>
struct has_promise_new : std::false_type {};

template <typename P>
struct has_promise_new<P, std::void_t<decltype(P::operator new(size_t{}))>> : std::true_type {};

// Type trait that checks whether a given promise type has an overloaded `operator new`
// that accepts the coroutine arguments in addition to the size.
// Per the C++ spec, `promise_type::operator new(size, args...)` is tried first.
template <typename P, typename ArgsTuple, typename = void>
struct has_promise_new_with_args_impl : std::false_type {};

template <typename P, typename... Args>
struct has_promise_new_with_args_impl<
    P, std::tuple<Args...>,
    std::void_t<decltype(P::operator new(size_t{}, std::declval<Args>()...))>> : std::true_type {};

template <typename P, typename... Args>
using has_promise_new_with_args = has_promise_new_with_args_impl<P, std::tuple<Args...>>;

// Type traits that checks whether the promise_type `P` has a member `operator delete`.
template <typename P, typename = void>
struct has_promise_delete : std::false_type {};

template <typename P>
struct has_promise_delete<
    P, std::void_t<decltype(P::operator delete(std::declval<void*>(), size_t{}))>>
    : std::true_type {};

// Type trait for unsized promise delete: `P::operator delete(void*)`.
template <typename P, typename = void>
struct has_promise_delete_unsized : std::false_type {};

template <typename P>
struct has_promise_delete_unsized<P,
                                  std::void_t<decltype(P::operator delete(std::declval<void*>()))>>
    : std::true_type {};

// Allocate the coroutine frame using the promise type's operator new if available.
// Per the C++ spec, the fallback chain is:
//   1. P::operator new(size, args...) — with coroutine arguments
//   2. P::operator new(size)          — without coroutine arguments
//   3. ::operator new(size)           — global
template <typename P, typename... Args>
void* promise_allocate(size_t size, Args&&... args) {
  if constexpr (sizeof...(Args) > 0 && has_promise_new_with_args<P, Args...>::value) {
    return P::operator new(size, std::forward<Args>(args)...);
  } else if constexpr (has_promise_new<P>::value) {
    return P::operator new(size);
  } else {
    return ::operator new(size);
  }
}
}  // namespace coro_detail
#endif  // GENERATOR_REWRITE_EXAMPLES_TYPE_TRAITS_H