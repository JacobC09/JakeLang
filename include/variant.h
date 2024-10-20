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
    // Constructors
    Ptr(T&& obj) : _impl(std::make_unique<T>(std::move(obj))) {}
    Ptr(const T& obj) : _impl(std::make_unique<T>(obj)) {}

    // Implicit conversion from std::unique_ptr
    Ptr(std::unique_ptr<T>&& uniqueObj) : _impl(std::move(uniqueObj)) {}

    Ptr(const Ptr& other) : _impl(std::make_unique<T>(*other._impl)) {}
    Ptr(Ptr&& other) noexcept = default;

    // Assignment operators
    Ptr& operator=(const Ptr& other) {
        if (this != &other) {
            *_impl = *other._impl;
        }
        return *this;
    }

    Ptr& operator=(Ptr&& other) noexcept = default;

    // Assigment operator
    Ptr& operator=(std::unique_ptr<T>&& uniqueObj) {
        _impl = std::move(uniqueObj);
        return *this;
    }

    // Access operators
    T& operator*() { return *_impl; }
    const T& operator*() const { return *_impl; }

    T* operator->() { return _impl.get(); }
    const T* operator->() const { return _impl.get(); }

private:
    std::unique_ptr<T> _impl;
};

template <typename T>
class Shared {
public:
    // Constructors
    Shared(T&& obj) : _impl(std::make_shared<T>(std::move(obj))) {}
    Shared(const T& obj) : _impl(std::make_shared<T>(obj)) {}

    // Implicit conversion from std::shared_ptr
    Shared(const std::shared_ptr<T>& sharedObj) : _impl(sharedObj) {}

    Shared(const Shared& other) = default;
    Shared(Shared&& other) noexcept = default;

    // Assignment operators
    Shared& operator=(const Shared& other) = default;
    Shared& operator=(Shared&& other) noexcept = default;

    // Assignment operator
    Shared& operator=(const std::shared_ptr<T>& sharedObj) {
        _impl = sharedObj;
        return *this;
    }

    // Access operators
    T& operator*() { return *_impl; }
    const T& operator*() const { return *_impl; }

    T* operator->() { return _impl.get(); }
    const T* operator->() const { return _impl.get(); }

private:
    std::shared_ptr<T> _impl;
};