//
// Created by 张德文 on 2021/10/31.
//

#ifndef ANDROIDAVLEARN_SCOPEDJSTRING_H
#define ANDROIDAVLEARN_SCOPEDJSTRING_H

#include <jni.h>

class ScopedJstring {
public:
    ScopedJstring(JNIEnv* _env, jstring _jstr);
    ScopedJstring(JNIEnv* _env, const char* _char);

    ~ScopedJstring();

    const char* GetChar() const;
    const char* SafeGetChar() const;
    jstring GetJstr() const;

private:
    ScopedJstring();
    ScopedJstring(const ScopedJstring&);
    ScopedJstring& operator=(const ScopedJstring&);

    JNIEnv* m_env;
    jstring m_str;
    const char* m_char;
    bool m_jstr2char;

};


#endif //ANDROIDAVLEARN_SCOPEDJSTRING_H
