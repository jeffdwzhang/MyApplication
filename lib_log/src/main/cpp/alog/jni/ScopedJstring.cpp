//
// Created by 张德文 on 2021/10/31.
//

#include <cstring>
#include "ScopedJstring.h"

ScopedJstring::ScopedJstring(JNIEnv *_env, jstring _jstr)
: m_env(_env), m_str((jstring)_env->NewLocalRef(_jstr)),m_char(nullptr),m_jstr2char(true){
    if (_env == nullptr || _jstr == nullptr) {
        return;
    }
    if (_env->ExceptionOccurred()) {
        return;
    }
    m_char = m_env->GetStringUTFChars(m_str, nullptr);
}

ScopedJstring::ScopedJstring(JNIEnv *_env, const char *_char)
: m_env(_env), m_str(nullptr), m_char(_char), m_jstr2char(false){
    if (m_env == nullptr || m_char == nullptr) {
        return;
    }
    if (m_env -> ExceptionOccurred()) {
        return;
    }
    // 获取Java String的相关函数
    jclass strClazz = m_env->FindClass("java/lang/String");
    jmethodID ctorId = m_env->GetMethodID(strClazz, "<init>", "([BLjava/lang/String;)V");
    jbyteArray bytes = m_env->NewByteArray((jsize)strlen(m_char));
    m_env->SetByteArrayRegion(bytes, 0, (jsize)strlen(m_char), (jbyte*)m_char);
    jstring encoding = m_env->NewStringUTF("utf-8");
    m_str = (jstring)m_env->NewObject(strClazz, ctorId, bytes, encoding);

    m_env->DeleteLocalRef(bytes);
    m_env->DeleteLocalRef(encoding);
    m_env->DeleteLocalRef(strClazz);

}

ScopedJstring::~ScopedJstring() {
    if (m_env == nullptr || m_str == nullptr || m_char == nullptr) {
        return;
    }
    if (m_env->ExceptionOccurred()) {
        return;
    }
    if (m_jstr2char) {
        m_env->ReleaseStringUTFChars(m_str, m_char);
    }
    m_env->DeleteLocalRef(m_str);
}

const char * ScopedJstring::GetChar() const {
    if (m_env -> ExceptionOccurred()) {
        return nullptr;
    }
    return m_char;
}

const char* ScopedJstring::SafeGetChar() const {
    const char * realStr = GetChar();
    return nullptr == realStr ? "" : realStr;
}

jstring ScopedJstring::GetJstr() const {
    if (m_env->ExceptionOccurred()) {
        return nullptr;
    }
    return m_str;
}

ScopedJstring& ScopedJstring::operator=(const ScopedJstring&) {

}
