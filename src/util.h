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

/*
 *  Safety and signalling utilities
 */

/* Option for types which do not have a sentinel value but may not return properly */
#define ATRA_OPTION_DECLARE(type) \
typedef struct Option_##type { \
    type some; \
    int none; \
} Option_##type; \
\
Option_##type atra_option_##type##_error(void); \
Option_##type atra_option_##type##_ok(type x);


#define ATRA_OPTION_DEFINE(type) \
\
Option_##type atra_option_##type##_error(void) { \
    return (Option_##type) { .none = 1 }; \
} \
\
Option_##type atra_option_##type##_ok(type x) { \
    return (Option_##type) { .none = 0, .some = x }; \
} \

ATRA_OPTION_DECLARE(char)
ATRA_OPTION_DECLARE(uchar)
ATRA_OPTION_DECLARE(short)
ATRA_OPTION_DECLARE(ushort)
ATRA_OPTION_DECLARE(int)
ATRA_OPTION_DECLARE(uint)
ATRA_OPTION_DECLARE(long)
ATRA_OPTION_DECLARE(ulong)

ATRA_OPTION_DECLARE(i8)
ATRA_OPTION_DECLARE(u8)
ATRA_OPTION_DECLARE(i16)
ATRA_OPTION_DECLARE(u16)
ATRA_OPTION_DECLARE(i32)
ATRA_OPTION_DECLARE(u32)
ATRA_OPTION_DECLARE(i64)
ATRA_OPTION_DECLARE(u64)
ATRA_OPTION_DECLARE(usize)

ATRA_OPTION_DECLARE(float)
ATRA_OPTION_DECLARE(double)
ATRA_OPTION_DECLARE(f32)
ATRA_OPTION_DECLARE(f64)

/*
 *  Filesystem utilities
 */

/* Loads a file into memory as an array of bytes.
 * Optionally returns size of data. Set `size` to NULL if unwanted. */
ATRA_API u8 * atra_util_loadfile(const char *path, size_t *size);

/* Saves an array of bytes to a file */
ATRA_API void atra_util_savefile(const char *path, const u8 *data, size_t size);

/* Creates a new directory with the given path. Returns 1 on success, otherwise 0. */
ATRA_API int atra_util_createdir(const char *path);

/* Returns 1 if the file at the given path exists, otherwise 0 */
ATRA_API int atra_util_fileexists(const char *path);

/* Returns 1 if the directory at the given path exists, otherwise 0 */
ATRA_API int atra_util_direxists(const char *path);

/* Returns 1 if the given file has the specified extension, otherwise 0 */
ATRA_API int atra_util_filehasext(const char *path, const char *ext);

/* Returns a pointer to the extension of a given file name (includes dot '.') */
ATRA_API const char * atra_util_getfileext(const char *filename);

/* Returns a pointer to the file name component of a given path string */
ATRA_API const char * atra_util_getfilename(const char *path);

/* Returns a string with the file name component of a given path string.
 * Returned memory address must be freed. */
ATRA_API const char * atra_util_getfilenamenoext(const char *path);

/* Returns a string with the path component of a given path string.
 * Returned memory address must be freed. */
ATRA_API const char * atra_util_getdirpath(const char *path);

/* Returns a string with the path component of a directory path string.
 * Returned memory address must be freed. */
ATRA_API const char * atra_util_getprevdirpath(const char *path);

/* Returns the full path string of the current working directory.
 * Returned memory address must be freed */
ATRA_API const char * atra_util_getworkingdir(void);

/* Changes the current working directory to the given path (if it exists).
 * Returns 1 on success, otherwise 0. */
ATRA_API int atra_util_changedir(const char *path);

/* Returns a list of strings, containing the names of all files in the given directory.
 * Optional argument 'ext' allows filtering by file extension (set to NULL if unwanted).
 * Returned list and arrays must be freed. */
ATRA_API List atra_util_listdir(const char *path, char *ext);

/* Removes the specified file from the filesystem */
ATRA_API void atra_util_deletefile(const char *path);

/* Sets the default root working directory where the program should run.
 * This is so that relative file paths operate correctly.
 * NOTE: The default root directory is the path to the executable. */
ATRA_API void atra_util_setrootdir(const char *);

/* Returns a pointer to the executable's path string.
 * Returned memory must be freed. */
ATRA_API char * atra_util_getexecpath(void);

/* Returns a pointer to the operating system's application data directory (internal)
 * ~/.local/share/ on Linux.
 * %appdata%/Roaming/ on Windows */
ATRA_API const char * atra_util_getappdatapath(void);

/* Returns true if any file(s) have been dropped into the application window */
ATRA_API int atra_util_hasdroppedfiles(void);

/* Returns a list of file paths which have been dropped into the window */
ATRA_API List atra_util_getdroppedfiles(void);

