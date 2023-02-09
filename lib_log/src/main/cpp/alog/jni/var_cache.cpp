//
// Created by 张德文 on 2021/7/4.
//

#include "var_cache.h"
#include "scope_jenv.h"
#include <cstdlib>
#include <android/log.h>
#include <set>

template <typename T> T& getListByClass(JNIEnv* _env, const jclass& _clz, std::map<jclass, T>& _map) {
    for (typename std::map<jclass, T>::iterator iter = _map.begin(); iter != _map.end(); ++iter) {
        if (_env->IsSameObject(_clz, (*iter).first))
            return (*iter).second;
    }

    jclass gloableClz = (jclass)_env->NewGlobalRef(_clz);
    std::pair<typename std::map<jclass, T>::iterator, bool> retPair = _map.insert(std::pair<jclass, T>(gloableClz, T()));
    ASSERT(retPair.second);

    return retPair.first->second;
}

VarCache* VarCache::m_instance = nullptr;

VarCache::VarCache(): m_vm(nullptr) {
}

VarCache::~VarCache() {
    ScopeJniEnv scopeJniEnv(m_vm);
    JNIEnv* env = scopeJniEnv.GetEnv();

    // 移除全部的全局引用
    for (std::map<std::string, jclass>::iterator iter = m_clazz_map.begin();
         iter != m_clazz_map.end(); ++iter) {
        env->DeleteGlobalRef(iter->second);
    }
}

VarCache* VarCache::Singleton() {
    if (nullptr == m_instance) {
        m_instance = new VarCache();
    }
    return m_instance;
}

void VarCache::Release() {
    if (m_instance != nullptr) {
        delete m_instance;
        m_instance = nullptr;
    }
}

JavaVM* VarCache::GetJvm() {
    return m_vm;
}

void VarCache::SetJvm(JavaVM *_vm) {
    m_vm = _vm;
}

jclass VarCache::GetClass(JNIEnv* _env, const char* const _clazzpath) {

    if (nullptr == _env || nullptr == _clazzpath) {
        return nullptr;
    }

    if (_env->ExceptionOccurred()) {
        return nullptr;
    }

    std::map<std::string, jclass>::iterator iter = m_clazz_map.find(_clazzpath);
    if (iter != m_clazz_map.end()) {
        return iter->second;
    }

    // 根据类路径名获取类信息
    jclass clz = _env->FindClass(_clazzpath);

    if (clz == nullptr) {
        if (_env->ExceptionOccurred()) {
            _env->ExceptionClear();
            char err_msg[512] = {0};
            snprintf(err_msg, sizeof(err_msg), "classPath:%s", _clazzpath);
            _env->ThrowNew(_env->FindClass("java/lang/UnsatisfiedLinkError"), err_msg);
        }
        return nullptr;
    }

    // 缓存class的全局引用
    jclass global_clz = (jclass) _env->NewGlobalRef(clz);
    _env->DeleteLocalRef(clz);
    m_clazz_map.insert(std::pair<std::string, jclass>(_clazzpath, global_clz));
    return global_clz;
}

void VarCache::CacheClass(const char* const _clazzpath, jclass _clz) {
    if (_clazzpath == nullptr || _clz == nullptr) {
        return;
    }

    std::map<std::string, jclass>::iterator iter = m_clazz_map.find(_clazzpath);
    if (iter == m_clazz_map.end()) {
        m_clazz_map.insert(std::pair<std::string, jclass>(_clazzpath, _clz));
    }
    return;
}

/**
 * 获取静态方法Id
 * @param _env
 * @param _clz
 * @param _methodname
 * @param _signature
 * @return
 */
jmethodID VarCache::GetStaticMethodId(JNIEnv *_env, jclass _clz, const char *const _methodname,
                                      const char *const _signature) {
    if (_env->ExceptionOccurred()) {
        return nullptr;
    }

    if (nullptr == _clz) {
        return nullptr;
    }

#ifdef USE_JNI_METHOD_CACHE
    std::list<method_struct>& methodStructList = getListByClass(_env, _clz, staticMethodMap);
#endif

    jmethodID mid = _env->GetStaticMethodID(_clz, _methodname, _signature);

#ifdef USE_JNI_METHOD_CACHE
    if (nullptr != mid) {
        method_struct methodStruct;
        methodStruct.methodName.assign(_methodname);
        methodStruct.signature.assign(_signature);
        methodStruct.mid = mid;
        methodStructList.push_back(methodStruct);

    } else
#endif
    {
        if (_env->ExceptionOccurred()) {
            _env->ExceptionClear();
            char err_msg[512] = {0};
            snprintf(err_msg, sizeof(err_msg), "method:%s, sig:%s", _methodname, _signature);
            _env->ThrowNew(_env->FindClass("java/lang/UnsatisfiedLinkError"), err_msg);
        }
    }

    return mid;

}

