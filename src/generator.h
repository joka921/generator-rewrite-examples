#ifndef GENERATOR_REWRITE_EXAMPLES_GENERATOR_H
#define GENERATOR_REWRITE_EXAMPLES_GENERATOR_H

#include "./suspend.h"

 // A simple generator class. It is templated on the used `Handle` type, so we can either use it with `std::coroutine_handle`, or with our own.
    template<typename T,
        template <typename...> typename GeneratorHandle
        >
    class generator;

    namespace detail {
        template<typename T,
            template <typename...> typename GeneratorHandle>
        class generator_promise {
        public:
            // Even if the generator only yields `const` values, the `value_type`
            // shouldn't be `const` because otherwise several static checks when
            // interacting with the STL fail.
            using value_type = std::remove_cv_t<std::remove_reference_t<T>>;
            using reference_type = std::conditional_t<std::is_reference_v<T>, T, T &>;
            using pointer_type = std::remove_reference_t<T> *;

            generator_promise() = default;

            generator<T, GeneratorHandle> get_return_object() noexcept;

            constexpr SuspendAlways initial_suspend() const noexcept {
                return {};
            }

            constexpr SuspendAlways final_suspend() const noexcept { return {}; }

            template<typename U = T,
                std::enable_if_t<!std::is_rvalue_reference<U>::value, int> = 0>
            SuspendAlways yield_value(
                std::remove_reference_t<T> &value) noexcept {
                m_value = std::addressof(value);
                return {};
            }

            SuspendAlways yield_value(
                std::remove_reference_t<T> &&value) noexcept {
                m_value = std::addressof(value);
                return {};
            }

            void unhandled_exception() { m_exception = std::current_exception(); }

            void return_void() {
            }

            reference_type value() const noexcept {
                return static_cast<reference_type>(*m_value);
            }

            // Don't allow any use of 'co_await' inside the generator coroutine.
            template<typename U>
            void await_transform(U &&value) = delete;

            void rethrow_if_exception() const {
                if (m_exception) {
                    std::rethrow_exception(m_exception);
                }
            }

        private:
            pointer_type m_value;
            std::exception_ptr m_exception;
        };

        struct generator_sentinel {
        };

        template<typename T,
            template <typename...> typename GeneratorHandle,
            bool ConstDummy = false>
        class generator_iterator {
            using promise_type = generator_promise<T, GeneratorHandle>;
            using coroutine_handle = GeneratorHandle<promise_type>;

        public:
            using iterator_category = std::input_iterator_tag;
            // What type should we use for counting elements of a potentially infinite
            // sequence?
            using difference_type = std::ptrdiff_t;
            using value_type = typename promise_type::value_type;
            using reference = typename promise_type::reference_type;
            using pointer = typename promise_type::pointer_type;

            // Iterator needs to be default-constructible to satisfy the Range concept.
            generator_iterator() noexcept : m_coroutine(nullptr) {
            }

            explicit generator_iterator(coroutine_handle coroutine) noexcept
                : m_coroutine(coroutine) {
            }

            friend bool operator==(const generator_iterator &it,
                                   generator_sentinel) noexcept {
                return !it.m_coroutine || it.m_coroutine.done();
            }

            friend bool operator!=(const generator_iterator &it,
                                   generator_sentinel s) noexcept {
                return !(it == s);
            }

            friend bool operator==(generator_sentinel s,
                                   const generator_iterator &it) noexcept {
                return (it == s);
            }

            friend bool operator!=(generator_sentinel s,
                                   const generator_iterator &it) noexcept {
                return it != s;
            }

            generator_iterator &operator++() {
                m_coroutine.resume();
                if (m_coroutine.done()) {
                    m_coroutine.promise().rethrow_if_exception();
                }

                return *this;
            }

            // Need to provide post-increment operator to implement the 'Range' concept.
            void operator++(int) { (void) operator++(); }

            reference operator*() const noexcept { return m_coroutine.promise().value(); }

            pointer operator->() const noexcept { return std::addressof(operator*()); }

        private:
            coroutine_handle m_coroutine;
        };
    } // namespace detail

    template<typename T,
        template <typename...> typename GeneratorHandle>
    class [[nodiscard]] generator {
    public:
        using promise_type = detail::generator_promise<T, GeneratorHandle>;
        using iterator =
        detail::generator_iterator<T, GeneratorHandle, false>;
        using value_type = typename iterator::value_type;

        generator() noexcept : m_coroutine(nullptr) {
        }

        generator(generator &&other) noexcept : m_coroutine(other.m_coroutine) {
            other.m_coroutine = nullptr;
        }

        generator(const generator &other) = delete;

        explicit generator(GeneratorHandle<promise_type> coroutine) noexcept
           : m_coroutine(coroutine) {
        }

        ~generator() {
            if (m_coroutine) {
                m_coroutine.destroy();
            }
        }

        generator &operator=(generator other) noexcept {
            swap(other);
            return *this;
        }

        iterator begin() {
            if (m_coroutine) {
                m_coroutine.resume();
                if (m_coroutine.done()) {
                    m_coroutine.promise().rethrow_if_exception();
                }
            }

            return iterator{m_coroutine};
        }

        detail::generator_sentinel end() noexcept {
            return detail::generator_sentinel{};
        }

        void swap(generator &other) noexcept {
            std::swap(m_coroutine, other.m_coroutine);
        }

    private:
        friend class detail::generator_promise<T, GeneratorHandle>;

        GeneratorHandle<promise_type> m_coroutine;
    };

    template<typename T, template <typename...> typename H>
    void swap(generator<T, H> &a, generator<T, H> &b) {
        a.swap(b);
    }

    namespace detail {
        template<typename T, template <typename...> typename H>
        generator<T, H>
        generator_promise<T, H>::get_return_object() noexcept {
            using coroutine_handle = H<generator_promise<T, H> >;
            return generator<T, H>{coroutine_handle::from_promise(*this)};
        }
    } // namespace detail



#endif //GENERATOR_REWRITE_EXAMPLES_GENERATOR_H