#ifndef GENERATOR_REWRITE_EXAMPLES_MACROS_H
#define GENERATOR_REWRITE_EXAMPLES_MACROS_H

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
/* TODO encourage tail call optimization for empty `__VA_ARGS__`*/ \
  throw std::runtime_error("not yet implemented"); \
  /*awaiter.await_suspend(handle).resume(); */ \
  return __VA_ARGS__; \
} \
} \
} void()
#define CO_AWAIT_IMPL(awaiterMem) CO_AWAIT_IMPL_IMPL(CO_GET(awaiterMem), Hdl::from_promise(pt))

#define CO_RESUME(index, awaiterMem) \
  label_##index:                                                       \
  CO_GET(awaiterMem).await_resume();                                   \
  this->awaiterMem.destroy();

#define CO_YIELD(index, awaiterMem, value)                            \
    CO_STORAGE_CONSTRUCT(this->awaiterMem, (promise().yield_value(value)));           \
    this->curState = index;                                            \
    CO_AWAIT_IMPL(awaiterMem); \
    CO_RESUME(index, awaiterMem);

#define CO_AWAIT_VOID(index, awaiterMem, expr)                                  \
{                                                                              \
this->awaiterMem.construct(coro_detail::get_awaitable(promise(), expr));      \
this->curState = index;                                                      \
    CO_AWAIT_IMPL(awaiterMem);                                                \
    CO_RESUME(index, awaiterMem);

#define CO_RETURN_IMPL_IMPL(finalAwaiterMem, ...)                                  \
this->setDone();                                                          \
CO_STORAGE_CONSTRUCT(this->finalAwaiterMem, (promise().final_suspend()));                  \
CO_AWAIT_IMPL_IMPL(this->finalAwaiterMem.get().ref_, Hdl::from_promise(pt), __VA_ARGS__);                                              \
this->destroy(&this->frm);                      \
void()

#define CO_RETURN_IMPL(index, finalAwaiterMem)                                  \
  this->destroySuspendedCoro(index);                                           \
  CO_RETURN_IMPL_IMPL(finalAwaiterMem);                                 \
  return;                                                                      \
  void()

#define CO_RETURN_VOID(index, finalAwaiterMem)                                  \
    if constexpr (coro_detail::has_return_void<std::decay_t<decltype(promise())>>::value) { \
    promise().return_void();                                                     \
    }                                                                             \
    CO_RETURN_IMPL(index, finalAwaiterMem);                                     \
    void()

#define DESTROY_UNCONDITIONALLY(mem) this->mem.destroy(); this->__constructed.mem=false; void()
#define DESTROY_IF_CONSTRUCTED(mem) if (this->__constructed.mem) {DESTROY_UNCONDITIONALLY(mem);} void()

#define CO_RETURN_FALLOFF(index, finalAwaiterMem) CO_RETURN_VOID(index, finalAwaiterMem)

#define CO_RETURN_VALUE(index, finalAwaiterMem, value)                           \
    promise().return_value(value);                                               \
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
[&](auto& member) -> decltype(auto) { \
using M = std::decay_t<decltype(member)>; \
if constexpr (M::isOwning) { \
new(member.buffer) typename M::Storage __VA_ARGS__; \
} else { \
new(member.buffer) typename M::Storage{ coro_detail::dependent_addressof<M>(__VA_ARGS__)} ;\
} \
this->__constructed.mem=true; \
return std::move(CO_GET(mem)); \
}(this->mem)



#endif //GENERATOR_REWRITE_EXAMPLES_MACROS_H