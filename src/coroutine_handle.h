
#ifndef GENERATOR_REWRITE_EXAMPLES_COROUTINE_HANDLE_H
#define GENERATOR_REWRITE_EXAMPLES_COROUTINE_HANDLE_H

#include <cstddef>

// A type-erased "base" class for a coroutine frame, consists of two function pointers to implement
// the member functions of the `CoroutineHandle` replacement below.
// This is exactly the layout GCC uses internally.
struct HandleFrame {
    using F = void(void *);
    using B = bool(void *);
    F *resumeFunc;
    F *destroyFunc;
};

// A replacement for `std::coroutine_handle`. Consists of a single pointer to the `HandleFrame` from above, to which it
// delegates all the member functions.
template<typename Promise = void>
struct Handle {
    HandleFrame *ptr;

    // Construction and assignment.
    Handle(HandleFrame *ptr) noexcept : ptr(ptr) {}
    Handle& operator=(const Handle&) = default;
    Handle(const Handle&) = default;
    Handle(Handle&&) = default;
    Handle& operator=(Handle&&) = default;
    Handle& operator=(std::nullptr_t) noexcept { ptr = nullptr; return *this;}

    // resume, done, destroy, and operator bool just use the indirection to the `HandleFrame`.
    void resume() { ptr->resumeFunc(reinterpret_cast<void*>(ptr)); }
    void destroy() { ptr->destroyFunc(reinterpret_cast<void*>(ptr)); }
    operator bool() const { return static_cast<bool>(ptr); }
    bool done() const { return ptr->resumeFunc == nullptr; }

    //  Only for non-void `Promise` types: convert from promise to handle and back.
    //  Assumes that in our final coroutine frame the promise object will be declared directly after the
    // `HandleFrame` above.
    static constexpr size_t promise_offset =
            (sizeof(HandleFrame) + alignof(Promise) - 1) & ~(alignof(Promise) - 1);

    static Handle from_promise(Promise &p) {
        auto *ptr = reinterpret_cast<HandleFrame *>(reinterpret_cast<char *>(&p) -
                                                    promise_offset);
        return Handle{ptr};
    }

    const Promise &promise() const {
        return *reinterpret_cast<const Promise *>(reinterpret_cast<const char *>(ptr) +
                                                  promise_offset);
    }

    Promise &promise() {
        return const_cast<Promise&>(const_cast<const Handle&>(*this).promise());
    }
};

// Handle<void> specialization — type-erased handle without promise access.
template<>
struct Handle<void> {
    HandleFrame *ptr;

    Handle() noexcept : ptr(nullptr) {}
    Handle(HandleFrame *ptr) noexcept : ptr(ptr) {}

    // Converting constructor: enables implicit Handle<P> -> Handle<void>.
    template<typename P>
    Handle(Handle<P> other) noexcept : ptr(other.ptr) {}

    Handle& operator=(const Handle&) = default;
    Handle(const Handle&) = default;
    Handle(Handle&&) = default;
    Handle& operator=(Handle&&) = default;
    Handle& operator=(std::nullptr_t) noexcept { ptr = nullptr; return *this; }

    void resume() { ptr->resumeFunc(reinterpret_cast<void*>(ptr)); }
    void destroy() { ptr->destroyFunc(reinterpret_cast<void*>(ptr)); }
    operator bool() const { return static_cast<bool>(ptr); }
    bool done() const { return ptr->resumeFunc == nullptr; }
};

// Equivalent of std::noop_coroutine() — returns a Handle<void> that does nothing on resume/destroy.
inline Handle<void> noop_coroutine() {
    static HandleFrame noop_frame{
        [](void*) {},
        [](void*) {}
    };
    return Handle<void>{&noop_frame};
}

#endif //GENERATOR_REWRITE_EXAMPLES_COROUTINE_HANDLE_H