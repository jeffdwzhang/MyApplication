//
// Created by 张德文 on 2021/7/4.
//

#ifndef ANDROIDAVLEARN_VAR_CACHE_H
#define ANDROIDAVLEARN_VAR_CACHE_H

#include <jni.h>
#include <string>
#include <map>
#include <list>

struct method_struct {
    std::string methodName;
    std::string signature;
    jmethodID mid;
};

struct field_struct {
    std::string fieldName;
    std::string signature;
    jfieldID fid;
};

struct JniMethodInfo {
    std::string clazzname;
    std::string methodname;
    std::string methodsig;

    JniMethodInfo(const std::string& _clazzname, const std::string& _methodname, const std::string& _methodsig)
        : clazzname(_clazzname), methodname(_methodname), methodsig(_methodsig){

    }

    bool operator <(const JniMethodInfo& _info) const {
        if (clazzname < _info.clazzname) {
            return true;
        }

        if (clazzname == _info.clazzname && methodname < _info.methodname) {
            return true;
        }

        if (clazzname == _info.clazzname && methodname == _info.methodname
                && methodsig < _info.methodsig) {
            return true;
        }

        return false;
    }

};

class VarCache {
public:
    static VarCache* Singleton();
    static void Release();

    ~VarCache();

    JavaVM* GetJvm();
    void SetJvm(JavaVM* vm);

    jclass GetClass(JNIEnv* _env, const char* const _clazzname);
    void CacheClass(const char* const _clazzpath, jclass _clz);

    jmethodID GetStaticMethodId(JNIEnv* _env,
                                const char* const _clzname,
                                const char* const _methodname,
                                const char* const _signature);

    jmethodID GetStaticMethodId(JNIEnv* _env,
                                jclass _clz,
                                const char* const _methodname,
                                const char* const _signature);

    jmethodID GetMethodId(JNIEnv* _env,
                          const char* const _clzname,
                          const char* const _methodname,
                          const char* const _signature);

    jmethodID GetMethodId(JNIEnv* _env,
                          jclass _clz,
                          const char* const _methodname,
                          const char* const _signature);

    jfieldID GetStaticFieldId(JNIEnv* _env,
                              const char* const _clzname,
                              const char* const _fieldname,
                              const char* const _signature);

    jfieldID GetStaticFieldId(JNIEnv* _env,
                              jclass _clz,
                              const char* const _fieldname,
                              const char* const _signature);

    jfieldID GetFieldId(JNIEnv* _env,
                        const char* const _clzname,
                        const char* const _fieldname,
                        const char* const _signature);

    jfieldID GetFieldId(JNIEnv* _env,
                        jclass _clz,
                        const char* const _fieldname,
                        const char* const _signature);

private:
    VarCache();

    static VarCache* m_instance;

    JavaVM* m_vm;

    std::map<std::string, jclass> m_clazz_map;
    std::map<jclass, std::list<method_struct> > m_static_method_map;
    std::map<jclass, std::list<method_struct> > m_method_map;
    std::map<jclass, std::list<field_struct> > m_field_map;
};

#ifdef __GNUC__
#define VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#define VARIABLE_IS_NOT_USED
#endif

bool LoadClass(JNIEnv* _env);
bool AddClass(const char* const _clazzpath);

#define DEFINE_FIND_CLASS(clazzname, clazzpath) \
    VARIABLE_IS_NOT_USED static bool b_##clazzname = AddClass(clazzpath); \
    VARIABLE_IS_NOT_USED static const char* clazzname = clazzpath;

bool LoadStaticMethod(JNIEnv* env);
bool AddStaticMethod(const char* const _clazzname, const char* const _methodname, const char* const _signature);

bool LoadMethod(JNIEnv* _env);
bool AddMethod(const char* const _clazzname, const char* const _methodname, const char* const _signature);

#define DEFINE_FIND_STATIC_METHOD(methodid, clazzname, methodname, signature) \
     VARIABLE_IS_NOT_USED static bool b_static_##methodid = AddStaticMethod(clazzname, methodname, signature); \
     VARIABLE_IS_NOT_USED const static JniMethodInfo methodid = JniMethodInfo(clazzname, methodname, signature);
#define DEFINE_FIND_METHOD(methodid, clazzname, methodname, signature) \
     VARIABLE_IS_NOT_USED static bool b_##methodid = AddMethod(clazzname, methodname, signature); \
     VARIABLE_IS_NOT_USED const static JniMethodInfo methodid = JniMethodInfo(clazzname, methodname, signature);


#endif //ANDROIDAVLEARN_VAR_CACHE_H
