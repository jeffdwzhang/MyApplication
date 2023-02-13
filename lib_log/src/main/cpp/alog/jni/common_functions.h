//
// Created by 张德文 on 2021/6/27.
//

#ifndef ANDROIDAVLEARN_COMMON_FUNCTIONS_H
#define ANDROIDAVLEARN_COMMON_FUNCTIONS_H

#include <jni.h>
#include <string>
#include <map>

struct JniMethodInfo;
class AutoBuffer;

jvalue JNU_CallMethodByName(JNIEnv* _env, jobject obj, const char* _name, const char* descriptor, ...);
jvalue JNU_CallStaticMethodByName(JNIEnv* _env, jclass clazz, const char* _name, const char* descriptor, ...);
jvalue JNU_CallStaticMethodByName(JNIEnv* _env, const char* _class_name, const char* _name, const char* descriptor, ...);
jvalue JNU_GetStaticFieldByName(JNIEnv* _env, jclass clazz, const char* name, const char* sig);
jvalue JNU_GetField(JNIEnv* _env, jobject obj, const char* name, const char* sig);

jvalue JNU_CallMethodByMethodInfo(JNIEnv* _env, jobject obj, JniMethodInfo method_info, ...);

jbyteArray JNU_Buffer2JbyteArray(JNIEnv* _env, AutoBuffer& ab);
jbyteArray JNU_Buffer2JbyteArray(JNIEnv* _env, const void* _buffer, size_t _length);
void JNU_FreeJbyteArray(JNIEnv* _env, jbyteArray bytes);

bool JNU_JbyteArray2Buffer(JNIEnv* _env, const jbyteArray bytes, AutoBuffer& ab);

wchar_t *JNU_Jstring2Wchar(JNIEnv* _env, const jstring jstr);
void JNU_FreeWchar(JNIEnv* _env, const jstring jstr, wchar_t* wchar);

jstring JNU_Chars2Jstring(JNIEnv* _env, const char* pat);
void JNU_FreeJstring(JNIEnv* _env, jstring str);

std::map<std::string, std::string> JNU_JObject2Map(JNIEnv* _env, const jobject _obj);





#endif //ANDROIDAVLEARN_COMMON_FUNCTIONS_H
