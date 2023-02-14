//
// Created by 张德文 on 2022/7/26.
//

#include <cinttypes>
#include <filesystem>
#include "log_zstd_buffer.h"
#include "log_zlib_buffer.h"
#include "ALogAppender.h"
#include "lock.h"
#include "mutex.h"
#include "thread.h"
#include "tickcount.h"
#include "mmap_util.h"
#include "strutil.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "ALogAppender"
#include "LogUtil.h"
#include "time_utils.h"

#define LOG_EXT "alog"

#define HEX_STRING "0123456789abcdef"

static const int file_path_len = 1024;

static const int kMaxDumpLength = 4096;
static const unsigned int kBufferBlockLength = 150 * 1024;  // 150K
static const long kMinLogAliveTime = 2 * 24 * 60 * 60;    // 2 days in second

static Mutex sg_mutex_dir_attr;

extern void log_formater(const LoggerInfo* _info, const char* _log, PtrBuffer& _log_buffer);

void (*g_log_write_callback)(const LoggerInfo*, const char*) = nullptr;

ALogAppender* ALogAppender::NewInstance(const ALogConfig& _config) {
//    LOGD("NewInstance");
    return new ALogAppender(_config);
}

/**
 * 延迟释放
 * @param _appender
 */
void ALogAppender::DelayRelease(ALogAppender*& _appender) {
    LOGD("delay release");
    if (_appender->m_log_close) {
        return;
    }
    _appender->Close();
    // 启动线程，5s后调用release
    Thread(std::bind(&Release, _appender)).start_after(5000); // 延迟5s释放
}

void ALogAppender::Release(ALogAppender*& _appender) {

    LOGD("release");
    _appender->Close();
    delete _appender;
    _appender = nullptr;
    return;
}

ALogAppender::ALogAppender(const ALogConfig& _config)
    : m_thread_async(std::bind(&ALogAppender::__AsyncLogThread, this)) {
    // 打开
    Open(_config);
}

ALogAppender::~ALogAppender() {

}

/**
 * 打开一个log输出实例
 *
 * @param _config
 */