/* Returns the timestamp of the last time a file was modified */
ATRA_API time_t atra_util_getfilemodtime(void);

/* Compresses an array of bytes using the DEFLATE algorithm.
 * Returned memory address must be freed. */
ATRA_API u8 * atra_util_compressbytes(u8 *data, int length, int *comp_length);

/* Decompresses an array of bytes using the DEFLATE algorithm.
 * Returned memory address must be freed. */
ATRA_API u8 * atra_util_decompressbytes(u8 *data, int comp_length, int *length);


/*
 *  UTF-8 functions
 */

/* Translates a UTF-8 string to an array of codepoints.
 * Returned memory must be freed. */
ATRA_API int * atra_utf_tocodepoints(const char *str, int *count);

/* Translates a UTF-8 string to a space-terminated array of codepoints.
 * Returned memory must be freed. */
ATRA_API int * atra_utf_tocodepointsterm(const char *str, int *count);

/* Returns the total number of codepoints in a UTF-8 string */
ATRA_API int atra_utf_getcodepointcount(const char *str);

/* Returns the next codepoint in a UTF-8 string, advancing the `pos` */
ATRA_API int atra_utf_getcodepoint(const char *str, int *pos);

/* Translates a single codepoint to a UTF-8 string.  Returns size as parameter.
 * Subsequent calls clobber previous results. */
ATRA_API const char * atra_utf_fromcodepoint(int codepoint, int *size);

/* Translates a codepoint array into a UTF-8 string.
 * Returned memory must be freed. */
ATRA_API char * atra_utf_fromcodepointstr(int *codepoints, int length);

/* Returns non-zero if the given codepoint matches any UTF-8 breaking characters
 * in which a word-wrap can occur */
ATRA_API int atra_utf_iscodepointbreaking(int codepoint);

/* Removes a codepoint from a UTF-8 codepoint string in range [start, end) */
ATRA_API void atra_utf_removecodepoints(int *codepoints, int *code_count, int start, int end);

/* Inserts a given string at the given position
 * Returns a newly allocated codepoint array */
ATRA_API int * atra_utf_insertstr(int *codepoints, int *code_count, const char *str, int pos);


/*
 *  Text manipulation functions
 */

/* An explicit definition of the non-standard `strdup` function 
 * Returned string must be freed. */
ATRA_API char * atra_util_strdup(const char *str);

/* Returns part of a string as a new null-terminated string. 
 * Returned string must be freed. */
ATRA_API char * atra_util_substr(const char *str, int pos, int len);

/* Replaces occurrences of `replace` in `str` with `by`.
 * Returned string must be freed. */
ATRA_API char * atra_util_replacestr(char *str, const char *replace, const char *by);

/* Inserts a string in a specific position in another string.
 * Returned string must be freed. */
ATRA_API char * atra_util_insertstr(const char *str, const char *insert, int pos);

/* Removes a set number of characters from a given string */
ATRA_API void atra_util_removerange(char *str, int pos, int length);

/* Joins text with a delimiter.
 * Returned string must be freed. */
ATRA_API char * atra_util_joinstr(List *strs, const char *delim);

/* Tokenises a string into a list of strings using a string of delimiters.
 * Returned string must be freed. */
ATRA_API List atra_util_tokenisestr(const char *str, const char *delims);

/* Appends text at a specific position and moves cursor. */
ATRA_API void atra_util_appendstr(char *str, const char *append, int *pos);

/* Returns the index of the first occurrence of one string within another */
ATRA_API int atra_util_findindex(const char *str, const char *find);

/* Converts a string to upper-case characters.
 * Returned string must be freed. */
ATRA_API char * atra_util_toupper(const char *str);

/* Converts a string to lower-case characters.
 * Returned string must be freed. */
ATRA_API char * atra_util_tolower(const char *str);

/*
 *  Random number generation
 */

/* Generates a pseudo-random 32-bit integer in range [min, max] */
ATRA_API int atra_util_randomint(int min, int max);

/* Generates a pseudo-random 32-bit floating point number in range [min, max] */
ATRA_API float atra_util_randomfloat(float min, float max);


/*
 *  Miscellaneous utilities
 */

/* Terminates the application with exit code EXIT_FAILURE */
ATRA_API void atra_util_die(const char *msg);

/* Halts the engine and gracefully exits on memory allocation error */
ATRA_API void atra_util_checkalloc(void *ptr);

/* Prints SDL errors in a standardised fashion */
ATRA_API void atra_util_sdl_printerror(void);

/* Opens the given URL in the default browser (if available) */
ATRA_API void atra_util_openurl(const char *url);

/* Returns the number of logical CPU cores */
ATRA_API int atra_util_getcorecount(void);

#ifdef __cplusplus
}
#endif

#endif /* ATRA_UTIL_H */
