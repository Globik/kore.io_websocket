#ifndef _MACROS_H
#define _MACROS_H

#include <errno.h>  // errno
#include <stdio.h> // fprintf, stdout
#include <string.h> // strerror
#include <sys/syscall.h> // SYS_gettid
#include <unistd.h> // syscall
#include <uv.h> // uv_*

#define DEBUG(fmt, ...) fprintf(stderr, "DEBUG:%lu:%s:%d:%s:" fmt, syscall(SYS_gettid), __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define ERROR(fmt, ...) fprintf(stderr, "ERROR:%lu:%s:%d:%s:" fmt, syscall(SYS_gettid), __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define FATAL(fmt, ...) fprintf(stderr, "FATAL:%lu:%s:%d:%s(%i)%s:" fmt, syscall(SYS_gettid), __FILE__, __LINE__, __func__, errno, strerror(errno), ##__VA_ARGS__)
//#define DEBUG(fmt, ...) fprintf(stdout, "DEBUG:%lu:%li:%s:%d:%s:" fmt, syscall(SYS_gettid), clock(), __FILE__, __LINE__, __func__, ##__VA_ARGS__)
//#define ERROR(fmt, ...) fprintf(stderr, "ERROR:%lu:%li:%s:%d:%s(%i)%s:" fmt, syscall(SYS_gettid), clock(), __FILE__, __LINE__, __func__, errno, strerror(errno), ##__VA_ARGS__)
//#define DEBUG(fmt, ...) fprintf(stderr, "DEBUG:%lu:%lu:%s:%d:%s:" fmt, syscall(SYS_gettid), uv_hrtime(), __FILE__, __LINE__, __func__, ##__VA_ARGS__)
//#define ERROR(fmt, ...) fprintf(stderr, "ERROR:%lu:%lu:%s:%d:%s(%i)%s:" fmt, syscall(SYS_gettid), uv_hrtime(), __FILE__, __LINE__, __func__, errno, strerror(errno), ##__VA_ARGS__)

#endif // _MACROS_H
