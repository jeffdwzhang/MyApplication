//
// Created by 张德文 on 2021/6/24.
//

#include "alog_interface.h"
#include "common_functions.h"
#include "ScopedJstring.h"
#include "ALogCategory.h"
#include "jni_bridge.h"

#define LOG_TAG "ALogModule"
#include "LogUtil.h"

/**
 * 新建一个native层log输出对象，并返回对应的指针
 *
 * @param env
 * @param obj
 * @param _log_config
 * @return
 */
jlong newLogInstance(JNIEnv *env, jobject obj, jobject _log_config) {
    if (nullptr == _log_config) {
        return -1;
    }

    // 获取对应的配置参数
    jint level = JNU_GetField(env, _log_config, "level", "I").i;
    jint mode = JNU_GetField(env, _log_config, "mode", "I").i;
    jstring logdir = (jstring)JNU_GetField(env, _log_config, "logDir", "Ljava/lang/String;").l;
    jstring nameprefix = (jstring)JNU_GetField(env, _log_config, "namePrefix", "Ljava/lang/String;").l;
    jstring pubkey = (jstring)JNU_GetField(env, _log_config, "pubKey", "Ljava/lang/String;").l;
    jint compressmode = JNU_GetField(env, _log_config, "compressMode", "I").i;
    jint compresslevel = JNU_GetField(env, _log_config, "compressLevel", "I").i;
    jstring cachedir = (jstring)JNU_GetField(env, _log_config, "cacheDir", "Ljava/lang/String;").l;
    jint cachedays = JNU_GetField(env, _log_config, "cacheDays", "I").i;

    std::string cachedir_str;
    if (nullptr != cachedir) {
        ScopedJstring cache_dir_jstr(env, cachedir);
        cachedir_str = cache_dir_jstr.GetChar();
    }

    std::string pubkey_str;
    if (nullptr != pubkey) {
        ScopedJstring pubkey_jstr(env, pubkey);
        pubkey_str = pubkey_jstr.GetChar();
    }

    std::string logdir_str;
    if (nullptr != logdir) {
        ScopedJstring logdir_jstr(env, logdir);
        logdir_str = logdir_jstr.GetChar();
    }

    std::string nameprefix_str;
    if (nullptr != nameprefix) {
        ScopedJstring nameprefix_jstr(env, nameprefix);
        nameprefix_str = nameprefix_jstr.GetChar();
    }

    ALogConfig config = {(AppenderMode)mode, logdir_str, cachedir_str, nameprefix_str, pubkey_str, (CompressMode)compressmode, compresslevel, cachedays};
    ALogCategory* category = NewLoggerInstance(config, (ALogLevel)level);
    if (nullptr == category) {
        return -1;
    }

    // 返回对应的实例指针；
    return reinterpret_cast<intptr_t>(category);
}

/**
 * 打开对应的log实例，
 * 实例对象的指针保存在native全局变量中
 *
 * @param env
 * @param obj
 * @param _log_config
 */
void appenderOpen(JNIEnv *env, jobject obj, jobject _log_config) {
    if (nullptr == _log_config) {
        return;
    }
    // 解析log配置参数
    jint level = JNU_GetField(env, _log_config, "level", "I").i;
    jint mode = JNU_GetField(env, _log_config, "mode", "I").i;
    jstring logdir = (jstring)JNU_GetField(env, _log_config, "logDir", "Ljava/lang/String;").l;
    jstring nameprefix = (jstring)JNU_GetField(env, _log_config, "namePrefix", "Ljava/lang/String;").l;
    jstring pubkey = (jstring)JNU_GetField(env, _log_config, "pubKey", "Ljava/lang/String;").l;
    jint compressmode = JNU_GetField(env, _log_config, "compressMode", "I").i;
    jint compresslevel = JNU_GetField(env, _log_config, "compressLevel", "I").i;
    jstring cachedir = (jstring)JNU_GetField(env, _log_config, "cacheDir", "Ljava/lang/String;").l;
    jint cachedays = JNU_GetField(env, _log_config, "cacheDays", "I").i;

    std::string cachedir_str;
    if (nullptr != cachedir) {
        ScopedJstring cache_dir_jstr(env, cachedir);
        cachedir_str = cache_dir_jstr.GetChar();
    }

    std::string pubkey_str;
    if (nullptr != pubkey) {
        ScopedJstring pubkey_jstr(env, pubkey);
        pubkey_str = pubkey_jstr.GetChar();
    }

    std::string logdir_str;
    if (nullptr != logdir) {
        ScopedJstring logdir_jstr(env, logdir);
        logdir_str = logdir_jstr.GetChar();
    }

    std::string nameprefix_str;
    if (nullptr != nameprefix) {
        ScopedJstring nameprefix_jstr(env, nameprefix);
        nameprefix_str = nameprefix_jstr.GetChar();
    }

    LOGI("appenderOpen -> level:%d, logdir_str:%s, nameprefix_str:%s, cachedays:%d", level, logdir_str.c_str(), nameprefix_str.c_str(), cachedays);

    ALogConfig config = {(AppenderMode)mode, logdir_str, cachedir_str, nameprefix_str, pubkey_str,
                         (CompressMode)compressmode, compresslevel, cachedays};

    appender_open(config, (ALogLevel)level);

}


