//
// Created by 张德文 on 2023/1/16.
//

#ifndef MYAPPLICATION_EXTRACT_H
#define MYAPPLICATION_EXTRACT_H

#ifdef __cplusplus
extern "C" {
#endif

const char* ExtractFileName(const char* _path);

void ExtractFunctionName(const char* _func, char* _func_ret, int _len);

#ifdef __cplusplus
}
#endif


#endif //MYAPPLICATION_EXTRACT_H
