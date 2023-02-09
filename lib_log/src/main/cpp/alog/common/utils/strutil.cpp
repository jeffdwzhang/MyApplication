//
// Created by 张德文 on 2023/1/16.
//

#include "strutil.h"

namespace strutil {

    #define STARTSWITH(T) bool StartsWith(const T& str, const T& substr) { return str.find(substr) == 0; }

    #define ENDSWITH(T) bool EndsWith(const T& str, const T& substr) { \
        size_t i = str.rfind(substr);                                      \
        return (i != T::npos) && (i == (str.length() - substr.length()));  \
    }

    STARTSWITH(std::string)
    STARTSWITH(std::wstring)

    ENDSWITH(std::string)
    ENDSWITH(std::wstring)
}