void ALogAppender::Open(const ALogConfig &_config) {

    LOGD("Open begin.");

    m_config = _config;
    LOGD("Open -> log dir %s", m_config.logdir.c_str());

    // 加锁
    ScopedLock dir_attr_lock(sg_mutex_dir_attr);
    if (!m_config.cachedir.empty()) {
        // 创建缓存目录
        LOGD("Open -> create cache dir %s", m_config.cachedir.c_str());
        std::filesystem::create_directories(m_config.cachedir);

        Thread(std::bind(&ALogAppender::__DelTimeoutFile, this, m_config.cachedir)).start_after(2 * 10 * 1000);
        Thread(std::bind(&ALogAppender::__MoveOldFiles, this, m_config.cachedir, m_config.logdir, m_config.nameprefix)).start_after(3 * 10 * 1000);
    }

    // 创建log文件目录
    std::filesystem::create_directories(m_config.logdir);
    // 启动删除过期文件的线程任务
    Thread(std::bind(&ALogAppender::__DelTimeoutFile, this, m_config.logdir)).start_after(2 * 10 * 1000);

    dir_attr_lock.unlock();

    tickcount_t tick;
    tick.gettickcount();

    //
    char mmap_file_path[512] = {0};
    snprintf(mmap_file_path, sizeof(mmap_file_path), "%s/%s.mmap3",
             m_config.cachedir.empty() ? m_config.logdir.c_str() : m_config.cachedir.c_str(),
             m_config.nameprefix.c_str());
    LOGD("mmap file path : %s", mmap_file_path);

    LOGD("config compress_mode:%d", _config.compress_mode);
    LOGD("config public_key:%s", _config.pub_key.c_str());
    bool use_mmap = false;
    if (OpenMmapFile(mmap_file_path, kBufferBlockLength, m_mmap_file)) {
        if (_config.compress_mode == kZstd) {
            m_log_buff = new LogZstdBuffer(m_mmap_file.data(),
                                           kBufferBlockLength, true, m_config.pub_key.c_str(),
                                           m_config.compress_level);
        } else {
            m_log_buff = new LogZlibBuffer(m_mmap_file.data(), kBufferBlockLength, true, m_config.pub_key.c_str());
        }
        use_mmap = true;
    } else {

        char* buffer = new char[kBufferBlockLength];

        if (m_config.compress_mode == kZstd) {
            m_log_buff = new LogZstdBuffer(buffer, kBufferBlockLength, true, m_config.pub_key.c_str(), m_config.compress_level);
        } else {
            m_log_buff = new LogZlibBuffer(buffer, kBufferBlockLength, true, m_config.pub_key.c_str());
        }
        use_mmap = false;
    }

    if (nullptr == m_log_buff->GetData().Ptr()) {
        if (use_mmap && m_mmap_file.is_open()) {
            CloseMmapFile(m_mmap_file);
        }
        return;
    }
    LOGD("Open -> use mmap:%d", use_mmap);

    AutoBuffer buffer;
    m_log_buff->Flush(buffer);

    ScopedLock lock(m_mutex_log_file);
    m_log_close = false;
    LOGD("Open -> set mode to %d", m_config.mode);
    SetMode(m_config.mode);
    lock.unlock();

    char mark_info[512] = {0};
    __GetMarkInfo(mark_info, sizeof(mark_info));
//    LOGD("Open -> mark info : %s", mark_info);

    if (buffer.Ptr()) {
        WriteTips2File("~~~~~~~~~ begin of mmap ~~~~~~~~~~\n");
        __Log2File(buffer.Ptr(), buffer.Length(), false);
        WriteTips2File("~~~~~~~~~ end of mmap ~~~~~%s\n", mark_info);
    }

    tickcountdiff_t get_mmap_time = tickcount_t().gettickcount() - tick;

    char appender_info[728] = {0};
    snprintf(appender_info, sizeof(appender_info), "^^^^^^^^^^" __DATE__ "^^^" __TIME__ "^^^^^^^^^^%s", mark_info);

    Write(nullptr, appender_info);

    char logmsg[256] = {0};
    snprintf(logmsg, sizeof(logmsg), "get mmap time: %" PRIu64, (int64_t)get_mmap_time);
    Write(nullptr, logmsg);

    snprintf(logmsg, sizeof(logmsg), "log appender mode:%d, use mmap:%d", (int)m_config.mode, use_mmap);
    Write(nullptr, logmsg);

    if (!m_config.cachedir.empty()) {
        std::filesystem::space_info info = std::filesystem::space(m_config.cachedir);
        snprintf(logmsg, sizeof(logmsg), "cache dir space info, capacity:%" PRIuMAX" free:%" PRIuMAX" available:%" PRIuMAX, info.capacity, info.free, info.available);
        Write(nullptr, logmsg);
    }

    std::filesystem::space_info info = std::filesystem::space(m_config.logdir);
    snprintf(logmsg, sizeof(logmsg), "log dir space info, capacity:%" PRIuMAX" free:%" PRIuMAX" available:%" PRIuMAX, info.capacity, info.free, info.available);
    Write(nullptr, logmsg);

    LOGD("Open end.");

}

