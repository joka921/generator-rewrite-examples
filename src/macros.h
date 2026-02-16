#ifndef GENERATOR_REWRITE_EXAMPLES_MACROS_H
#define GENERATOR_REWRITE_EXAMPLES_MACROS_H

// helper macros that are used inside the `CoroImpl` and coroutine frames that inherit from `CoroImpl` (see coroutine_frame.h).
// When called without VA_ARGS (from doStepImpl), returns Handle<void>:
//   - Handle<void>{} (nullptr) for void/bool await_suspend (= just suspend, no symmetric transfer)
//   - Handle<void>{nextHandle.ptr} for handle await_suspend (= symmetric transfer target)
// When called with VA_ARGS (from ramp), returns the VA_ARGS value via comma operator.
// The comma-operator trick `return (Handle<void>{} __VA_OPT__(, __VA_ARGS__))` yields:
//   - `return (Handle<void>{})` when VA_ARGS is empty (returns Handle<void>)
//   - `return (Handle<void>{}, ret)` when VA_ARGS is present (comma operator returns ret)
#define CO_AWAIT_IMPL_IMPL(awaiterInput, handle, ...) \
{ \
  auto& awaiter = awaiterInput; \
  if (!awaiter.await_ready()) { \
    using type = decltype(awaiter.await_suspend(handle)); \
    if constexpr (std::is_void_v<type>) { \
    awaiter.await_suspend(handle); \
    return (Handle<void>{} __VA_OPT__(, std::move(__VA_ARGS__))); \
  } else if constexpr (std::is_same_v<type, bool>) { \
    if (! [&](auto& awaiter) {if constexpr (std::is_same_v<type, bool>) return awaiter.await_suspend(handle); return true;}(awaiter)) { \
      return (Handle<void>{} __VA_OPT__(, std::move(__VA_ARGS__))); \
    } \
} else { \
/* Symmetric transfer: await_suspend returns a handle to resume next. */ \
/* When no VA_ARGS (inside doStepImpl), return the handle for the trampoline. */ \
/* When VA_ARGS present (inside ramp), call .resume() on it and return the ramp result. */ \
   auto nextHandle = [&](auto& aw) { if constexpr (!std::is_void_v<type> && !std::is_same_v<type, bool>) { return aw.await_suspend(handle); } }(awaiter); \
   __VA_OPT__(nextHandle.resume();) \
   return (Handle<void>{nextHandle.ptr} __VA_OPT__(, std::move(__VA_ARGS__))); \
} \
} \
} void()
#define CO_AWAIT_IMPL(awaiterMem) CO_AWAIT_IMPL_IMPL(CO_GET(awaiterMem), CoroFrameBase::Hdl::from_promise(self->promise()))

#define CO_RESUME(index, awaiterMem, ...) \
  label_##index:                                                       \
  __VA_ARGS__ CO_GET(awaiterMem).await_resume();                                   \
  self->awaiterMem.destroy();

#define CO_YIELD(index, awaiterMem, value, ...)                            \
    CO_INIT(awaiterMem, (self->promise().yield_value(value)));           \
    self->curState = index;                                            \
    CO_AWAIT_IMPL(awaiterMem); \
    CO_RESUME(index, awaiterMem, __VA_ARGS__); \
    void()

#define CO_AWAIT(index, awaiterMem, expr, ...)                                  \
    CO_INIT(awaiterMem, (coro_detail::get_awaiter(self->promise(), expr)));      \
    self->curState = index;                                                      \
    CO_AWAIT_IMPL(awaiterMem);                                                \
    CO_RESUME(index, awaiterMem, __VA_ARGS__); \
    void()

#define CO_RETURN_IMPL_IMPL(finalAwaiterMem, ...)                                  \
self->setDone();                                                          \
CO_STORAGE_CONSTRUCT(self->finalAwaiterMem, (self->promise().final_suspend()));                  \
CO_AWAIT_IMPL_IMPL(self->finalAwaiterMem.get().ref_, CoroFrameBase::Hdl::from_promise(self->promise()), __VA_ARGS__);                                              \
self->destroy();                      \
void()

#define CO_RETURN_IMPL(index, finalAwaiterMem)                                  \
  self->destroySuspendedCoro(index);                                           \
  CO_RETURN_IMPL_IMPL(finalAwaiterMem);                                 \
  return {};                                                                   \
  void()

#define CO_RETURN_VOID(index, finalAwaiterMem)                                  \
    if constexpr (coro_detail::has_return_void<std::decay_t<decltype(self->promise())>>::value) { \
    self->promise().return_void();                                                     \
    }                                                                             \
    CO_RETURN_IMPL(index, finalAwaiterMem);                                     \
    void()

#define DESTROY_UNCONDITIONALLY(mem) self->mem.destroy(); self->__constructed.mem=false; void()
#define DESTROY_IF_CONSTRUCTED(mem) if (self->__constructed.mem) {DESTROY_UNCONDITIONALLY(mem);} void()

#define CO_RETURN_FALLOFF(index, finalAwaiterMem) CO_RETURN_VOID(index, finalAwaiterMem)

#define CO_RETURN_VALUE(index, finalAwaiterMem, value)                           \
    self->promise().return_value(value);                                               \
    CO_RETURN_IMPL(index, finalAwaiterMem);                                       \
    void()

#define TRY_BEGIN(try_index) self->currentTryBlock_ = (try_index); void()
#define TRY_END(parent_index, label) self->currentTryBlock_ = (parent_index); label_##label: void()


#define FOR_LOOP_HEADER(N)


#define CO_GET(arg) self->arg.get().ref_
#define CO_GET_STATE(arg) arg.get().ref_

// TODO<joka921> Update the other OWNING also to the new lambda syntax.
#define CO_INIT_REF(mem, ...)  \
    new(self->mem.buffer) typename decltype(self->mem)::Storage{ &__VA_ARGS__} ;\
    self->__constructed.mem=true

#define CO_BRACED_INIT(mem, ...) CO_INIT_REF(mem, __VA_ARGS__)
#define CO_PAREN_INIT(mem, ...) CO_INIT_REF(mem, __VA_ARGS__)

#define CO_INIT(mem, ...) \
  CO_STORAGE_CONSTRUCT(self->mem, __VA_ARGS__);



#endif //GENERATOR_REWRITE_EXAMPLES_MACROS_H