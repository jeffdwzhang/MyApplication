//
// Created by 张德文 on 2021/7/4.
//

#include <unistd.h>
#include <stddef.h>
#include <pthread.h>
#include "scope_jenv.h"
#include "var_cache.h"

ScopeJniEnv::ScopeJniEnv(JavaVM *jvm, jint capacity)
: m_env(nullptr),  m_status(0){
    if (nullptr == jvm) {
        jvm = VarCache::Singleton()->GetJvm();
    }

    do {
        m_status = jvm->GetEnv((void**)&m_env, JNI_VERSION_1_6);

        if (JNI_OK == m_status) {
            break;
        }

        char thread_name[32] = {0};
        snprintf(thread_name, sizeof(thread_name), "alog::%d", (int)gettid());
        JavaVMAttachArgs args;
        args.group = nullptr;
        args.name = thread_name;
        args.version = JNI_VERSION_1_6;
        m_status = jvm->AttachCurrentThread(&m_env, &args);

        if (JNI_OK == m_status) {
            thread_local struct OnExit {
                ~OnExit() {
                    if (nullptr != VarCache::Singleton()->GetJvm()) {
                        VarCache::Singleton()->GetJvm()->DetachCurrentThread();
                    }
                }
            } dummy;
        } else {
            m_env = nullptr;
            return;
        }
    } while (false);

    jint ret = m_env->PushLocalFrame(capacity);

}

ScopeJniEnv::~ScopeJniEnv() {
    if (nullptr != m_env) {
        m_env->PopLocalFrame(nullptr);
    }
}

JNIEnv* ScopeJniEnv::GetEnv() {
    return m_env;
}

int ScopeJniEnv::Status() {
    return m_status;
}