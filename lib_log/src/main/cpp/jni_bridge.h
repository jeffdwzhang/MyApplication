//
// Created by 张德文 on 2021/6/24.
//

#ifndef _JNI_BRIDGE_H
#define _JNI_BRIDGE_H

#include <jni.h>

int jniThrowException(JNIEnv *env, const char* className, const char* msg);
JNIEnv* getJNIEnv();

/**
 * jni动态注册函数的包装
 * @param env
 * @param className
 * @param gMethods
 * @param numMethods
 * @return
 */
int jniRegisterNativeMethods(JNIEnv* env, const char* className,
                             const JNINativeMethod* gMethods, int numMethods);

#endif // _JNI_BRIDGE_H