void ALogAppender::Write(const LoggerInfo* _info, const char* _log) {
    if (m_log_close) {
        // 如果log已经关闭了那么就直接返回呗
        return;
    }

    if (nullptr == _log) {
        // log内容为null，等于不写入任何东西
        return;
    }

    thread_local uint32_t recursion_count = 0;
    thread_local std::string recursion_str;
    recursion_count++; // 递归计数加1

    if (m_consolelog_open || (nullptr != _info && 1 == _info->traceLog)) {
        // 输出到console
        __android_log_write(ANDROID_LOG_INFO, nullptr == _info ? LOG_TAG : _info->tag, _log);
    }

    if (g_log_write_callback != nullptr) {
        g_log_write_callback(_info, _log);
    }

    if (2 <= recursion_count && recursion_str.empty()) {
        // 如果出现来递归调用，输出对应的错误信息
        if (recursion_count > 10) {
            // 递归深度最大不超过10
            return;
        }
        recursion_str.resize(kMaxDumpLength);
        LoggerInfo info = *_info;
        info.level = kLevelFatal;

        char recursive_log[256] = {0};
        snprintf(recursive_log, sizeof(recursive_log), "ERROR!!!  Recursive calls!!!, count:%u", recursion_count);

        PtrBuffer tmp((void *)recursion_str.data(), 0, kMaxDumpLength);
        log_formater(&info, recursive_log, tmp);
        if (recursion_str.capacity() >= strnlen(_log, kMaxDumpLength)) {
            recursion_str += _log;
        }
        // 输出信息到控制台
//        ConsoleLog(&info, recursion_str.c_str());
    } else {

        if (!recursion_str.empty()) {
            WriteTips2File(recursion_str.c_str());
            recursion_str.clear();
        }

        if (kAppenderSync == m_config.mode) {
            __WriteSync(_info, _log);
        } else {
            __WriteAsync(_info, _log);
        }
    }

    recursion_count--;  // 结束时递归深度减1
}

void ALogAppender::SetMode(AppenderMode _mode) {
    m_config.mode = _mode;
    m_cond_buffer_async.notifyAll();

    if (kAppenderAsync == m_config.mode && !m_thread_async.isruning()) {
        LOGD("SetMode -> start Async Thread");
        m_thread_async.start();
    }
}

void ALogAppender::Flush() {
    m_cond_buffer_async.notifyAll();
}

void ALogAppender::FlushSync() {
    if (kAppenderSync == m_config.mode) {
        // 同步写模式下，内容已经刷新了，无需再次操作
        return;
    }

    // 加锁
    ScopedLock lock_buffer(m_mutex_buffer_async);

    if (nullptr == m_log_buff) {
        //
        return;
    }

    LOGD("FlushSync -> flush data to file");
    AutoBuffer tmp;
    m_log_buff->Flush(tmp);
    lock_buffer.unlock();

    if (tmp.Ptr()) {
        __Log2File(tmp.Ptr(), tmp.Length(), false);
    }

}

void ALogAppender::Close() {
    if (m_log_close) return;

    LOGD("Close log file");

    char mark_info[512] = {0};
    __GetMarkInfo(mark_info, sizeof(mark_info));

    char appender_info[728] = {0};
    snprintf(appender_info, sizeof(appender_info), "$$$$$$$$$$" __DATE__ "$$$" __TIME__ "$$$$$$$$$$%s\n", mark_info );
    Write(nullptr, appender_info);

    m_log_close = true;

    m_cond_buffer_async.notifyAll();  // 唤起异步任务线程
    if (m_thread_async.isruning()) {
        // 等待异步任务线程关闭
        m_thread_async.join();
    }

    ScopedLock buffer_lock(m_mutex_buffer_async);

    if (m_mmap_file.is_open()) {

        CloseMmapFile(m_mmap_file);
    } else {
        if (nullptr != m_log_buff) {
            delete[] (char*)((m_log_buff->GetData()).Ptr());
        }
    }

    delete m_log_buff;
    m_log_buff = nullptr;
    buffer_lock.unlock();

    ScopedLock lock(m_mutex_log_file);
    // 关闭log文件
    __CloseLogFile();

}

void ALogAppender::WriteTips2File(const char* _tips_format, ...) {
    if (nullptr == _tips_format) {
        return;
    }

    char tips_info[4 * 1024] = {0};
    va_list ap;
    va_start(ap, _tips_format);
    vsnprintf(tips_info, sizeof(tips_info), _tips_format, ap);
    va_end(ap);

    AutoBuffer tmp_buff;
    m_log_buff->Write(tips_info, strnlen(tips_info, sizeof(tips_info)), tmp_buff);

    __Log2File(tmp_buff.Ptr(), tmp_buff.Length(), false);
}