void appenderClose(JNIEnv *env, jobject obj) {
    appender_close();
}


void appenderFlush(JNIEnv *env, jobject obj, jlong _log_instance_ptr, jboolean isSync) {
    appender_flush();
}

/**
 * 写log
 * @param env
 * @param obj
 * @param _log_instance_ptr
 * @param _level
 * @param _tag
 * @param _line
 * @param _pid
 * @param _tid
 * @param _mainid
 * @param _log
 */
void logWrite(JNIEnv *env, jobject obj, jlong _log_instance_ptr, jint _level, jstring _tag, jint _pid, jlong _tid, jlong _mainid, jstring _log) {

    LoggerInfo info;
    memset(&info, 0, sizeof(info));
    gettimeofday(&info.timeval, nullptr);
    info.level = (ALogLevel)_level;
    info.pid = _pid;
    info.tid = _tid;
    info.maintid = _mainid;

    const char* tag_cstr = nullptr;
    const char* log_cstr = nullptr;

    if (nullptr != _tag) {
        tag_cstr = env->GetStringUTFChars(_tag, nullptr);
    }

    if (nullptr != _log) {
        log_cstr = env->GetStringUTFChars(_log, nullptr);
    }

    info.tag = nullptr == tag_cstr ? "" : tag_cstr;

    // 输出log
    LoggerWrite(_log_instance_ptr, &info, log_cstr);

    if (nullptr != _tag) {
        env->ReleaseStringUTFChars(_tag, tag_cstr);
    }

    if (nullptr != _log) {
        env->ReleaseStringUTFChars(_log, log_cstr);
    }
}


/**
 * 获取底层设置的log级别
 * @param env
 * @param obj
 * @param _log_instance_ptr
 * @return
 */
jint getLogLevel(JNIEnv *env, jobject obj, jlong _log_instance_ptr) {
    return GetLevel(_log_instance_ptr);
}

/**
 * 设置log模式
 * @param env
 * @param obj
 * @param _log_instance_ptr
 * @param _mode
 */
void setAppenderMode(JNIEnv *env, jobject obj, jlong _log_instance_ptr, jint _mode) {
    SetAppenderMode(_log_instance_ptr, (AppenderMode)_mode);
}

/**
 *
 * @param env
 * @param obj
 * @param _log_instance_ptr
 * @param _is_open
 */
void setConsoleLogOpen(JNIEnv *env, jobject obj, jlong _log_instance_ptr, bool _is_open) {
    SetConsoleLogOpen(_log_instance_ptr, _is_open);
}

/**
 * 设置log文件最大size，超过指定的size的log会被清理
 * @param env
 * @param obj
 * @param _log_instance_ptr
 * @param _max_size
 */
void setMaxFileSize(JNIEnv *env, jobject obj, jlong _log_instance_ptr, long _max_size) {
    SetMaxFileSize(_log_instance_ptr, _max_size);
}

/**
 * 设置log的最长保留时间，过期的log文件会被删除
 * @param env
 * @param obj
 * @param _log_instance_ptr
 * @param _alive_time
 */
void setMaxAliveTime(JNIEnv *env, jobject obj, jlong _log_instance_ptr, long _alive_time) {
    SetMaxAliveTime(_log_instance_ptr, _alive_time);
}

//----------------------------------------------------
// Java类名称
static const char *className = "org/zy/alog/ALog";

// Java方法与native函数的对应表
static JNINativeMethod gJni_Methods_table[] = {
        {"appenderOpen", "(Lorg/zy/alog/ALog$ALogConfig;)V", (void*)appenderOpen},
        {"appenderClose", "()V", (void*)appenderClose},
        {"appenderFlush", "(JZ)V", (void*)appenderFlush},
        {"logWrite", "(JILjava/lang/String;IJJLjava/lang/String;)V", (void*)logWrite},
        {"getLogLevel", "(J)I", (void*)getLogLevel},
        {"setConsoleLogOpen", "(JZ)V", (void*)setConsoleLogOpen},
        {"setMaxFileSize", "(JJ)V", (void*)setMaxFileSize},
        {"setMaxAliveTime", "(JJ)V", (void*)setMaxAliveTime}
};

// 动态注册对应的方法
int register_ALog_NativeMethods(JNIEnv* env){
    return jniRegisterNativeMethods(env,
                                    className,
                                    gJni_Methods_table,
                                    sizeof(gJni_Methods_table)/sizeof(gJni_Methods_table[0]));
}