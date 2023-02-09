//
// Created by 张德文 on 2021/4/2.
//

#include "jni_bridge.h"
#include "LogUtil.h"

static JavaVM *sVM;

// 动态注册ALog模块的native函数
extern int register_ALog_NativeMethods(JNIEnv* env);

int jniThrowException(JNIEnv *env, const char* className, const char* msg) {
    jclass exceptionClass = env->FindClass(className);
    if (exceptionClass == nullptr) {
        return -1;
    }

    if (env -> ThrowNew(exceptionClass, msg) != JNI_OK) {
        return -1;
    }
    return 0;
}

JNIEnv* getJNIEnv() {

    JNIEnv* env = nullptr;
    if (sVM->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        return nullptr;
    }
    return env;

}


int jniRegisterNativeMethods(JNIEnv* env, const char* className,
                             const JNINativeMethod* gMethods, int numMethods)
{
    LOGI("Registering %s Native Methods\n", className);
    jclass clazz = (env)->FindClass( className);
    if (clazz == nullptr) {
        LOGE("Native registration unable to find class '%s'\n", className);
        return -1;
    }

    int result = 0;
    if ((env)->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'\n", className);
        result = -1;
    }

    (env)->DeleteLocalRef(clazz);

    return result;
}


/**
 * JNI_OnLoad是So中调用的第一个函数，在so加载到虚拟机中时被调用
 *
 * 动态注册一般都是在JNI_OnLoad函数中完成
 */
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{

    JNIEnv *pEnv = nullptr;
    jint result = -1;
    if (vm->GetEnv((void**)&pEnv, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }

    // 注册log相关的native方法
    register_ALog_NativeMethods(pEnv);

    return JNI_VERSION_1_6;
}




