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

#define ATRA_VERSION "ALPHA 1.0.0"

#ifdef __linux__
    #define _POSIX_C_SOURCE 200112L 
#endif

#include "glad.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <cglm/cglm.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define CGLTF_IMPLEMENTATION

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

#define FALSE           0
#define TRUE            1

#ifndef PI
    #define PI          3.14159265358979323846
#endif

#ifndef PATH_MAX
    #define PATH_MAX    4096
#endif

#define BIT(b) (1 << (b))
#define BIT_SET(f, b) ((f) |= (b))
#define BIT_CLR(f, b) ((f) &= ~(b))
#define BIT_TOG(f, b) ((f) ^= (b))
#define BIT_CHK(f, b) (((f) & (b)) != 0)

/* Branchless decider between two numbers */
#define DECIDE(cond, t, f) \
        ((((cond) != 0) * (t)) + \
        (((cond) == 0) * (f)))

#define MIN(a, b) DECIDE((a) < (b), (a), (b))
#define MAX(a, b) DECIDE((a) > (b), (a), (b))
#define CLAMP(n, min, max) MAX(min, MIN(max, n))

#define DEG_TO_RAD(x) ((x) * (PI / 180))
#define RAD_TO_DEG(x) (((x) / PI) * 180)

/* Silences unused variable warnings */
#define ATRA_UNUSED(x) (void)(x)

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef float       f32;
typedef double      f64;

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;

typedef size_t      usize;
typedef uintptr_t   uptr;
typedef ptrdiff_t   iptr;
typedef void *      vptr;

typedef int ivec2[2];

typedef struct Quad {
    float x, y, w, h;
} Quad;

typedef struct IQuad {
    int x, y, w, h;
} IQuad;

#ifdef __cplusplus
}
#endif

#endif /* ATRA_COMMON_H */
