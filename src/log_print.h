//log.h
#ifndef LOG_H
#define LOG_H

#ifdef ENABLE_LINUX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

inline void log_print(const void *tag, const char *fmt, ...)
{
    printf("[%s] ", (char *) tag);
    va_list vl;
    va_start(vl, fmt);
    vprintf(fmt, vl);
    va_end(vl);
}

#endif

#ifdef ENABLE_ANDROID
#include <android/log.h>
#define log_print(p1,...)    __android_log_print(ANDROID_LOG_INFO,p1,__VA_ARGS__)
#endif

#endif
