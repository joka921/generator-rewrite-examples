#ifndef GENERATOR_REWRITE_EXAMPLES_INLINE_GENERATOR_H
#define GENERATOR_REWRITE_EXAMPLES_INLINE_GENERATOR_H

#include <optional>
#include "./suspend.h"
#include "./inline_handle.h"
#include "./generator.h"  // for generator_sentinel

namespace detail
{
    // Promise type for inline generators. Like `generator_promise` but without
    // `get_return_object()` and without the template-template handle parameter.
    template <typename T>
    class inline_generator_promise
    {
    public:
        using value_type = std::remove_cv_t<std::remove_reference_t<T>>;
        using reference_type = std::conditional_t<std::is_reference_v<T>, T, const T&>;
        using pointer_type = std::remove_reference_t<T>*;

        inline_generator_promise() = default;

        constexpr SuspendAlways initial_suspend() const noexcept { return {}; }
        constexpr SuspendAlways final_suspend() const noexcept { return {}; }


        /*
        SuspendAlways yield_value(std::remove_reference_t<T> &value) noexcept {
            m_value = std::addressof(value);
            return {};
        }

        SuspendAlways yield_value(std::remove_reference_t<T> &&value) noexcept {
            m_value = std::addressof(value);
            return {};
        }
        */
        SuspendAlways yield_value(std::remove_reference_t<T> value) noexcept
        {
            m_value = value;
            return {};
        }

        void unhandled_exception() { CO_STORAGE_CONSTRUCT(m_exception, (std::current_exception())); }

        void return_void()
        {
        }

        value_type value() const noexcept
        {
            return m_value;
        }

        // Don't allow any use of 'co_await' inside the generator coroutine.
        template <typename U>
        void await_transform(U&& value) = delete;

        void rethrow_if_exception()
        {
            if (m_has_exception)
            {
                auto except = std::move(m_exception.get().ref_);
                m_exception.destroy();
                std::rethrow_exception(std::move(except));
            }
        }

    private:
        value_type m_value;
        coro_storage<std::exception_ptr&, true> m_exception;
        bool m_has_exception;
    };

    // Iterator for inline generators. Stores a non-owning pointer to the InlineHandle
    // inside the generator.
    template <typename T, typename CoroFrame>
    class inline_generator_iterator
    {
        using handle_type = InlineHandle<CoroFrame>;

    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::remove_cv_t<std::remove_reference_t<T>>;
        using reference = std::conditional_t<std::is_reference_v<T>, T, const T&>;
        using pointer = std::remove_reference_t<T>*;

        inline_generator_iterator() noexcept : m_handle(nullptr)
        {
        }

        explicit inline_generator_iterator(handle_type* handle) noexcept
            : m_handle(handle)
        {
        }

        friend bool operator==(const inline_generator_iterator& it,
                               generator_sentinel) noexcept
        {
            return !it.m_handle || it.m_handle->done();
        }

        friend bool operator!=(const inline_generator_iterator& it,
                               generator_sentinel s) noexcept
        {
            return !(it == s);
        }

        friend bool operator==(generator_sentinel s,
                               const inline_generator_iterator& it) noexcept
        {
            return (it == s);
        }

        friend bool operator!=(generator_sentinel s,
                               const inline_generator_iterator& it) noexcept
        {
            return it != s;
        }

        inline_generator_iterator& operator++()
        {
            m_handle->resume();
            if (m_handle->done())
            {
                m_handle->promise().rethrow_if_exception();
            }
            return *this;
        }

        void operator++(int) { (void)operator++(); }

        value_type operator*() const noexcept { return m_handle->promise().value(); }

        pointer operator->() noexcept { return std::addressof(operator*()); }

    private:
        handle_type* m_handle;
    };
} // namespace detail

// Inline generator: stores the full coroutine frame by value (no heap allocation).
// Templated on the concrete CoroFrame type (not type-erased).
template <typename T, typename CoroFrame>
class [[nodiscard]] inline_generator
{
public:
    using iterator = detail::inline_generator_iterator<T, CoroFrame>;
    using value_type = typename iterator::value_type;

    explicit inline_generator(CoroFrame&& frame)
        : m_handle(std::move(frame))
    {
    }

    inline_generator(inline_generator&&) = default;
    inline_generator& operator=(inline_generator&&) = default;
    inline_generator(const inline_generator&) = delete;
    inline_generator& operator=(const inline_generator&) = delete;

    iterator begin()
    {
        if (m_handle)
        {
            m_handle.resume();
            if (m_handle.done())
            {
                m_handle.promise().rethrow_if_exception();
            }
        }
        return iterator{&m_handle};
    }

    detail::generator_sentinel end() noexcept
    {
        return detail::generator_sentinel{};
    }

private:
    InlineHandle<CoroFrame> m_handle;
};

#endif //GENERATOR_REWRITE_EXAMPLES_INLINE_GENERATOR_H