bool ALogAppender::GetCurrentLogPath(char* _log_path, uint32_t _len) {
    if (nullptr == _log_path || 0 == _len) {
        return false;
    }

    if (m_config.logdir.empty()) {
        return false;
    }

    strncpy(_log_path, m_config.logdir.c_str(), _len-1);
    _log_path[_len-1] = '\0';
    LOGD("GetCurrentLogPath -> log path:%s", _log_path);
    return true;
}

bool ALogAppender::GetCurrentLogCachePath(char* _cache_path, uint32_t _len) {
    if (nullptr == _cache_path || 0 == _len) {
        return false;
    }

    if (m_config.cachedir.empty()) {
        return false;
    }

    strncpy(_cache_path, m_config.cachedir.c_str(), _len-1);
    _cache_path[_len-1] = '\0';
    LOGD("GetCurrentLogCachePath -> log cache path:%s", _cache_path);
    return true;
}

void ALogAppender::SetConsoleLog(bool _is_open) {
    m_consolelog_open = _is_open;
}

void ALogAppender::SetMaxFileSize(uint64_t _max_byte_size) {
    m_max_file_size = _max_byte_size;
}

void ALogAppender::SetMaxAliveDuration(uint64_t _max_time) {
    if (_max_time > kMinLogAliveTime) {
        m_max_alive_time = _max_time;
    }
}

std::string ALogAppender::__MakeLogFileNamePrefix(const timeval& _tv,
                                    const char* _prefix) {
    time_t sec = _tv.tv_sec;
    tm tcur = *localtime((const time_t*)&sec);

    char temp[64] = {0};
    snprintf(temp, 64, "_%d%02d%02d", 1900 + tcur.tm_year, 1 + tcur.tm_mon, tcur.tm_mday);

    std::string fileNamePrefix = _prefix;
    fileNamePrefix += temp;
    LOGD("__MakeLogFileNamePrefix -> fileNamePrefix:%s", fileNamePrefix.c_str());
    return fileNamePrefix;
}

void ALogAppender::__GetFileNamesByPrefix(const std::string& _logdir,
                            const std::string& _fileprefix,
                            const std::string& _fileext,
                            std::vector<std::string>& _filename_vec) {
    std::filesystem::path path(_logdir);
    if (!std::filesystem::is_directory(path)) {
        return;
    }

    std::filesystem::directory_iterator end_iter;
    std::string fileName;

    for (std::filesystem::directory_iterator iter(path); iter != end_iter; ++iter) {
        if (std::filesystem::is_regular_file(iter->status())) {
            fileName = iter->path().filename().string();
            if (strutil::StartsWith(fileName, _fileprefix) && strutil::EndsWith(fileName, LOG_EXT)) {
                _filename_vec.push_back(fileName);
            }
        }
    }
}

static bool __string_compare_greater(const std::string& s1, const std::string& s2) {
    if (s1.length() == s2.length()) {
        return s1 > s2;
    }
    return s1.length() > s2.length();
}

