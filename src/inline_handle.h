#ifndef GENERATOR_REWRITE_EXAMPLES_INLINE_HANDLE_H
#define GENERATOR_REWRITE_EXAMPLES_INLINE_HANDLE_H

#include <utility>

// Owning, move-only handle that stores a CoroFrame by value (no heap allocation).
template<typename CoroFrame>
struct InlineHandle {
    CoroFrame frame;
    bool m_valid = true;

    explicit InlineHandle(CoroFrame&& f) noexcept : frame(std::move(f)) {}

    // Move constructor — invalidates the source to prevent double-destroy.
    InlineHandle(InlineHandle&& other) noexcept
        : frame(std::move(other.frame)), m_valid(other.m_valid) {
        other.m_valid = false;
    }

    InlineHandle& operator=(InlineHandle&& other) noexcept {
        if (this != &other) {
            if (m_valid) frame.destroy();
            frame = std::move(other.frame);
            m_valid = other.m_valid;
            other.m_valid = false;
        }
        return *this;
    }

    // Properly cleans up the suspended coroutine frame.
    ~InlineHandle() {
        if (m_valid) frame.destroy();
    }

    InlineHandle(const InlineHandle&) = delete;
    InlineHandle& operator=(const InlineHandle&) = delete;

    // Direct call, no function pointer indirection.
    void resume() { frame.doStep(); }
    bool done() const { return frame.done(); }
    auto& promise() { return frame.promise(); }
    const auto& promise() const { return frame.pt; }
    explicit operator bool() const { return m_valid; }
};

#endif //GENERATOR_REWRITE_EXAMPLES_INLINE_HANDLE_H
