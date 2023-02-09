//
// Created by 张德文 on 2023/1/16.
//

#include "extract.h"

#include <cstring>
#include <stddef.h>

const char* ExtractFileName(const char* _path) {
    if (nullptr == _path) {
        return "";
    }

    const char* pos = strrchr(_path, '\\');

    if (nullptr == pos) {
        pos = strrchr(_path, '/');
    }

    if (nullptr == pos || '\0' == *(pos + 1)) {
        return _path;
    } else {
        return pos + 1;
    }
}

void ExtractFunctionName(const char* _func, char* _func_ret, int _len) {

}
