#pragma once
#include <stdarg.h>
#include <string>

std::string formatStr(const char* format, ...) {
    va_list args;
    va_start(args, format);

    std::string result;

    while (*format) {
        if (*format == '%') {
            ++format;

            switch (*format) {
                case 'd': {
                    int i = va_arg(args, int);
                    result += std::to_string(i);
                    break;
                }

                case 'f': {
                    double d = va_arg(args, double);
                    result += std::to_string(d);
                    break;
                }

                case 's': {
                    const char* str = va_arg(args, const char*);
                    result += str;
                    break;
                }

                case 'c': {
                    char c = static_cast<char>(va_arg(args, int));
                    result.push_back(c);
                    break;
                }

                default: break;
            }

        } else {
            result.push_back(*format);
        }
        ++format;
    }

    va_end(args);

    return result;
}
