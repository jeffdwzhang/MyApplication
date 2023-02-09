//
// Created by 张德文 on 2021/7/4.
//

#include "scope_jenv.h"

ScopeJniEnv::ScopeJniEnv(JavaVM *jvm, jint capacity)
: vm(jvm), env(nullptr), we_attach(false), status(0){

    do {

    } while (false);

}

ScopeJniEnv::~ScopeJniEnv() {
    if (nullptr != env) {
        env->PopLocalFrame(nullptr);
    }
}

JNIEnv* ScopeJniEnv::GetEnv() {
    return env;
}

int ScopeJniEnv::Status() {
    return status;
}