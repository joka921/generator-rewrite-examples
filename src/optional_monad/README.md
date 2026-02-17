# Optional Monad Implementation

This directory contains an implementation of the optional monad pattern, adapted from [toby-allsopp's coroutine_monad](https://github.com/toby-allsopp/coroutine_monad) library.

## Files

- **LICENSE.txt** - MIT License with attribution to Toby Allsopp
- **maybe_simple.h** - Simplified optional monad header
- **maybe_simple_example.cpp** - Working example demonstrating the monad

## Implementation Note

The original plan was to integrate with this repository's manual C++17 coroutine lowering system (using `stackless_coro_crtp`/`InlineCoroImpl` and the `CO_AWAIT`/`CO_RETURN` macros). However, this encountered compilation issues related to how the macros handle awaiter types and template instantiation.

Instead, this is a **simplified implementation** that demonstrates the optional monad concept using:
- Standard C++ `std::optional<T>`
- Monadic bind operation (`and_then`)
- Function composition with automatic short-circuiting

## Features Demonstrated

✓ **Monadic composition** - Chain optional-returning operations with `and_then`
✓ **Short-circuiting** - When any operation returns `nullopt`, the entire chain returns `nullopt`
✓ **Cross-TU usage** - `safe_divide()` defined in .cpp, `safe_sqrt()` inline in .h
✓ **Exception handling** - Exceptions propagate correctly
✓ **Two styles** - Both monadic (functional) and explicit (imperative) approaches

## Building

```bash
cmake -B build
cmake --build build --target MaybeMonadExample
./build/MaybeMonadExample
```

## Example Usage

### Monadic Style (Functional)

```cpp
auto result = and_then(
    safe_divide(10, 2),    // Returns std::optional<int>(5)
    [](int x) {
        return and_then(
            safe_sqrt(x * x),  // Returns std::optional<int>(5)
            [](int y) {
                return safe_add(y, 10);  // Returns std::optional<int>(15)
            }
        );
    }
);
// result == std::optional<int>(15)
```

### Explicit Style (Imperative)

```cpp
auto r1 = safe_divide(10, 2);
if (!r1) return std::nullopt;  // Short-circuit

auto r2 = safe_sqrt(*r1);
if (!r2) return std::nullopt;  // Short-circuit

return safe_add(*r2, 10);
```

### Short-Circuiting Example

```cpp
auto result = chained_calculation(10, 0, 16);
// result == std::nullopt (short-circuited on divide by zero)
```

## Why Not Full Coroutine Integration?

The manual C++17 coroutine lowering system in this repository uses complex macros (`CO_AWAIT`, `CO_RETURN_VALUE`, etc.) that expand to state machines with handle types and symmetric transfer logic. When attempting to integrate the optional monad as an awaitable type, we encountered:

1. **Macro instantiation issues** - The `CO_AWAIT_IMPL_IMPL` macro has a lambda with conditional returns based on `if constexpr`, which causes "variable has incomplete type 'void'" errors with certain compiler versions
2. **Template deduction complexity** - The awaiter's `await_suspend` needs to work with both `stackless_coroutine_handle<Promise>` and inline frames, creating template instantiation conflicts
3. **Short-circuit mechanism** - Implementing short-circuiting requires accessing frame state (`setDone()`) which isn't exposed through the handle interface

While these issues could potentially be resolved with significant macro refactoring, the simplified approach better demonstrates the optional monad *concept* without getting bogged down in the intricacies of manual coroutine lowering.

## Key Takeaway

The optional monad pattern is about **compositional error handling** - chaining operations that might fail, with automatic short-circuiting when any operation returns "no value". This implementation demonstrates that concept clearly, even without full coroutine integration.
