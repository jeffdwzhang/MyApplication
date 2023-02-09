//
// Created by 张德文 on 2022/10/30.
//

#ifndef MYAPPLICATION_COMPILER_UTIL_H
#define MYAPPLICATION_COMPILER_UTIL_H

#if defined(__GNUC__)
#define WEAK_FUNC     __attribute__((weak))
#elif defined(_MSC_VER) && !defined(_LIB)
#define WEAK_FUNC __declspec(selectany)
#else
#define WEAK_FUNC
#endif

#if defined(__GNUC__)
#define EXPORT_FUNC __attribute__ ((visibility ("default")))
#elif defined(_MSC_VER)
#define EXPORT_FUNC __declspec(dllexport)
#else
#error "export"
#endif

#if defined(_MSC_VER) && defined(MARS_USE_DLLS)
#ifdef MARS_COMMON_EXPORTS
#define MARS_COMMON_EXPORT __declspec(dllexport)
#else
#define MARS_COMMON_EXPORT __declspec(dllimport)
#endif
#else
#define MARS_COMMON_EXPORT
#endif


#ifndef VARIABLE_IS_NOT_USED
#ifdef __GNUC__
#define VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#define VARIABLE_IS_NOT_USED
#endif
#endif

#endif //MYAPPLICATION_COMPILER_UTIL_H
