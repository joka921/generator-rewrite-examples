#ifndef GENERATOR_REWRITE_EXAMPLES_UNIFIED_GENERATOR_H
#define GENERATOR_REWRITE_EXAMPLES_UNIFIED_GENERATOR_H

#include <cstddef>
#include <exception>
#include <iterator>
#include <type_traits>
#include <utility>
#include "./suspend.h"
#include "./coroutine_handle.h"
#include "./inline_handle.h"
#include "./coro_storage.h"

// Forward declarations
template<typename T, typename Policy> class unified_generator;
template<typename T> struct HeapGeneratorPolicy;
template<typename T, typename CoroFrame> struct InlineGeneratorPolicy;

namespace detail {

// ---------------------------------------------------------------------------
// Unified promise type for both heap and inline generators.
// Value storage is selected at compile time based on type traits:
//   - By-value if T is trivially copyable, sizeof <= 16, and not a reference
//   - By-pointer otherwise
// ---------------------------------------------------------------------------
template<typename T>
class unified_generator_promise {
public:
    using value_type = std::remove_cv_t<std::remove_reference_t<T>>;
    using reference_type = std::conditional_t<std::is_reference_v<T>, T, const T&>;
    using pointer_type = std::remove_reference_t<T>*;

    static constexpr bool store_by_value =
        std::is_trivially_copyable_v<value_type> &&
        sizeof(value_type) <= 16 &&
        !std::is_reference_v<T>;

    using storage_type = std::conditional_t<store_by_value, value_type, pointer_type>;

    // Return type adapts to storage mode:
    //   - by value when stored by value
    //   - by const ref (or T& for reference T) when stored as pointer
    using return_type = std::conditional_t<store_by_value, value_type, reference_type>;

    unified_generator_promise() = default;

    // get_return_object: used by heap CoroImpl::ramp(), not called in inline path.
    unified_generator<T, HeapGeneratorPolicy<T>> get_return_object() noexcept;

    constexpr SuspendAlways initial_suspend() const noexcept { return {}; }
    constexpr SuspendAlways final_suspend() const noexcept { return {}; }

    template<typename U = T,
        std::enable_if_t<!std::is_rvalue_reference<U>::value, int> = 0>
    SuspendAlways yield_value(std::remove_reference_t<T>& value) noexcept {
        if constexpr (store_by_value) { m_value = value; }
        else { m_value = std::addressof(value); }
        return {};
    }

    SuspendAlways yield_value(std::remove_reference_t<T>&& value) noexcept {
        if constexpr (store_by_value) { m_value = std::move(value); }
        else { m_value = std::addressof(value); }
        return {};
    }

    void unhandled_exception() {
        CO_STORAGE_CONSTRUCT(m_exception, (std::current_exception()));
        m_has_exception = true;
    }

    void return_void() {}

    return_type value() const noexcept {
        if constexpr (store_by_value) { return m_value; }
        else { return static_cast<reference_type>(*m_value); }
    }

    // Don't allow any use of 'co_await' inside the generator coroutine.
    template<typename U>
    void await_transform(U&& value) = delete;

    void rethrow_if_exception() {
        if (m_has_exception) {
            auto except = std::move(m_exception.get().ref_);
            m_exception.destroy();
            m_has_exception = false;
            std::rethrow_exception(std::move(except));
        }
    }

private:
    storage_type m_value;
    coro_storage<std::exception_ptr&, true> m_exception;
    bool m_has_exception = false;
};

// Sentinel type for unified generators.
struct unified_generator_sentinel {};

// ---------------------------------------------------------------------------
// Unified iterator — works with both heap handles and inline handles via a
// pointer-to-handle indirection.
// ---------------------------------------------------------------------------
template<typename T, typename Policy>
class unified_generator_iterator {
    using handle_type = typename Policy::handle_type;
    using promise_type = typename Policy::promise_type;

public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = typename promise_type::value_type;
    using reference = typename promise_type::return_type;

    unified_generator_iterator() noexcept : m_handle(nullptr) {}

    explicit unified_generator_iterator(handle_type* handle) noexcept
        : m_handle(handle) {}

    friend bool operator==(const unified_generator_iterator& it,
                           unified_generator_sentinel) noexcept {
        return !it.m_handle || !(*it.m_handle) || it.m_handle->done();
    }

    friend bool operator!=(const unified_generator_iterator& it,
                           unified_generator_sentinel s) noexcept {
        return !(it == s);
    }

