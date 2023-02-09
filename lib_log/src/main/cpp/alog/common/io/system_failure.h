//
// Created by 张德文 on 2023/2/8.
//

#ifndef MYAPPLICATION_SYSTEM_FAILURE_H
#define MYAPPLICATION_SYSTEM_FAILURE_H

#include <errno.h>

template<class E> __attribute__ ((__noreturn__)) inline void throw_exception(E const& e) {
    throw e;
}

class failure : public std::exception {
public:
    explicit failure(const std::string& what_arg) : what_(what_arg) { }
    virtual ~failure()  {}
    virtual const char* what() const throw() { return what_.c_str(); };
private:
    std::string what_;
};

inline failure system_failure(const char* msg) {
    std::string result;
    result += msg;
    const char* system_msg = errno ? strerror(errno) : "";
    result.reserve(std::strlen(msg) + 2 + std::strlen(system_msg));
    result.append(msg);
    result.append(": ");
    result.append(system_msg);
    return failure(result);
}

inline failure system_failure(const std::string& msg) {
    return system_failure(msg.c_str());
}

inline void throw_system_failure(const std::string& msg) {
    throw_exception(system_failure(msg));
}

#endif //MYAPPLICATION_SYSTEM_FAILURE_H
