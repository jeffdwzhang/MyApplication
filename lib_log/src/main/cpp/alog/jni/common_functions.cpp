//
// Created by 张德文 on 2021/6/27.
// 定义一些常用的JNI辅助函数和常量
//

#include "common_functions.h"
#include "var_cache.h"

jvalue JNU_CallMethodByName(JNIEnv *env,
                            jobject obj,
                            const char* name,
                            const char* descriptor, ...) {

}

jvalue __JNU_CallMethodByName(JNIEnv *env,
                              jobject obj,
                              const char* name,
                              const char* descriptor) {

    jclass clazz;
    jmethodID mid;
    jvalue result;
    memset(&result, 0, sizeof(result));

    if (env->EnsureLocalCapacity(2) == JNI_OK) {
        clazz = env->GetObjectClass(obj);

    }


}

jvalue JNU_GetStaticFieldByName(JNIEnv* env, jclass clazz, const char* name, const char* sig) {

    jvalue result;
    memset(&result, 0 , sizeof(result));

    if (env->ExceptionOccurred()) {
        return result;
    }

    jfieldID fid = VarCache::Singleton()->GetFieldId(env, clazz, name, sig);

    if (nullptr == fid) {
        return result;
    }

    // 判断第一个字符
    switch (*sig) {
        case '[':
        case 'L':
            result.l = env->GetStaticObjectField(clazz, fid);
            break;
        case 'Z':
            result.z = env->GetStaticBooleanField(clazz, fid);
            break;
        case 'B':
            result.b = env->GetStaticByteField(clazz, fid);
            break;
        case 'C':
            result.c = env->GetStaticCharField(clazz, fid);
            break;
        case 'S':
            result.s = env->GetStaticShortField(clazz, fid);
            break;
        case 'I':
            result.i = env->GetStaticIntField(clazz,fid);
            break;
        case 'J':
            result.j = env->GetStaticLongField(clazz, fid);
            break;
        case 'F':
            result.f = env->GetStaticFloatField(clazz, fid);
            break;
        case 'D':
            result.d = env->GetStaticDoubleField(clazz, fid);
            break;
        default:
            env->FatalError("illegal _descriptor");
            break;
    }

    return result;
}

jvalue JNU_GetField(JNIEnv* env, jobject obj, const char* name, const char* sig) {
    jvalue result;
    memset(&result, 0, sizeof(result));

    if (env->ExceptionOccurred()) {
        return result;
    }

    jclass clazz = env->GetObjectClass(obj);
    jfieldID fid = VarCache::Singleton()->GetFieldId(env, clazz, name, sig);
    env->DeleteLocalRef(clazz);

    if (nullptr == fid) {
        return result;
    }

    // 判断第一个字符
    switch (*sig) {
        case '[':
        case 'L':
            result.l = env->GetObjectField(obj, fid);
            break;
        case 'Z':
            result.z = env->GetBooleanField(obj, fid);
            break;
        case 'B':
            result.b = env->GetByteField(obj, fid);
            break;
        case 'C':
            result.c = env->GetCharField(obj, fid);
            break;
        case 'S':
            result.s = env->GetShortField(obj, fid);
            break;
        case 'I':
            result.i = env->GetIntField(obj,fid);
            break;
        case 'J':
            result.j = env->GetLongField(obj, fid);
            break;
        case 'F':
            result.f = env->GetFloatField(obj, fid);
            break;
        case 'D':
            result.d = env->GetDoubleField(obj, fid);
            break;
        default:
            env->FatalError("illegal _descriptor");
            break;
    }

    return result;
}
