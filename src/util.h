/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  util.h - A collection of miscellaneous utility functions.
 *
 *  */

#ifndef ATRA_UTIL_H
#define ATRA_UTIL_H

#include "common.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Loads a file into memory as an array of bytes */
ATRA_API i8 * atra_util_loadfile(const char *path);

/* Saves an array of bytes to a file */
ATRA_API void atra_util_savefile(const char *path, const char *data, size_t nitems);

/* Returns a List of strings, containing the names of all files in the given
 * directory.  Gives the option of filtering by file extention. */
ATRA_API List atra_util_listdir(const char *path, char *ext);

/* Removes the specified file from the filesystem */
ATRA_API void atra_util_deletefile(const char *path);

/* An explicit definition of the non-standard `strdup` function 
 * Returns newly allocated memory, containing a copy of the given string */
ATRA_API char * atra_util_strdup(const char *str);

/* Terminates the application with exit code EXIT_FAILURE */
ATRA_API void atra_util_die(const char *msg);

/* Halts the engine and gracefully exits on memory allocation error */
ATRA_API void atra_util_checkalloc(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* ATRA_UTIL_H */
