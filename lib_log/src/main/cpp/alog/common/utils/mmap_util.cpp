//
// Created by 张德文 on 2022/8/17.
//

#include "mmap_util.h"

#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <cinttypes>
#include <filesystem>

bool isMmapFileOpenSucc(mapped_file& _mmap_file) {
    return !_mmap_file.operator !() && _mmap_file.is_open();
}

bool OpenMmapFile(const char* _filepath, unsigned int _size, mapped_file& _mmap_file) {
    if (nullptr == _filepath || 0 == strnlen(_filepath, 128) || 0 == _size) {
        return false;
    }

    if (isMmapFileOpenSucc(_mmap_file)) {
        // 关闭旧的mmap文件
        CloseMmapFile(_mmap_file);
    }

    if (_mmap_file.is_open() && _mmap_file.operator!()) {
        return false;
    }

    basic_mapped_file_params<std::filesystem::path> param;
    param.path = std::filesystem::path(_filepath);
    param.flags = mapped_file_base::readwrite;

    bool file_exist = std::filesystem::exists(_filepath);
    if (!file_exist) {
        param.new_file_size = _size;
    }

    _mmap_file.open(param);

    bool is_open = isMmapFileOpenSucc(_mmap_file);

    return is_open;
}

void CloseMmapFile(mapped_file& _mmap_file) {
    if (_mmap_file.is_open()) {
        _mmap_file.close();
    }
}