#ifndef RETURN_OBJECT_HOLDER_H
#define RETURN_OBJECT_HOLDER_H

#include <iostream>
#include <optional>
#include <utility>

// An object that starts out unitialized. Initialized by a call to emplace.
template <typename T>
using deferred = std::optional<T>;

template <typename T>
struct return_object_holder {
  // The staging object that is returned (by copy/move) to the caller of the coroutine.
  deferred<T> stage;
  return_object_holder*& p;

  // When constructed, we assign a pointer to ourselves to the supplied reference to
  // pointer.
  return_object_holder(return_object_holder*& p) : stage{}, p(p) { p = this; }

  // Copying doesn't make any sense (which copy should the pointer refer to?).
  return_object_holder(return_object_holder const&) = delete;
  // To move, we just update the pointer to point at the new object.
  return_object_holder(return_object_holder&& other) : stage(std::move(other.stage)), p(other.p) {
    p = this;
  }

  // Assignment doesn't make sense.
  void operator=(return_object_holder const&) = delete;
  void operator=(return_object_holder&&) = delete;

  ~return_object_holder() = default;

  // Construct the staging value; arguments are perfect forwarded to T's constructor.
  template <typename... Args>
  void emplace(Args&&... args) {
    stage.emplace(std::forward<Args>(args)...);
  }

  // We assume that we will be converted only once, so we can move from the staging
  // object. We also assume that `emplace` has been called at least once.
  operator T() { return std::move(*stage); }
};

template <typename T>
auto make_return_object_holder(return_object_holder<T>*& p) {
  return return_object_holder<T>{p};
}

#endif  // RETURN_OBJECT_HOLDER_H