long ALogAppender::__GetNextFileIndex(const std::string& _fileprefix,
                        const std::string& _fileext) {

    std::vector<std::string> fileNameVec;
    __GetFileNamesByPrefix(m_config.logdir, _fileprefix, _fileext, fileNameVec);
    if (!m_config.cachedir.empty()) {
        __GetFileNamesByPrefix(m_config.cachedir, _fileprefix, _fileext, fileNameVec);
    }

    long index = 0;
    if (fileNameVec.empty()) {
        return index;
    }

    std::sort(fileNameVec.begin(), fileNameVec.end(), __string_compare_greater);
    std::string last_filename = *(fileNameVec.begin());
    std::size_t ext_pos = last_filename.rfind("."+_fileext);
    std::size_t index_len = ext_pos - _fileprefix.length();
    if (index_len > 0) {
        std::string index_str = last_filename.substr(_fileprefix.length(), index_len);
        if (strutil::StartsWith(index_str, "_")) {
            index_str = index_str.substr(1);
        }
        index = atol(index_str.c_str());
    }

    uint64_t filesize = 0;
    std::string logfilepath = m_config.logdir + "/" + last_filename;
    if (std::filesystem::exists(logfilepath)) {
        filesize += std::filesystem::file_size(logfilepath);
    }
    if (!m_config.cachedir.empty()) {
        logfilepath = m_config.cachedir + "/" + last_filename;
        if (std::filesystem::exists(logfilepath)) {
            filesize += std::filesystem::file_size(logfilepath);
        }
    }

    return (filesize > m_max_file_size) ? index + 1 : index;
}

void ALogAppender::__MakeLogFileName(const timeval& _tv,
                                     const std::string& _logdir,
                                     const char* _prefix,
                                     const std::string& _fileext,
                                     char* _filepath,
                                     unsigned int _len) {
    long index = 0;
    std::string logFileNamePrefix = __MakeLogFileNamePrefix(_tv, _prefix);
    if (m_max_file_size > 0) {
        index = __GetNextFileIndex(logFileNamePrefix, _fileext);
    }

    std::string logFilePath = _logdir;
    logFilePath += "/";
    logFilePath += logFileNamePrefix;

    if (index > 0) {
        char temp[24] = {0};
        snprintf(temp, 24, "_%ld", index);
        logFilePath += temp;
    }

    logFilePath += ".";
    logFilePath += _fileext;

    strncpy(_filepath, logFilePath.c_str(), _len -1);
    _filepath[_len - 1] = '\0';   // c类型的字符串数组必须以\0结尾
}

void ALogAppender::__GetMarkInfo(char* _info, size_t _info_len) {

    struct timeval tv;
    gettimeofday(&tv, nullptr);

    time_t sec = tv.tv_sec;
    struct tm tm_tmp = *localtime((const time_t*)&sec);
    char tmp_time[64] = {0};
    // 格式化时间字符串
    strftime(tmp_time, sizeof(tmp_time), "%Y-%m-%d %z %H:%M:%S", &tm_tmp);  // 年-月-日 时区 时:分:秒
    snprintf(_info, _info_len, "[%" PRIdMAX ",%" PRIdMAX"][%s]", logger_pid(), logger_tid(), tmp_time);

}

void ALogAppender::__WriteTips2Console(const char* _tips_format, ...) {
    if (nullptr == _tips_format) {
        return;
    }

    char tips_info[4096] = {0};
    va_list ap;
    va_start(ap, _tips_format);
    vsnprintf(tips_info, sizeof(tips_info), _tips_format, ap);
    va_end(ap);

    // 输出到控制台
    LOGD("%s", tips_info);

}

bool ALogAppender::__WriteFile(const void* _data, size_t _len, FILE* _file) {
    if (nullptr == _file) {
        return false;
    }

    long before_len = ftell(_file);
    LOGD("__WriteFile -> before_len:%ld, log len:%zu", before_len, _len);
    if (before_len < 0) {
        return false;
    }

    if (1 != fwrite(_data, _len, 1, _file)) {
        int err = ferror(_file);

        __WriteTips2Console("write file error:%d", err);

        return false;
    }

    return true;
}

