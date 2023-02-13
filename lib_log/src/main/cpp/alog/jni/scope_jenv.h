//
// Created by 张德文 on 2021/7/4.
//

#ifndef ANDROIDAVLEARN_SCOPE_JENV_H
#define ANDROIDAVLEARN_SCOPE_JENV_H

#include <jni.h>

class ScopeJniEnv {

public:
    ScopeJniEnv(JavaVM* jvm, jint capacity = 16);
    ~ScopeJniEnv();

    JNIEnv* GetEnv();
    int Status();

private:
    JNIEnv* m_env;
    int m_status;
};

#endif //ANDROIDAVLEARN_SCOPE_JENV_H
