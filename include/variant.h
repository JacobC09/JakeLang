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