bool ALogAppender::__OpenLogFile(const std::string& _log_dir) {
    LOGD("__OpenLogFile -> dir:%s", _log_dir.c_str());
    if (m_config.logdir.empty()) {
        return false;
    }

    // 获取当前时间
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    if (nullptr != m_logfile) {
        //
        time_t sec = tv.tv_sec;
        tm tcur = *localtime((const time_t*)&sec);
        tm fileTm = *localtime(&open_file_time);

        if (fileTm.tm_year == tcur.tm_year
                && fileTm.tm_mon == tcur.tm_mon
                && fileTm.tm_mday == tcur.tm_mday) {
            LOGD("__OpenLogFile -> same file has been opened");
            return true;
        }
        LOGD("__OpenLogFile -> close old file");
        // 非同一天的，则关闭旧的文件
        fclose(m_logfile);
        m_logfile = nullptr;
    }

    uint64_t now_tick = gettickcount();
    time_t now_time = tv.tv_sec;

    open_file_time = tv.tv_sec;

    char logFilePath[file_path_len] = {0};
    __MakeLogFileName(tv, _log_dir, m_config.nameprefix.c_str(), LOG_EXT, logFilePath, file_path_len);

    if (now_time < m_last_time) {
        m_logfile = fopen(m_last_file_path, "ab");
        if (nullptr == m_logfile) {
            __WriteTips2Console("open file error:%d %s, path:%s", errno, strerror(errno), m_last_file_path);
        }
        return nullptr != m_logfile;
    }

    m_logfile = fopen(logFilePath, "ab");

    if (nullptr == m_logfile) {
        __WriteTips2Console("open file error:%d %s, path:%s", errno, strerror(errno), logFilePath);
    }

    if (0 != m_last_time && (now_time - m_last_time) > (time_t)((now_tick - m_last_tick) / 1000 + 300)) {
        struct tm tm_temp = *localtime((const time_t*)&m_last_time);
        char last_time_str[64] = {0};
        strftime(last_time_str, sizeof(last_time_str), "%Y-%m-%d %z %H:%M:%S", &tm_temp);

        tm_temp = *localtime((const time_t*)&now_time);
        char now_time_str[64] = {0};
        strftime(now_time_str, sizeof(last_time_str), "%Y-%m-%d %z %H:%M:%S", &tm_temp);

        char log[1024] = {0};
        snprintf(log, sizeof(log), "[F][ last log file:%s from %s to %s, time_diff:%ld, tick_diff:%" PRIu64 "\n",
                 m_last_file_path, last_time_str, now_time_str, now_time - m_last_time, now_tick - m_last_tick);

        AutoBuffer temp_buff;
        m_log_buff->Write(log, strnlen(log, sizeof(log)), temp_buff);
        __WriteFile(temp_buff.Ptr(), temp_buff.Length(), m_logfile);
    }

    memcpy(m_last_file_path, logFilePath, sizeof(m_last_file_path));
    m_last_tick = now_tick;
    m_last_time = now_time;

    return nullptr != m_logfile;
}


void ALogAppender::__CloseLogFile() {
    LOGD("__CloseLogFile");
    if (nullptr == m_logfile) {
        return;
    }

    // 关闭log文件
    open_file_time = 0;
    fclose(m_logfile);
    m_logfile = nullptr;
}

bool ALogAppender::__CacheLogs() {

    if (m_config.cachedir.empty() || m_config.cache_day <= 0) {
        // 缓存目录为空或者缓存时间为0，则表示不使用缓存
        return false;
    }

    struct timeval tv;
    gettimeofday(&tv, nullptr);
    char logFilePath[file_path_len] = {0};

    __MakeLogFileName(tv, m_config.logdir, m_config.nameprefix.c_str(), LOG_EXT, logFilePath, file_path_len);
    if (std::filesystem::exists(logFilePath)) {
        return false;
    }

    static const uintmax_t kAvailableSizeThreshold = (uintmax_t)1 * 1024 * 1024 * 1024;
    std::filesystem::space_info info = std::filesystem::space(m_config.cachedir);
    if (info.available < kAvailableSizeThreshold) {
        return false;
    }

    return true;
}


