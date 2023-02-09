//
// Created by 张德文 on 2021/6/27.
//

#ifndef ANDROIDAVLEARN_COMMON_FUNCTIONS_H
#define ANDROIDAVLEARN_COMMON_FUNCTIONS_H

#include <jni.h>
#include <string>
#include <map>

jvalue JNU_CallMethodByName(JNIEnv* _env, jobject obj, const char* _name, const char* descriptor, ...);
jvalue JNU_CallStaticMethodByName(JNIEnv* _env, jclass clazz, const char* _name, const char* descriptor, ...);
jvalue JNU_CallStaticMethodByName(JNIEnv* _env, const char* _class_name, const char* _name, const char* descriptor, ...);
jvalue JNU_GetStaticFieldByName(JNIEnv* env, jclass clazz, const char* name, const char* sig);
jvalue JNU_GetField(JNIEnv* env, jobject obj, const char* name, const char* sig);





#endif //ANDROIDAVLEARN_COMMON_FUNCTIONS_H