jmethodID VarCache::GetStaticMethodId(JNIEnv *_env, const char *const _clzname,
                                      const char *const _methodname,
                                      const char *const _signature) {
    jclass clazz = GetClass(_env, _clzname);
    return GetStaticMethodId(_env, clazz, _methodname, _signature);
}

jmethodID VarCache::GetMethodId(JNIEnv *_env, jclass _clz, const char *const _methodname,
                                const char *const _signature) {

    if (_env->ExceptionOccurred()) {
        return nullptr;
    }

    if (nullptr == _clz) {
        return nullptr;
    }

#ifdef USE_JNI_METHOD_CACHE

#endif

    jmethodID mid = _env->GetMethodID(_clz, _methodname, _signature);

#ifdef USE_JNI_METHOD_CACHE

#endif
    {
        if (_env->ExceptionOccurred()) {
            _env->ExceptionClear();
            char err_msg[512] = {0};
            snprintf(err_msg, sizeof(err_msg), "method:%s, sig:%s", _methodname, _signature);
            _env->ThrowNew(_env->FindClass("java/lang/UnsatisfiedLinkError"), err_msg);
        }
    }

    return mid;
}

jmethodID VarCache::GetMethodId(JNIEnv *_env, const char *const _clzname,
                                const char *const _methodname, const char *const _signature) {
    jclass clazz = GetClass(_env, _clzname);

    return GetMethodId(_env, clazz, _methodname, _signature);
}

jfieldID VarCache::GetStaticFieldId(JNIEnv *_env, jclass _clz, const char *const _field_name,
                                    const char *const _signature) {

    if (_env->ExceptionOccurred()) {
        return nullptr;
    }

    if (nullptr == _clz) {
        return nullptr;
    }

    jfieldID fid = _env->GetStaticFieldID(_clz, _field_name, _signature);

    return fid;

}

jfieldID VarCache::GetStaticFieldId(JNIEnv *_env, const char *const _clzname,
                                     const char *const _fieldname, const char *const _signature) {
    jclass clazz = GetClass(_env, _clzname);
    return GetStaticFieldId(_env, clazz, _fieldname, _signature);
}

jfieldID VarCache::GetFieldId(JNIEnv *_env, jclass _clz, const char *const _field_name,
                              const char *const _signature) {

    if (_env->ExceptionOccurred()) {
        return nullptr;
    }

    if (nullptr == _clz) {
        return nullptr;
    }
    jfieldID fId = _env->GetFieldID(_clz, _field_name, _signature);

    return fId;
}

jfieldID VarCache::GetFieldId(JNIEnv *_env, const char *const _clzname,
                              const char *const _field_name, const char *const _signature) {

    jclass clazz = GetClass(_env, _clzname);
    return GetFieldId(_env, clazz, _field_name, _signature);
}


static std::set<std::string>& __GetClassNameSet() {
    static std::set<std::string> clazz_name_set;
    return clazz_name_set;
}


bool LoadClass(JNIEnv* env) {
    std::set<std::string>& clazz_name_set = __GetClassNameSet();
    for (std::set<std::string>::iterator it = clazz_name_set.begin(); it != clazz_name_set.end(); ++it) {
        jclass clz = VarCache::Singleton()->GetClass(env, (*it).c_str());
        if (nullptr == clz) {
            clazz_name_set.clear();
            return false;
        }
    }
    clazz_name_set.clear();
    return true;
}

bool AddClass(const char* const clazz_path) {
    std::set<std::string>& clazz_name_set = __GetClassNameSet();
    return clazz_name_set.insert(clazz_path).second;
}

static std::set<JniMethodInfo>& __GetStaticMethodInfoSet() {
    static std::set<JniMethodInfo> method_info_set;
    return method_info_set;
}

static std::set<JniMethodInfo>& __GetMethodInfoSet() {
    static std::set<JniMethodInfo> methodInfoSet;
    return methodInfoSet;
}


