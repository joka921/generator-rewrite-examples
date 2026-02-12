#ifndef GENERATOR_REWRITE_EXAMPLES_CORO_STORAGE_H
#define GENERATOR_REWRITE_EXAMPLES_CORO_STORAGE_H

#include <sanitizer/asan_interface.h>
// Implementation of a local variable storage inside a coroutine frame.
// Is templated on the reference type of the local variable, and the
// information on whether the storage is owning the variable, or just storing a pointer.
// This allows to implement e.g. lifetime extension for rvalue and const lvalue references.

// NOTE/Caveat: The actual TYPE of the object is currently not stored, so a `const int` and `const int& = <someTemporary>`
// cannot be told apart e.g. for `decltype` expressions.
template<typename Ref, bool isOwningV>
struct coro_storage {
    static constexpr bool isOwning = isOwningV;

    // Buffer for either an object of type `decay_t<Ref>` or for a pointer to such an object,
    // depending on whether the storage is owning or not.
    using Storage = std::conditional_t<isOwning, std::decay_t<Ref>, std::add_pointer_t<std::decay_t<Ref> > >;
    alignas(Storage) char buffer[sizeof(Storage)];


    // Proxy type to obtain a reference to the stored value. The correct way to obtain a reference on a `coro_storage c` is to call
    // `c.get().ref_`. The proxy type gives us the correct behavior for `rvalue` references, because without the proxy object,
    // they would behave like being wrapped in `std::move`.
    struct Proxy {
        Ref ref_;
    };
    Proxy get() {
        Storage &storage = *std::launder(reinterpret_cast<Storage *>(buffer));
        if constexpr (isOwning) {
            return Proxy{static_cast<Ref>(storage)};
        } else {
            return Proxy{static_cast<Ref>(*storage)};
        }
    }

    // Destroy the stored object.
    // Note: We deliberately don't provide a `construct` method, but the using code should
    // use `placement new` directly on the buffer to not break copy elision.
    void destroy() {
        std::launder(reinterpret_cast<Storage *>(buffer))->~Storage();
    }
};

namespace detail {
    // Required to make `if constexpr` work below.
    template <typename M, typename T>
    auto* dependent_addressof(T& t) { return &t;}
}

// Engage the buffer of a `coro_storage` object. The variadic args in all cases have to be
// either an expression in parenthesis, or in braces. If the `storage` doesn't own the object,
// then a single parenthesized lvalue-reference is expected, of which the address is stored in
// the buffer.
// Examples:
// `int a = 4; coro_storage<int&, false> storage; CO_STORAGE_CONSTRUCT(storage, (a))` stores a pointer.
// `coro_storage<std::vector<int>&&, true> storage; CO_STORAGE_CONSTRUCT(storage, {3, 4, c})` uses brace initialization
//  into the buffer.
#define CO_STORAGE_CONSTRUCT(storage, ...) \
[&](auto& storageArg) -> decltype(auto) { \
  using M = std::decay_t<decltype(storageArg)>; \
  if constexpr (M::isOwning) { \
    new(storageArg.buffer) typename M::Storage __VA_ARGS__; \
    ASAN_UNPOISON_MEMORY_REGION(storageArg.buffer, sizeof(typename M::Storage)); \
   } else { \
     new(storageArg.buffer) typename M::Storage{ detail::dependent_addressof<M>(__VA_ARGS__)} ;\
    ASAN_UNPOISON_MEMORY_REGION(storageArg.buffer, sizeof(typename M::Storage)); \
  } \
}(storage)


#endif //GENERATOR_REWRITE_EXAMPLES_CORO_STORAGE_H