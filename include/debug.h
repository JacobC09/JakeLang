#pragma once
#include <iostream>
#include <vector>

inline void print() {
    std::cout << std::endl;
}

template<typename T>
inline void print(std::vector<T> vector) {
    std::cout << "{";
    for (int i = 0; i < vector.size(); i++) {
        std::cout << vector[i] << (i == vector.size() - 1 ? "}" : ", ");
    }
    
    if (!vector.size()) 
        std::cout << "}";

    std::cout << std::endl;
}

template<typename First, typename ... Strings>
inline void print(First arg, const Strings&... rest) {
    std::cout << arg << " ";
    print(rest...);
}