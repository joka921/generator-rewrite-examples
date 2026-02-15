#ifndef GENERATOR_REWRITE_EXAMPLES_MACROS_H
#define GENERATOR_REWRITE_EXAMPLES_MACROS_H

// helper macros that are used inside the `CoroImpl` and coroutine frames that inherit from `CoroImpl` (see coroutine_frame.h).
#define CO_AWAIT_IMPL_IMPL(awaiterInput, handle, ...) \
{ \
  auto& awaiter = awaiterInput; \
  if (!awaiter.await_ready()) { \
    using type = decltype(awaiter.await_suspend(handle)); \
    if constexpr (std::is_void_v<type>) { \
    awaiter.await_suspend(handle); \
    return __VA_ARGS__; \
  } else if constexpr (std::is_same_v<type, bool>) { \
    if (! [&](auto& awaiter) {if constexpr (std::is_same_v<type, bool>) return awaiter.await_suspend(handle); return true;}(awaiter)) { return __VA_ARGS__; } \
} else { \
/* Symmetric transfer: the generic lambda + inner if-constexpr ensures the */ \
/* .resume() call is only instantiated when await_suspend returns a handle. */ \
  [&](auto& aw) { if constexpr (!std::is_void_v<type> && !std::is_same_v<type, bool>) { aw.await_suspend(handle).resume(); } }(awaiter); \
  return __VA_ARGS__; \
} \
} \
} void()
#define CO_AWAIT_IMPL(awaiterMem) CO_AWAIT_IMPL_IMPL(CO_GET(awaiterMem), CoroFrameBase::Hdl::from_promise(this->promise()))

#define CO_RESUME(index, awaiterMem) \
  label_##index:                                                       \
  CO_GET(awaiterMem).await_resume();                                   \
  this->awaiterMem.destroy();

#define CO_YIELD(index, awaiterMem, value)                            \
    CO_STORAGE_CONSTRUCT(this->awaiterMem, (this->promise().yield_value(value)));           \
    this->curState = index;                                            \
    CO_AWAIT_IMPL(awaiterMem); \
    CO_RESUME(index, awaiterMem);

#define CO_AWAIT_VOID(index, awaiterMem, expr)                                  \
{                                                                              \
this->awaiterMem.construct(coro_detail::get_awaitable(this->promise(), expr));      \
this->curState = index;                                                      \
    CO_AWAIT_IMPL(awaiterMem);                                                \
    CO_RESUME(index, awaiterMem);

#define CO_RETURN_IMPL_IMPL(finalAwaiterMem, ...)                                  \
this->setDone();                                                          \
CO_STORAGE_CONSTRUCT(this->finalAwaiterMem, (this->promise().final_suspend()));                  \
CO_AWAIT_IMPL_IMPL(this->finalAwaiterMem.get().ref_, CoroFrameBase::Hdl::from_promise(this->promise()), __VA_ARGS__);                                              \
this->destroy();                      \
void()

#define CO_RETURN_IMPL(index, finalAwaiterMem)                                  \
  this->destroySuspendedCoro(index);                                           \
  CO_RETURN_IMPL_IMPL(finalAwaiterMem);                                 \
  return;                                                                      \
  void()

#define CO_RETURN_VOID(index, finalAwaiterMem)                                  \
    if constexpr (coro_detail::has_return_void<std::decay_t<decltype(this->promise())>>::value) { \
    this->promise().return_void();                                                     \
    }                                                                             \
    CO_RETURN_IMPL(index, finalAwaiterMem);                                     \
    void()

#define DESTROY_UNCONDITIONALLY(mem) this->mem.destroy(); this->__constructed.mem=false; void()
#define DESTROY_IF_CONSTRUCTED(mem) if (this->__constructed.mem) {DESTROY_UNCONDITIONALLY(mem);} void()

#define CO_RETURN_FALLOFF(index, finalAwaiterMem) CO_RETURN_VOID(index, finalAwaiterMem)

#define CO_RETURN_VALUE(index, finalAwaiterMem, value)                           \
    this->promise().return_value(value);                                               \
    CO_RETURN_IMPL(index, finalAwaiterMem);                                       \
    void()

#define TRY_BEGIN(try_index) this->currentTryBlock_ = (try_index); void()
#define TRY_END(parent_index, label) this->currentTryBlock_ = (parent_index); label_##label: void()


#define FOR_LOOP_HEADER(N)


#define CO_GET(arg) this->arg.get().ref_
#define CO_GET_STATE(arg) arg.get().ref_

// TODO<joka921> Update the other OWNING also to the new lambda syntax.
#define CO_INIT_REF(mem, ...)  \
    new(this->mem.buffer) typename decltype(this->mem)::Storage{ &__VA_ARGS__} ;\
    this->__constructed.mem=true

#define CO_BRACED_INIT(mem, ...) CO_INIT_REF(mem, __VA_ARGS__)
#define CO_PAREN_INIT(mem, ...) CO_INIT_REF(mem, __VA_ARGS__)

#define CO_INIT(mem, ...) \
  CO_STORAGE_CONSTRUCT(this->mem, __VA_ARGS__);



#endif //GENERATOR_REWRITE_EXAMPLES_MACROS_H