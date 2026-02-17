
#ifndef GENERATOR_REWRITE_EXAMPLES_COROUTINE_HANDLE_H
#define GENERATOR_REWRITE_EXAMPLES_COROUTINE_HANDLE_H

#include <cstddef>

// Forward declarations needed for HandleFrame's resumeFunc return type.
template<typename Promise> struct stackless_coroutine_handle;
template<> struct stackless_coroutine_handle<void>;

// A type-erased "base" class for a coroutine frame, consists of two function pointers to implement
// the member functions of the `stackless_coroutine_handle` below.
// resumeFunc returns stackless_coroutine_handle<void> to support symmetric transfer trampolining:
// a default-constructed stackless_coroutine_handle<void> (ptr=nullptr) means "no more symmetric transfers".
struct HandleFrame {
    stackless_coroutine_handle<void>(*resumeFunc)(void *);
    void(*destroyFunc)(void *);
};

// Common base class for `stackless_coroutine_handle<void>`  and stackless_coroutine_handle<some_promise_type>` below.
namespace detail
{
    struct stackless_handle_base {
        HandleFrame *ptr;

        stackless_handle_base() noexcept : ptr(nullptr) {}
        stackless_handle_base(HandleFrame *ptr) noexcept : ptr(ptr) {}

        stackless_handle_base& operator=(const stackless_handle_base&) = default;
        stackless_handle_base(const stackless_handle_base&) = default;
        stackless_handle_base(stackless_handle_base&&) = default;
        stackless_handle_base& operator=(stackless_handle_base&&) = default;
        stackless_handle_base& operator=(std::nullptr_t) noexcept { ptr = nullptr; return *this; }

        // Trampoline: keep resuming returned handles until we get a null handle
        // (no more symmetric transfers). Defined out-of-line after stackless_coroutine_handle<void> is complete.
        void resume();
        void destroy() { ptr->destroyFunc(reinterpret_cast<void*>(ptr)); }
        operator bool() const { return static_cast<bool>(ptr); }
        bool done() const { return ptr->resumeFunc == nullptr; }
    };
}

// A replacement for `std::coroutine_handle`. Consists of a single pointer to the `HandleFrame` from above, to which it
// delegates all the member functions.
template<typename Promise>
struct stackless_coroutine_handle : public detail::stackless_handle_base {
    using stackless_handle_base::stackless_handle_base;
    using stackless_handle_base::operator=;

    //  Only for non-void `Promise` types: convert from promise to handle and back.
    //  Assumes that in our final coroutine frame the promise object will be declared directly after the
    // `HandleFrame` above.
    static constexpr size_t promise_offset =
            (sizeof(HandleFrame) + alignof(Promise) - 1) & ~(alignof(Promise) - 1);

    static stackless_coroutine_handle from_promise(Promise &p) {
        auto *ptr = reinterpret_cast<HandleFrame *>(reinterpret_cast<char *>(&p) -
                                                    promise_offset);
        return stackless_coroutine_handle{ptr};
    }

    const Promise &promise() const {
        return *reinterpret_cast<const Promise *>(reinterpret_cast<const char *>(ptr) +
                                                  promise_offset);
    }

    Promise &promise() {
        return const_cast<Promise&>(const_cast<const stackless_coroutine_handle&>(*this).promise());
    }
};

// stackless_coroutine_handle<void> specialization — type-erased handle without promise access.
template<>
struct stackless_coroutine_handle<void> : public detail::stackless_handle_base {
    using stackless_handle_base::stackless_handle_base;
    using stackless_handle_base::operator=;
};

// Now stackless_coroutine_handle<void> is complete — define the trampoline.
inline void detail::stackless_handle_base::resume() {
    stackless_coroutine_handle<void> next = ptr->resumeFunc(reinterpret_cast<void*>(ptr));
    while (next.ptr) {
        next = next.ptr->resumeFunc(reinterpret_cast<void*>(next.ptr));
    }
}

// Equivalent of std::noop_coroutine() — returns a stackless_coroutine_handle<void> that does nothing on resume/destroy.
namespace detail {
    inline stackless_coroutine_handle<void> noop_resume(void*) { return {}; }
    inline void noop_destroy(void*) {}
}
inline stackless_coroutine_handle<void> noop_coroutine() {
    static HandleFrame noop_frame{
        &detail::noop_resume,
        &detail::noop_destroy
    };
    return stackless_coroutine_handle<void>{&noop_frame};
}

// A stackful coroutine handle that is templated on the frame of a certain coroutine and stores this frame by value.
// Note: It is also possible to template this class on a reference-type, then only a (still strongly typed) reference to
// the frame will be stored.
template<typename CoroFrame>
struct stackful_coroutine_handle {
    CoroFrame frame;
    static constexpr bool isReference = std::is_reference_v<CoroFrame>;

    template <bool b = true, std::enable_if_t<b && !isReference, int> = 0>
    explicit stackful_coroutine_handle(CoroFrame&& f) noexcept : frame(std::move(f)) {}

    template <bool b = true, std::enable_if_t<b && isReference, int> = 0>
    explicit stackful_coroutine_handle(CoroFrame f) noexcept : frame(f) {}

    stackful_coroutine_handle(stackful_coroutine_handle&& other) = default;

    stackful_coroutine_handle& operator=(stackful_coroutine_handle&& other) noexcept  = default;

    // Note: The destructor does not call ` frame.destroy()` , this is the responsibility
    // of the coroutine type (e.g. a generator) that owns the handle (same as for ordinary stackless coroutines).
    ~stackful_coroutine_handle() = default;

    stackful_coroutine_handle(const stackful_coroutine_handle&) = delete;
    stackful_coroutine_handle& operator=(const stackful_coroutine_handle&) = delete;

    // Direct call, no function pointer indirection.
    void resume() { frame.doStep(); }
    void destroy() { frame.destroy(); }
    bool done() const { return frame.done(); }
    explicit constexpr operator bool() const { return true; }
    auto& promise() { return frame.promise(); }
    const auto& promise() const { return frame.promise_; }
};


#endif //GENERATOR_REWRITE_EXAMPLES_COROUTINE_HANDLE_H