    friend bool operator==(unified_generator_sentinel s,
                           const unified_generator_iterator& it) noexcept {
        return (it == s);
    }

    friend bool operator!=(unified_generator_sentinel s,
                           const unified_generator_iterator& it) noexcept {
        return it != s;
    }

    unified_generator_iterator& operator++() {
        m_handle->resume();
        if (m_handle->done()) {
            m_handle->promise().rethrow_if_exception();
        }
        return *this;
    }

    void operator++(int) { (void)operator++(); }

    reference operator*() const noexcept {
        return m_handle->promise().value();
    }

private:
    handle_type* m_handle;
};

} // namespace detail

// ---------------------------------------------------------------------------
// Policy types
// ---------------------------------------------------------------------------

// Policy for heap-allocated generators (type-erased handle, nullable).
template<typename T>
struct HeapGeneratorPolicy {
    using promise_type = detail::unified_generator_promise<T>;
    using handle_type = Handle<promise_type>;
    static constexpr bool nullable = true;
};

// Policy for inline (stack-allocated) generators (InlineHandle, non-nullable).
template<typename T, typename CoroFrame>
struct InlineGeneratorPolicy {
    using promise_type = detail::unified_generator_promise<T>;
    using handle_type = InlineHandle<CoroFrame>;
    static constexpr bool nullable = false;
};

// ---------------------------------------------------------------------------
// Unified generator
// ---------------------------------------------------------------------------
template<typename T, typename Policy>
class [[nodiscard]] unified_generator {
public:
    using handle_type = typename Policy::handle_type;
    using promise_type = typename Policy::promise_type;
    using iterator = detail::unified_generator_iterator<T, Policy>;
    using value_type = typename iterator::value_type;

    // Default constructor — only available for nullable policies (heap).
    template<bool Nullable = Policy::nullable, std::enable_if_t<Nullable, int> = 0>
    unified_generator() noexcept : m_handle(nullptr) {}

    // Construct from a handle.
    explicit unified_generator(handle_type h) noexcept
        : m_handle(std::move(h)) {}

    // Move-only.
    unified_generator(unified_generator&& other) noexcept
        : m_handle(std::move(other.m_handle)) {
        if constexpr (Policy::nullable) {
            other.m_handle = nullptr;
        }
    }

    unified_generator& operator=(unified_generator&& other) noexcept {
        if (this != &other) {
            if constexpr (Policy::nullable) {
                if (m_handle) m_handle.destroy();
                m_handle = other.m_handle;
                other.m_handle = nullptr;
            } else {
                m_handle = std::move(other.m_handle);
            }
        }
        return *this;
    }

    unified_generator(const unified_generator&) = delete;
    unified_generator& operator=(const unified_generator&) = delete;

    ~unified_generator() {
        if constexpr (Policy::nullable) {
            if (m_handle) m_handle.destroy();
        }
        // For inline: InlineHandle destructor handles cleanup via frame.destroy()
    }

    iterator begin() {
        if constexpr (Policy::nullable) {
            if (!m_handle) return iterator{nullptr};
        }
        m_handle.resume();
        if (m_handle.done()) {
            m_handle.promise().rethrow_if_exception();
        }
        return iterator{&m_handle};
    }

    detail::unified_generator_sentinel end() noexcept {
        return detail::unified_generator_sentinel{};
    }

    void swap(unified_generator& other) noexcept {
        std::swap(m_handle, other.m_handle);
    }

private:
    handle_type m_handle;
};

template<typename T, typename Policy>
void swap(unified_generator<T, Policy>& a, unified_generator<T, Policy>& b) {
    a.swap(b);
}

// Out-of-class definition of get_return_object (resolves circular dependency).
namespace detail {
    template<typename T>
    unified_generator<T, HeapGeneratorPolicy<T>>
    unified_generator_promise<T>::get_return_object() noexcept {
        using handle_t = Handle<unified_generator_promise<T>>;
        return unified_generator<T, HeapGeneratorPolicy<T>>{
            handle_t::from_promise(*this)};
    }
} // namespace detail

// ---------------------------------------------------------------------------
// Convenience aliases
// ---------------------------------------------------------------------------
template<typename T>
using heap_generator = unified_generator<T, HeapGeneratorPolicy<T>>;

template<typename T, typename CoroFrame>
using inline_gen = unified_generator<T, InlineGeneratorPolicy<T, CoroFrame>>;

#endif //GENERATOR_REWRITE_EXAMPLES_UNIFIED_GENERATOR_H
