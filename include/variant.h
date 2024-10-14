#pragma once

#define MAPBOX_VARIANT_MINIMIZE_SIZE
#define MAPBOX_VARIANT_OPTIMIZE_FOR_SPEED

#include <memory>
#include "mapbox/variant.hpp"

template <typename... Types>
using Variant = mapbox::util::variant<Types...>;

template <typename T>
class Ptr {
public:
    Ptr(T &&obj) : _impl(new T(std::move(obj))) {}
    Ptr(const T &obj) : _impl(new T(obj)) {}

    Ptr(const Ptr &other) : Ptr(*other._impl) {}
    Ptr &operator=(const Ptr &other) {
        *_impl = *other._impl;
        return *this;
    }
    
    ~Ptr() = default;

    T &operator*() { return *_impl; }
    const T &operator*() const { return *_impl; }

    T *operator->() { return _impl.get(); }
    const T *operator->() const { return _impl.get(); }

    Ptr(Ptr &&other) : Ptr(std::move(*other._impl)) {}
    Ptr &operator=(Ptr &&other)
    {
        *_impl = std::move(*other._impl);
        return *this;
    }

private:
    std::unique_ptr<T> _impl;
};


template <typename T>
class Shared {
public:
    // Constructor that takes an rvalue (T&&) and moves it into a shared_ptr
    Shared(T &&obj) : _impl(std::make_shared<T>(std::move(obj))) {}

    // Constructor that takes a const reference to T and copies it
    Shared(const T &obj) : _impl(std::make_shared<T>(obj)) {}

    // Copy constructor
    Shared(const Shared &other) : _impl(other._impl) {}

    // Copy assignment operator
    Shared &operator=(const Shared &other) {
        _impl = other._impl; // Shared ownership; no deep copy needed
        return *this;
    }

    // Move constructor
    Shared(Shared &&other) noexcept : _impl(std::move(other._impl)) {}

    // Move assignment operator
    Shared &operator=(Shared &&other) noexcept {
        _impl = std::move(other._impl);
        return *this;
    }

    // Default destructor (shared_ptr will automatically handle deletion)
    ~Shared() = default;

    // Dereference operator to access the managed object
    T &operator*() { return *_impl; }
    const T &operator*() const { return *_impl; }

    // Arrow operator to access the object's members
    T *operator->() { return _impl.get(); }
    const T *operator->() const { return _impl.get(); }

private:
    std::shared_ptr<T> _impl; // Use shared_ptr instead of unique_ptr
};
