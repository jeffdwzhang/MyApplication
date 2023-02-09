//
// Created by 张德文 on 2023/1/16.
//

#ifndef MYAPPLICATION_STRUTIL_H
#define MYAPPLICATION_STRUTIL_H

#include <string>
#include <vector>

namespace strutil {

    bool StartsWith(const std::string& str, const std::string& substr);
    bool EndsWith(const std::string& str, const std::string& substr);

    bool StartsWith(const std::wstring& str, const std::wstring& substr);
    bool EndsWith(const std::wstring& str, const std::wstring& substr);
}


#endif //MYAPPLICATION_STRUTIL_H
