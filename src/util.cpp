#include <string>
#include <cstdarg>
#include <iomanip>
#include <sstream>
#include <iostream>

std::string formatString(const std::string& str, int width, int precision, bool leftAlign, bool zeroPad) {
    std::ostringstream oss;

    // Set alignment
    if (leftAlign) {
        oss << std::left;
    } else {
        oss << std::right;  // Default is right alignment
        if (zeroPad) {
            oss << std::setfill('0');  // Use zero padding if not left aligned
        } else {
            oss << std::setfill(' ');  // Default is space fill
        }
    }

    // Apply width and precision
    if (precision >= 0) {
        oss << std::fixed << std::setprecision(precision);
    }
    oss << std::setw(width) << str;

    return oss.str();
}

std::string formatStr(const char* format, ...) {
    va_list args;
    va_start(args, format);

    std::string result;

    while (*format) {
        if (*format == '%') {
            ++format;
            bool leftAlign = false;
            bool zeroPad = false;
            int width = 0;
            int precision = -1;

            // Check for flags
            if (*format == '-') {
                leftAlign = true;
                ++format;
            } else if (*format == '0') {
                zeroPad = true;
                ++format;
            }

            // Parse width
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                ++format;
            }

            // Parse precision
            if (*format == '.') {
                ++format;
                precision = 0;
                while (*format >= '0' && *format <= '9') {
                    precision = precision * 10 + (*format - '0');
                    ++format;
                }
            }

            switch (*format) {
                case 'd': {
                    int i = va_arg(args, int);
                    result += formatString(std::to_string(i), width, -1, leftAlign, zeroPad);
                    break;
                }

                case 'f':  // Support for float and double
                case 'l':  // Support for long double as well (could be '%lf')
                {
                    if (*format == 'l') format++;
                    double d = va_arg(args, double);
                    std::ostringstream floatStream;
                    if (precision >= 0) {
                        floatStream << std::fixed << std::setprecision(precision) << d;
                    } else {
                        floatStream << d;  // Default precision
                    }
                    result += formatString(floatStream.str(), width, precision, leftAlign, zeroPad);
                    break;
                }

                case 's': {
                    std::string str = va_arg(args, std::string);
                    result += formatString(str, width, precision, leftAlign, zeroPad);
                    break;
                }

                case 'p': {
                    const char* cstr = va_arg(args, const char*);
                    result += formatString(cstr ? cstr : "(null)", width, precision, leftAlign, zeroPad);
                    break;
                }

                case 'c': {
                    char c = static_cast<char>(va_arg(args, int));
                    result += formatString(std::string(1, c), width, precision, leftAlign, zeroPad);
                    break;
                }

                case 'x': {
                    int x = va_arg(args, int);
                    std::ostringstream hexStream;
                    if (zeroPad && !leftAlign) hexStream << std::setfill('0');
                    hexStream << std::hex << std::setw(width) << x;  // Format hex
                    result += hexStream.str();
                    break;
                }

                case '%': {
                    result += '%';
                    break;
                }

                default: {
                    result += "<unsupported format>";
                    break;
                }
            }
        } else {
            result.push_back(*format);
        }
        ++format;
    }

    va_end(args);
    return result;
}
