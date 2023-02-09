# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)
# Declares and names the project.
project (alog)

# 设置lib库的名称
set(SELF_LIB_NAME alog)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/jni
        ${CMAKE_CURRENT_SOURCE_DIR}/alog
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/android
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/buffers
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/crypt
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/crypt/micro-ecc
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/io
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/thread
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/jni)

file(GLOB SELF_SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/crypt/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/android/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/buffers/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/crypt/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/crypt/micro-ecc/uECC.c
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/io/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/common/utils/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/alog/jni/*.cpp
        )

add_library(${SELF_LIB_NAME}
        SHARED
        ${SELF_SRC_FILES})

find_library(log-lib log)
find_library(zlib-lib z)
target_link_libraries(${SELF_LIB_NAME}
        ${log-lib}
        ${zlib-lib})
