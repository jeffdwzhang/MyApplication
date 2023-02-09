//
// Created by 张德文 on 2022/7/26.
//

#ifndef MYAPPLICATION_ALOGAPPENDER_H
#define MYAPPLICATION_ALOGAPPENDER_H

#include "ALogBase.h"
#include "log_base_buffer.h"
#include "thread.h"
#include "condition.h"
#include "mapped_file.h"

class ALogAppender {

public:
    static ALogAppender* NewInstance(const ALogConfig& _config);
    static void DelayRelease(ALogAppender*& _appender);
    static void Release(ALogAppender*& _appender);

    void Open(const ALogConfig& _config);
    void Write(const LoggerInfo* _info, const char* _log);
    void SetMode(AppenderMode _mode);
    void Flush();
    void FlushSync();
    void Close();
    void WriteTips2File(const char* _tips_format, ...);
    const char* Dump(const void* _dumpbuffer, size_t _len);

    bool GetCurrentLogPath(char* _log_path, uint32_t _len);
    bool GetCurrentLogCachePath(char* _cache_path, uint32_t _len);
    void SetConsoleLog(bool _is_open);
    void SetMaxFileSize(uint64_t _max_byte_size);
    void SetMaxAliveDuration(uint64_t _max_time);
    bool GetFilePathFromTimespan(int _timespan, const char* _prefix, std::vector<std::string>& _filepath_vec);
    bool MakeLogFileName(int _timespan, const char* _prefix, std::vector<std::string>& _filepath_vec);

private:
    ALogAppender(const ALogConfig& _config);
    ~ALogAppender();

    std::string __MakeLogFileNamePrefix(const timeval& _tv,
                                        const char* _prefix);
    void __GetFileNamesByPrefix(const std::string& _logdir,
                                const std::string& _fileprefix,
                                const std::string& _fileext,
                                std::vector<std::string>& _filename_vec);
    void __GetFilePathsFromTimeval(const timeval& _tv,
                                   const std::string& _logdir,
                                   const char* _prefix,
                                   const std::string& _fileext,
                                   std::vector<std::string>& _filepath_vec);

    long __GetNextFileIndex(const std::string& _fileprefix,
                            const std::string& _fileext);
    void __MakeLogFileName(const timeval& _tv,
                           const std::string& _logdir,
                           const char* _prefix,
                           const std::string& _fileext,
                           char* _filepath,
                           unsigned int _len);

    void __GetMarkInfo(char* _info, size_t _info_len);
    void __WriteTips2Console(const char* _tips_format, ...);

    bool __WriteFile(const void* _data, size_t _len, FILE* _file);
    bool __OpenLogFile(const std::string& _log_dir);
    void __CloseLogFile();
    bool __CacheLogs();
    void __Log2File(const void* _data, size_t _len, bool _move_file);
    void __AsyncLogThread();
    void __WriteSync(const LoggerInfo*_info, const char *_log);
    void __WriteAsync(const LoggerInfo*_info, const char *_log);
    void __DelTimeoutFile(const std::string& _log_path);
    bool __AppendFile(const std::string& _src_file, const std::string& _dst_file);
    void __MoveOldFiles(const std::string& _src_path, const std::string& _dst_path,
                        const std::string& _name_prefix);

private:
    ALogConfig m_config;
    LogBaseBuffer* m_log_buff = nullptr;
    mapped_file m_mmap_file;
    Thread m_thread_async;       // 异步任务线程
    Mutex m_mutex_buffer_async;
    Mutex m_mutex_log_file;

    FILE * m_logfile = nullptr;
    time_t open_file_time = 0;

#ifdef DEBUG
    bool m_consolelog_open = true;
#else
    bool m_consolelog_open = false;
#endif
    bool m_log_close = true;
    Condition m_cond_buffer_async;
    uint64_t m_max_file_size = 0;
    uint64_t m_max_alive_time = 10 * 24 * 60 * 60;   // 10天

    time_t m_last_time = 0;
    uint64_t m_last_tick = 0;
    char m_last_file_path[1024] = {0};

};


#endif //MYAPPLICATION_ALOGAPPENDER_H
