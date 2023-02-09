//
// Created by 张德文 on 2022/8/17.
//

#ifndef MYAPPLICATION_MMAP_UTIL_H
#define MYAPPLICATION_MMAP_UTIL_H

#include "mapped_file.h"

bool isMmapFileOpenSucc(mapped_file& _mmap_file);

bool OpenMmapFile(const char* _filepath, unsigned int _size, mapped_file& _mmap_file);

void CloseMmapFile(mapped_file& _mmap_file);

#endif //MYAPPLICATION_MMAP_UTIL_H