void ALogAppender::__Log2File(const void* _data, size_t _len, bool _move_file) {
    LOGD("__Log2File -> log length:%zu, move file:%d", _len, _move_file);
    if (nullptr == _data || 0 == _len) { // 如果数据长度为0，那么就直接返回
        return;
    }

    if (m_config.logdir.empty()) {
        return;
    }

    ScopedLock  lock_file(m_mutex_log_file);

    if (m_config.cachedir.empty()) {
        // 如果没有设置缓存目录，说明是直接写log文件
        if (__OpenLogFile(m_config.logdir)) {  // 打开log文件
            __WriteFile(_data, _len, m_logfile);  // 写数据
            if (kAppenderAsync == m_config.mode) {
                __CloseLogFile();  // 关闭文件，保证数据被实时同步到磁盘中
            }
        }
        return;
    }

    // 获取当前日期，使用日期来生成文件名
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    char logCacheFilePath[file_path_len] = {0};  // 文件名最长是1023字节('\0'占一个字节)
    // 创建log文件名
    __MakeLogFileName(tv, m_config.cachedir, m_config.nameprefix.c_str(), LOG_EXT, logCacheFilePath, file_path_len);
    LOGD("__Log2File -> log cache file path:%s", logCacheFilePath);

    bool cache_logs = __CacheLogs();
    LOGD("__Log2File -> cache_logs:%d", cache_logs);
    if ((cache_logs || std::filesystem::exists(logCacheFilePath)) && __OpenLogFile(m_config.cachedir)) {
        __WriteFile(_data, _len, m_logfile);
        if (kAppenderAsync == m_config.mode) {
            __CloseLogFile();  // 关闭文件，保证数据被实时同步到磁盘中
        }

        if (cache_logs || !_move_file) {
            return;
        }

        char logFilePath[file_path_len] = {0};
        __MakeLogFileName(tv, m_config.cachedir, m_config.nameprefix.c_str(), LOG_EXT, logFilePath, file_path_len);
        LOGD("__Log2File -> log file path:%s", logFilePath);

        if (__AppendFile(logCacheFilePath, logFilePath)) {
            if (kAppenderAsync == m_config.mode) {
                __CloseLogFile();  // 关闭文件，保证数据被实时同步到磁盘中
            }
            std::filesystem::remove(logCacheFilePath);
        }
        return;
    }

    bool write_success = false;
    bool open_success = __OpenLogFile(m_config.logdir);
    if (open_success) {
        write_success = __WriteFile(_data, _len, m_logfile);
        if (kAppenderAsync == m_config.mode) {
            __CloseLogFile();  // 关闭文件，保证数据被实时同步到磁盘中
        }
    }

    if (!write_success) {
        if (open_success && kAppenderSync == m_config.mode) {
            __CloseLogFile();
        }

        if (__OpenLogFile(m_config.cachedir)) {  // 打开log文件
            __WriteFile(_data, _len, m_logfile);  // 写数据
            if (kAppenderAsync == m_config.mode) {
                __CloseLogFile();  // 关闭文件，保证数据被实时同步到磁盘中
            }
        }
    }
}

void ALogAppender::__AsyncLogThread() {
    while (true) {

        ScopedLock lock_buffer(m_mutex_buffer_async);
        if (nullptr == m_log_buff) {
            break;
        }
        AutoBuffer tmp;
        m_log_buff->Flush(tmp);
        LOGD("__AsyncLogThread -> after flush, tmp length:%zu", tmp.Length());
        lock_buffer.unlock();


        if (nullptr != tmp.Ptr()) {
            __Log2File(tmp.Ptr(), tmp.Length(), true);
        }

        if (m_log_close) break;
        m_cond_buffer_async.wait(15 * 60 * 1000);
    }
}

/**
 * log同步写入到文件
 * @param _info
 * @param _log
 */
void ALogAppender::__WriteSync(const LoggerInfo* _info, const char* _log) {

    char temp[16 * 1024] = {0};
    PtrBuffer log(temp, 0, sizeof(temp));
    log_formater(_info, _log, log);
    LOGD("__WriteSync -> log:%s", temp);

    // 先把数据写入到缓冲区
    AutoBuffer tmp_buff;
    if (!m_log_buff->Write(log.Ptr(), log.getLength(), tmp_buff)) {
        return;
    }

    // 再将缓冲区中的log数据写入到文件中
    __Log2File(tmp_buff.Ptr(), tmp_buff.Length(), false);
}

