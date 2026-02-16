
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

// Common base class for `Handle<void>`  and Handle<some_promise_type>` below.
struct HandleBase {
    HandleFrame *ptr;

    HandleBase() noexcept : ptr(nullptr) {}
    HandleBase(HandleFrame *ptr) noexcept : ptr(ptr) {}

    HandleBase& operator=(const HandleBase&) = default;
    HandleBase(const HandleBase&) = default;
    HandleBase(HandleBase&&) = default;
    HandleBase& operator=(HandleBase&&) = default;
    HandleBase& operator=(std::nullptr_t) noexcept { ptr = nullptr; return *this; }

    void resume() { ptr->resumeFunc(reinterpret_cast<void*>(ptr)); }
    void destroy() { ptr->destroyFunc(reinterpret_cast<void*>(ptr)); }
    operator bool() const { return static_cast<bool>(ptr); }
    bool done() const { return ptr->resumeFunc == nullptr; }

    static void resumeTypeErased(void* ptr)
    {
        auto* handle = reinterpret_cast<HandleBase*>(ptr);
        [[clang::musttail]] return handle->ptr->resumeFunc(reinterpret_cast<void*>(handle->ptr));
    }
};

// A replacement for `std::coroutine_handle`. Consists of a single pointer to the `HandleFrame` from above, to which it
// delegates all the member functions.
template<typename Promise>
struct Handle : public HandleBase {
    using HandleBase::HandleBase;
    using HandleBase::operator=;
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
struct Handle<void> : public HandleBase {
    using HandleBase::HandleBase;
    using HandleBase::operator=;
};

// Equivalent of std::noop_coroutine() — returns a Handle<void> that does nothing on resume/destroy.
inline Handle<void> noop_coroutine() {
    static HandleFrame noop_frame{
        [](void*) {},
        [](void*) {}
    };
    return Handle<void>{&noop_frame};
}

// A fat coroutine handle that is templated on the frame of a certain coroutine and stores this frame by value without any heap allocations.
template<typename CoroFrame>
struct InlineHandle {
    CoroFrame frame;

    explicit InlineHandle(CoroFrame&& f) noexcept : frame(std::move(f)) {}

    // Move constructor — invalidates the source to prevent double-destroy.
    InlineHandle(InlineHandle&& other) = default;

    InlineHandle& operator=(InlineHandle&& other) noexcept  = default;

    // Note: The destructor does not call ` frame.destroy()` , this is the responsibility
    // of the coroutine type (e.g. a generator) that owns the handle (same as for ordinary stackelss coroutines).
    ~InlineHandle() = default;

    InlineHandle(const InlineHandle&) = delete;
    InlineHandle& operator=(const InlineHandle&) = delete;

    // Direct call, no function pointer indirection.
    void resume() { frame.doStep(); }
    void destroy() { frame.destroy(); }
    bool done() const { return frame.done(); }
    explicit constexpr operator bool() const { return true; }
    auto& promise() { return frame.promise(); }
    const auto& promise() const { return frame.pt; }

};


#endif //GENERATOR_REWRITE_EXAMPLES_COROUTINE_HANDLE_H