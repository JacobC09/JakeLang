#pragma once
#include <memory>
#include <variant>

// Forward declaration safe std::unique_ptr
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

// Wrapper for variant (provides indexOf<T>)
template <typename... Types>
class Variant : public std::variant<Types...> {
public:
    using base = std::variant<Types...>;

    using base::variant;
    using base::emplace;
    using base::index;
    using base::swap;
    using base::operator=;

    template <typename T>
    inline auto as() {
        return std::get<T>(*this);
    }

    template <typename T>
    inline auto as() const {
        return std::get<T>(*this);
    }

    template <typename T>
    inline bool is() {
        constexpr size_t targetIndex = variant_index<T, Types...>::value;
        return index() == targetIndex;
    }

    template <typename T>
    static constexpr size_t indexOf() {
        return variant_index<T, Types...>::value;
    }

private:
    template <typename Target, typename... Rest>
    struct variant_index;

    template <typename Target, typename... Rest>
    struct variant_index<Target, Target, Rest...> {
        static constexpr int value = 0;
    };

    template <typename Target, typename First, typename... Rest>
    struct variant_index<Target, First, Rest...> {
        static constexpr int value = 1 + variant_index<Target, Rest...>::value;
    };

    template <typename Target>
    struct variant_index<Target> {
        static constexpr int value = -1;
    };
};