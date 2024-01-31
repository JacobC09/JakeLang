#pragma once

template <typename ...Types>
class Variant {
public:
    Variant() : mIndex(-1), mStorage(nullptr) {};

    template <typename T>
    Variant(const T& value) {
        mIndex = index<T>();
        mStorage = new Storage<T>(value);
    }

    ~Variant() {
        delete mStorage;
    }

    template <typename T>
    inline T as() const {
        if (index<T>() != mIndex) throw InvalidAccess();     

        return reinterpret_cast<Storage<T>*>(mStorage)->value;
    }

    template <typename T>
    inline bool is() {
        return mIndex == index<T>();
    }

    template <typename T>
    inline static constexpr int index() {
        static int typeIndex = totalTypes++;
        return typeIndex;
    }

private:
    inline static int totalTypes = 0;

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