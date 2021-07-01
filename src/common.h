/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  common.h - Various declarations and macros common to almost all
 *             engine modules.
 *
 *  */

#ifndef ATRA_COMMON_H
#define ATRA_COMMON_H

#define ATRA_VERSION "DEV 0.1.4"

#include "glad.h"

#include <stdint.h>
#include <cglm/cglm.h>
#include <SDL2/SDL.h>


#if defined(_MSC_VER)
    #ifdef ATRA_LIB_EXPORT
        #define ATRA_API __declspec(dllexport)
    #else
        #define ATRA_API __declspec(dllimport)
    #endif

    #define ATRA_INLINE __forceinline
#else
    #define ATRA_ATTR(attr) __attribute__((attr))
    #define ATRA_API    ATRA_ATTR(visibility("default"))
    #define ATRA_INLINE static inline ATRA_ATTR(always_inline)
#endif

#if defined(ATRA_DEBUG)
    #define malloc(s)       atra_debug_malloc(s);
    #define calloc(n, s)    atra_debug_calloc(n, s);
    #define realloc(p, s)   atra_debug_realloc(p, s);
    #define free(p)         atra_debug_free(p);
#elif defined(ATRA_USE_ALLOCATOR)
    #define malloc(s)       atra_mem_malloc(s);
    #define calloc(n, s)    atra_mem_calloc(n, s);
    #define realloc(p, s)   atra_mem_realloc(p, s);
    #define free(p)         atra_mem_free(p);
#endif

#define FALSE   0
#define TRUE    1

#ifndef PI
    #define PI 3.14159265358979323846
#endif

/* Used to silence unused variable warnings */
#define ATRA_UNUSED(x) (void)(x)

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;

typedef struct Quad {
    float x, y, w, h;
} Quad;

#ifdef __cplusplus
}
#endif

#endif /* ATRA_COMMON_H */
