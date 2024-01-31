#pragma once
#include "debug.h"

// This is perhaps the most complicated peice of code I have ever written

template <typename ...Types>
class Variant {
public:
    Variant() : mIndex(-1), mStorage(nullptr) {};

    template <typename T>
    Variant(const T& value) {
        mIndex = index<T>();

        // if constexpr (sizeof(T) <= sizeof(mStorage)) {
        //     *((T*)mStorage) = value;
        // } else {
        // }

        mStorage = new Storage<T>(value);
    }

    ~Variant() {
        delete mStorage;
    }

    template <typename T>
    inline T as() const {
        if (index<T>() != mIndex) {
            throw InvalidAccess();
        }        

        // if constexpr (sizeof(T) <= sizeof(mStorage)) {
        //     return *((T*)mStorage);
        // }

        return reinterpret_cast<Storage<T>*>(mStorage)->value;
    }

    template <typename T>
    inline bool is() {
        return mIndex == index<T>();
    }

    template <typename T>
    inline static constexpr int index() {
        return IndexOfType<T, Types...>::value;
    }

private:
    template <typename T, typename ... Rest>
    struct IndexOfType;

    template <typename T, typename ...Rest>
    struct IndexOfType<T, T, Rest...> {
        static constexpr int value = 0;
    };

    template <typename T, typename First, typename ...Rest>
    struct IndexOfType<T, First, Rest...> {
        static constexpr int value = 1 + IndexOfType<T, Rest...>::value;
    };

    template <typename T>
    struct IndexOfType<T> {
        static constexpr int value = -1;
    };

private:
    struct BasicStorage {
        virtual ~BasicStorage() = default;
    };

    template <typename T>
    struct Storage : BasicStorage {
        Storage(T value) : value(value) {};

        T value;
    };

    int mIndex;
    BasicStorage* mStorage;

public:
    class InvalidAccess : public std::exception {
    public: 
        char* what() {
            return "Invalid Variant Access";
        }
    };
};