/**
 * 异步写入
 * @param _info
 * @param _log
 */
void ALogAppender::__WriteAsync(const LoggerInfo*_info, const char *_log) {

    char temp[16 * 1024] = {0};
    PtrBuffer log_buff(temp, 0, sizeof(temp));
    log_formater(_info, _log, log_buff);
    LOGD("__WriteAsync -> log:%s", temp);

    ScopedLock lock(m_mutex_buffer_async);  // 加锁，避免多线程同时写buffer，导致log混乱
    if (nullptr == m_log_buff) {
        return;
    }
    if (m_log_buff->GetData().getLength() >= kBufferBlockLength * 5 / 6) {
        int ret = snprintf(temp, sizeof(temp), "[F]sg_buffer_async_Length() >= BUFFER_BLOCK_LENGTH * 4/5, len:%zu\n",
                           m_log_buff->GetData().getLength());
        log_buff.Length(ret, ret);
    }

    LOGD("__WriteAsync -> write to log buf, length:%zu", log_buff.getLength());
    if (!m_log_buff->Write(log_buff.Ptr(), (unsigned int) log_buff.getLength())) {
        return;
    }

    if (m_log_buff->GetData().getLength() >= kBufferBlockLength * 1 / 3
    || (nullptr != _info && kLevelFatal == _info->level)) {
        // buffer长度满了三分之一，也就是缓存的内容超过了50K，就通知异步线程同步写log到文件
        m_cond_buffer_async.notifyAll();
    }
    LOGD("__WriteAsync -> write log finish.");
}

void ALogAppender::__DelTimeoutFile(const std::string& _log_path) {
    LOGD("delete timeout file : %s", _log_path.c_str());

}


bool ALogAppender::__AppendFile(const std::string& _src_file, const std::string& _dst_file) {
    if (_src_file == _dst_file) {
        return false;
    }

    if (!std::filesystem::exists(_src_file)) {
        // 如果src文件不存在，那么就直接返回
        return false;
    }
    if (0 == std::filesystem::file_size(_src_file)) {
        // 如果src文件的长度为0，说明无需用追加内容，返回true
        return true;
    }

    FILE* src_file = fopen(_src_file.c_str(), "rb");
    if (nullptr == src_file) {
        return false;
    }
    FILE* dst_file = fopen(_dst_file.c_str(), "ab");
    if (nullptr == dst_file) {
        fclose(src_file);
        return false;
    }

    fseek(src_file, 0, SEEK_END);
    long src_file_len = ftell(src_file);
    long dst_file_len = ftell(dst_file);
    fseek(src_file, 0, SEEK_SET);

    char buffer[4096] = {0};
    while (true) {
        if (feof(src_file)) {
            break;
        }

        size_t read_ret = fread(buffer, 1, sizeof(buffer), src_file);
        if (0 == read_ret) {
            break;
        }
        if (ferror(src_file)) {
            break;
        }

        fwrite(buffer, 1, read_ret, dst_file);
        if (ferror(dst_file)) {
            break;
        }
    }

    if (dst_file_len + src_file_len > ftell(dst_file)) {
        ftruncate(fileno(dst_file), dst_file_len);
        fclose(src_file);
        fclose(dst_file);
        return false;
    }

    fclose(src_file);
    fclose(dst_file);
    return true;
}

void ALogAppender::__MoveOldFiles(const std::string& _src_path, const std::string& _dst_path,
                                     const std::string& _name_prefix) {

    LOGD("move old file from %s to %s", _src_path.c_str(), _dst_path.c_str());

}

const char* ALogAppender::Dump(const void *_dumpbuffer, size_t _len) {
    return nullptr;
}
