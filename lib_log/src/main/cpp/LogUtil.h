//
// Created by 张德文 on 2021/3/13.
//

#ifndef _LOGUTIL_H
#define _LOGUTIL_H

#include <android/log.h>


#ifndef LOG_TAG
#define LOG_TAG "alog_jni"
#endif

// Log输出宏定义
#define LOGV(format, args...)  __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, format, ##args)
#define LOGD(format, args...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, format, ##args)
#define LOGI(format, args...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, format, ##args)
#define LOGW(format, args...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, format, ##args)
#define LOGE(format, args...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, format, ##args)


#endif //_LOGUTIL_H
