//
// Created by 张德文 on 2022/8/17.
//

#ifndef MYAPPLICATION_LOG_MAGIC_NUM_H
#define MYAPPLICATION_LOG_MAGIC_NUM_H

class LogMagicNum {
public:
    LogMagicNum() = delete;

    static const char kMagicSyncZlibStart = '\x06';
    static const char kMagicSyncNoCryptZlibStart ='\x08';
    static const char kMagicAsyncZlibStart ='\x07';
    static const char kMagicAsyncNoCryptZlibStart ='\x09';

    static const char kMagicSyncZstdStart = '\x0A';
    static const char kMagicSyncNoCryptZstdStart ='\x0B';
    static const char kMagicAsyncZstdStart ='\x0C';
    static const char kMagicAsyncNoCryptZstdStart ='\x0D';

    static const char kMagicEnd  = '\0';

    static bool MagicStartIsValid(char _magic) {
        return kMagicSyncZlibStart == _magic || kMagicSyncNoCryptZlibStart == _magic
               || kMagicAsyncZlibStart == _magic || kMagicAsyncNoCryptZlibStart == _magic
               || kMagicSyncZstdStart == _magic || kMagicSyncNoCryptZstdStart == _magic
               || kMagicAsyncZstdStart == _magic || kMagicAsyncNoCryptZstdStart == _magic;
    }
};

#endif //MYAPPLICATION_LOG_MAGIC_NUM_H
