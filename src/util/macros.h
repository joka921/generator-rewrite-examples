#ifndef GENERATOR_REWRITE_EXAMPLES_MACROS_H
#define GENERATOR_REWRITE_EXAMPLES_MACROS_H

// helper macros that are used inside the `stackless_coro_crtp` and coroutine frames that inherit from `stackless_coro_crtp` (see coroutine_frame.h).

// On the given awaiter, first call `await_ready`, and if the result is false call `await_suspend(handle)`. If the coroutine is not suspended
// (because either await_ready returned true, or await_suspend returned false), nothing further happens.
// If the coroutine has to be suspended then two cases might happen:
// 1. The variadic args are empty, then a `stackless_coroutine_handle<void>` is returned, which is either a null-handle
//    (for void or bool results of `suspend_always`, or a handle for symmetric transfer.
// 2. The variadic args are not empty, then those are returned, and handles for symmetric transfer are resumed before
//     the return statement. This second case allows us to use this macro in the ramp function, where the initial awaiter
//     is awaited, but the `get_return_object` has to be returned..
#define CO_AWAIT_IMPL_IMPL(awaiterInput, handle, ...) \
{ \
  auto& awaiter = awaiterInput; \
  if (!awaiter.await_ready()) { \
    using type = decltype(awaiter.await_suspend(handle)); \
    if constexpr (std::is_void_v<type>) { \
    awaiter.await_suspend(handle); \
    return (stackless_coroutine_handle<void>{} __VA_OPT__(, std::move(__VA_ARGS__))); \
  } else if constexpr (std::is_same_v<type, bool>) { \
    if (! [&](auto& awaiter) {if constexpr (std::is_same_v<type, bool>) return awaiter.await_suspend(handle); return true;}(awaiter)) { \
      return (stackless_coroutine_handle<void>{} __VA_OPT__(, std::move(__VA_ARGS__))); \
    } \
} else { \
/* Symmetric transfer: await_suspend returns a handle to resume next. */ \
/* When no VA_ARGS (inside doStepImpl), return the handle for the trampoline. */ \
/* When VA_ARGS present (inside ramp), call .resume() on it and return the ramp result. */ \
   auto nextHandle = [&](auto& aw) { if constexpr (!std::is_void_v<type> && !std::is_same_v<type, bool>) { return aw.await_suspend(handle); } else {return stackless_coroutine_handle<void>{};} }(awaiter); \
   __VA_OPT__(nextHandle.resume();) \
   return (stackless_coroutine_handle<void>{nextHandle.ptr} __VA_OPT__(, std::move(__VA_ARGS__))); \
} \
} \
} void()
#define CO_AWAIT_IMPL(awaiterMem) CO_AWAIT_IMPL_IMPL(CO_GET(awaiterMem), self->getHandle())

// The second half of a `co_await`. Create a label that corresponds to the `index` ,
// call `await_resume()`  on the awaiter, and destroy it. The variadic args
// are used to implement patterns like ` auto x = co_await something` ,
// (then ` auto x =`  will be the variadic args.
#define CO_RESUME(index, awaiterMem, ...) \
  label_##index:                                                       \
  __VA_ARGS__ CO_GET(awaiterMem).await_resume();                                   \
  self->awaiterMem.destroy();

// Co yield the given expression/value creating a suspension point with the given index.
// the variadic args are used to possibly do something with the result of the `co_yield`  expression.
#define CO_YIELD(index, awaiterMem, value, ...)                            \
    CO_INIT(awaiterMem, (self->promise().yield_value(value)));           \
    self->suspendIdx_ = index;                                            \
    CO_AWAIT_IMPL(awaiterMem); \
    CO_RESUME(index, awaiterMem, __VA_ARGS__); \
    void()

// Same as `CO_YIELD`, but for `co_await` expressions.
#define CO_AWAIT(index, awaiterMem, expr, ...)                                  \
    CO_INIT(awaiterMem, (coro_detail::get_awaiter(self->promise(), expr)));      \
    self->suspendIdx_ = index;                                                      \
    CO_AWAIT_IMPL(awaiterMem);                                                \
    CO_RESUME(index, awaiterMem, __VA_ARGS__); \
    void()

// Implementation of co_return. Does the first half of an
// `await`  on the `finalAwaiter`, and if that doesn't suspend,
// then immediately destroy the frame.
#define CO_RETURN_IMPL_IMPL(finalAwaiterMem, ...)                                  \
self->setDone();                                                          \
CO_STORAGE_CONSTRUCT(self->finalAwaiterMem, (self->promise().final_suspend()));                  \
CO_AWAIT_IMPL_IMPL(self->finalAwaiterMem.get().ref_, self->getHandle(), __VA_ARGS__);                                              \
self->destroy();                      \
void()

// Destroy all the local variables that are still active at the given index
// of the co_return statement, and then co_await the final suspend.
#define CO_RETURN_IMPL(index, finalAwaiterMem)                                  \
  self->destroySuspendedCoro(index);                                           \
  CO_RETURN_IMPL_IMPL(finalAwaiterMem);                                 \
  return {};                                                                   \
  void()

// Implement `co_return <void>`. Creates a suspension point (to which we will never resume,
// but which is used to properly destroy the data members of the frame.
#define CO_RETURN_VOID(index, finalAwaiterMem)                                  \
    if constexpr (coro_detail::has_return_void<std::decay_t<decltype(self->promise())>>::value) { \
    self->promise().return_void();                                                     \
    }                                                                             \
    CO_RETURN_IMPL(index, finalAwaiterMem);                                     \
    void()

// Implement `co_return <non-void-expr>`. Creates a suspension point (to which we will never resume,
// but which is used to properly destroy the data members of the frame.
#define CO_RETURN_VALUE(index, finalAwaiterMem, value)                           \
    self->promise().return_value(value);                                               \
    CO_RETURN_IMPL(index, finalAwaiterMem);                                       \
    void()


// TODO document those helper macros for the exception handling.
#define DESTROY_UNCONDITIONALLY(mem) self->mem.destroy(); self->__constructed.mem=false; void()
#define DESTROY_IF_CONSTRUCTED(mem) if (self->__constructed.mem) {DESTROY_UNCONDITIONALLY(mem);} void()

#define TRY_BEGIN(try_index) self->currentTryBlock_ = (try_index); void()
#define TRY_END(parent_index, label) self->currentTryBlock_ = (parent_index); label_##label: void()


#define FOR_LOOP_HEADER(N)


#define CO_GET(arg) self->arg.get().ref_

#define CO_INIT(mem, ...) \
  CO_STORAGE_CONSTRUCT(self->mem, __VA_ARGS__);



#endif //GENERATOR_REWRITE_EXAMPLES_MACROS_H