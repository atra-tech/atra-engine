/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  util.h - A collection of miscellaneous utility functions.
 *
 *  */

#include "atra.h"
#include "whereami.h"

#include <string.h>
#include <sys/stat.h>

#include <SDL2/SDL.h>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
    #include <dirent.h>
    #include <direct.h>
    #include <io.h>
#else
    #include <limits.h>
    #include <dirent.h>
    #include <unistd.h>
#endif

#define SINFL_NOSIMD
#include "stb/sinfl.h"

#include "stb/sdefl.h"

#define MAX_DECOMPRESSION_SIZE      64

#ifndef MAX_TEXT_BUFFER_LEN
    #define MAX_TEXT_BUFFER_LEN     4096
#endif
#ifndef MAX_UNICODE_CHARS
    #define MAX_UNICODE_CHARS       2048
#endif

#ifndef MAX_FILEPATH_LENGTH
    #ifdef __linux__
        #define MAX_FILEPATH_LENGTH 4096
    #else
        #define MAX_FILEPATH_LENGTH 512
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

static int rand_init = 0;

ATRA_OPTION_DEFINE(char)
ATRA_OPTION_DEFINE(uchar)
ATRA_OPTION_DEFINE(short)
ATRA_OPTION_DEFINE(ushort)
ATRA_OPTION_DEFINE(int)
ATRA_OPTION_DEFINE(uint)
ATRA_OPTION_DEFINE(long)
ATRA_OPTION_DEFINE(ulong)

ATRA_OPTION_DEFINE(i8)
ATRA_OPTION_DEFINE(u8)
ATRA_OPTION_DEFINE(i16)
ATRA_OPTION_DEFINE(u16)
ATRA_OPTION_DEFINE(i32)
ATRA_OPTION_DEFINE(u32)
ATRA_OPTION_DEFINE(i64)
ATRA_OPTION_DEFINE(u64)
ATRA_OPTION_DEFINE(usize)

ATRA_OPTION_DEFINE(float)
ATRA_OPTION_DEFINE(double)
ATRA_OPTION_DEFINE(f32)
ATRA_OPTION_DEFINE(f64)


/* Loads a file into memory as an array of bytes */
ATRA_API
u8 *
atra_util_loadfile(const char *path, size_t *size)
{
    FILE *file = fopen(path, "rb");
    size_t length;
    u8 *data;

    if (!file) {
        atra_log_print(LOG_ERROR, "util: Failed to load file \"%s\": " \
                "Unable to open file", path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    length = (ulong) ftell(file);
    fseek(file, 0, SEEK_SET);
    data = malloc((length + 1) * sizeof(char));
    fread(data, sizeof(char), length, file);
    data[length] = 0;
    fclose(file);

    if (size) {
        *size = length;
    }

    return data;
}

/* Saves an array of bytes to a file */
ATRA_API
void
atra_util_savefile(const char *path, const u8 *data, size_t size)
{
    FILE *file = fopen(path, "wb");

    if (!file) {
        atra_log_print(LOG_ERROR, "util: Failed to save file \"%s\": " \
                "Unable to open file", path);
        return;
    }
    fwrite(data, sizeof(char), size, file);
    fclose(file);
}

/* Creates a new directory with the given path. Returns 1 on success, otherwise 0. */
ATRA_API
int
atra_util_createdir(const char *path)
{
    int result = 0;
#ifdef _WIN32
    if (CreateDirectory(path, NULL)) {
        result = 1;
    } else {
        atra_log_print(LOG_WARN, "util: Failed to create directory: %s", path);
    }
#else
    if (mkdir(path, 0777) == 0) {
        result = 1;
    } else {
        atra_log_print(LOG_WARN, "util: Failed to create directory: %s", path);
    }
#endif
    return result;
}

/* Returns 1 if the file at the given path exists, otherwise 0 */
ATRA_API
int
atra_util_fileexists(const char *path)
{
    int result;

#ifdef _WIN32
    result = (_access(path, 0) != -1);
#else
    result = (access(path, F_OK) != -1);
#endif

    return result;
}

/* Returns 1 if the directory at the given path exists, otherwise 0 */
ATRA_API
int
atra_util_direxists(const char *path)
{
    int result = 0;
    DIR *dir = opendir(path);

    if (dir != NULL) {
        result = 1;
        closedir(dir);
    }

    return result;
}

/* Returns 1 if the given file has the specified extension, otherwise 0 */
ATRA_API
int
atra_util_filehasext(const char *path, const char *ext)
{
    const char *file_ext = atra_util_getfileext(path);
    char file_ext_lower[16] = {0};
    char *lower;
    char **elem;
    
    List check_exts;
    size_t i, result = 0;

    if (file_ext != NULL) {
        
        check_exts = atra_util_tokenisestr(ext, ";");

        lower = atra_util_tolower(file_ext);
        strcpy(file_ext_lower, lower);
        free(lower);

        for (i = 0; i < check_exts.size; i++) {
            elem = atra_list_get(&check_exts, i);
            lower = atra_util_tolower(*elem);

            if (strcmp(file_ext_lower, lower) == 0) {
                result = 1;
                free(lower);
                break;
            }
            free(lower);
        }
        atra_list_dealloc(&check_exts);
        atra_list_delete(&check_exts);
    }

    return (int)result;
}

/* Returns a pointer to the extension of a given file name (includes dot '.') */
ATRA_API
const char *
atra_util_getfileext(const char *filename)
{
    const char *dot = strrchr(filename, '.');

    if (!dot || dot == filename) {
        return NULL;
    }

    return dot;
}

ATRA_INLINE
const char *
strprbrk(const char *s, const char *charset)
{
    const char *latest = NULL;
    
    for (; (void)(s = strpbrk(s, charset)), s != NULL; latest = s++) {}

    return latest;
}

/* Returns a pointer to the file name component of a given path string */
ATRA_API
const char *
atra_util_getfilename(const char *path)
{
    const char *filename = NULL;

    if (path != NULL) {
        filename = strprbrk(path, "\\/");
    }

    if (!filename) {
        return path;
    }

    return filename + 1;
}

/* Returns a string with the file name component of a given path string.
 * Returns internal buffer, subsequent calls clobber results. */
ATRA_API
const char *
atra_util_getfilenamenoext(const char *path)
{
    #define MAX_FILENAME_NOEXT_LEN 1024

    static char filename[MAX_FILENAME_NOEXT_LEN];
    size_t size, i;

    memset(filename, 0, MAX_FILENAME_NOEXT_LEN);

    if (path != NULL) {
        strcpy(filename, atra_util_getfilename(path));
    }
    size = strlen(filename);

    for (i = 0; i < size && i < MAX_FILENAME_NOEXT_LEN; i++) {
        if (filename[i] == '.') {
            filename[i] = '\0';
            break;
        }
    }

    return filename;
}

/* Returns a string with the path component of a given path string.
 * Returns internal buffer, subsequent calls clobber previous results. */
ATRA_API
const char *
atra_util_getdirpath(const char *path)
{
    const char *lastslash = NULL;
    static char dirpath[MAX_FILEPATH_LENGTH] = {0};
    
    memset(dirpath, 0, MAX_FILEPATH_LENGTH);

    if (path[1] != ':' && path[0] != '\\' && path[0] != '/') {
        dirpath[0] = '.';
        dirpath[1] = '/';
    }
    lastslash = strprbrk(path, "\\/");

    if (lastslash) {
        if (lastslash == path) {
            dirpath[0] = path[0];
            dirpath[1] = '\0';
        } else {
            memcpy(dirpath + (path[1] != ':' && path[0] != '\\' && path[0] != '/' ? 2 : 0), path, strlen(path) - (strlen(lastslash) - 1));
            dirpath[strlen(path) - strlen(lastslash) + (path[1] != ':' && path[0] != '\\' && path[0] != '/' ? 2 : 0)] = '\0';
        }
    }
    
    return dirpath;
}

/* Returns a string with the path component of a directory path string.
 * Returns internal buffer, subsequent calls clobber previous results. */
ATRA_API
const char *
atra_util_getprevdirpath(const char *path)
{
    static char prevdirpath[MAX_FILEPATH_LENGTH] = {0};
    int path_len = (int) strlen(path);
    int i;
    
    memset(prevdirpath, 0, MAX_FILEPATH_LENGTH);

    if (path_len <= 3) {
        strcpy(prevdirpath, path);
    }

    for (i = path_len - 1; i >= 0 && path_len > 3; i--) {
        if (path[i] == '\\' || path[i] == '/') {
            if ((i == 2 && path[1] == ':') || i == 0) {
                i++;
            }
            strncpy(prevdirpath, path, (ulong) i);
            break;
        }
    }

    return prevdirpath;
}

/* Returns the full path string of the current working directory.
 * Uses internal buffer, subsequent calls clobber previous results. */
ATRA_API
const char *
atra_util_getworkingdir(void)
{
    static char curr_dir[MAX_FILEPATH_LENGTH];
    char *path;

    memset(curr_dir, 0, MAX_FILEPATH_LENGTH);
#ifdef _WIN32
    path = _getcwd(curr_dir, MAX_FILEPATH_LENGTH - 1);
#else
    path = getcwd(curr_dir, MAX_FILEPATH_LENGTH - 1);
#endif

    return path;
}

/* Changes the current working directory to the given path (if it exists).
 * Returns 1 on success, otherwise 0. */
ATRA_API
int
atra_util_changedir(const char *path)
{
    int result;

#ifdef _WIN32
    result = _chdir(path);
#else
    result = chdir(path);
#endif

    return (result == 0);
}

/* Returns a List of strings, containing the names of all files in the given
 * directory.  Gives the option of filtering by file extention. */
ATRA_API
List
atra_util_listdir(const char *path, char *ext)
{
    List result = {0};
    char *entry;
    
#ifdef _WIN32
    WIN32_FIND_DATA fd_file;
    HANDLE find = NULL;
    char fname[2048];

    if (ext != NULL) {
        sprintf(fname, "%s\\*.%s", path, ext);
    } else {
        sprintf(fname, "%s\\*.*", path);
    }

    if ((find = FindFirstFile(fname, &fd_file)) == INVALID_HANDLE_VALUE) {
        atra_log_print(LOG_ERROR, "util: Directory not found: \"%s\"", path);
        return (List){0};
    }
    result = atra_list_create(sizeof(char *));

    do {
        if (strcmp(fd_file.cFileName, ".") &&
                strcmp(fd_file.cFileName, "..")) {
            entry = malloc((strlen(fd_file.cFileName) + 1) * sizeof(char));
            strcpy(entry, fd_file.cFileName);
            atra_list_add(&result, &entry);
        }
    } while (FindNextFile(find, &fd_file));

    FindClose(find);
#else
    struct dirent *files;
    DIR *dir = opendir(path);

    if (!dir) {
        atra_log_print(LOG_ERROR, "util: Directory not found: \"%s\"", path);
        return (List){0};
    }
    result = atra_list_create(sizeof(char *));

    while ((files = readdir(dir))) {
        if (strcmp(files->d_name, ".") == 0 ||
                strcmp(files->d_name, "..") == 0) {
            continue;
        }

        if (ext != NULL) {
            entry = strrchr(files->d_name, '.');
        
            if (entry && !strcmp(++entry, ext)) {
                entry = malloc((strlen(files->d_name) + 1) * sizeof(char));
                strcpy(entry, files->d_name);
                atra_list_add(&result, &entry);
            }
        } else {
            entry = malloc((strlen(files->d_name) + 1) * sizeof(char));
            strcpy(entry, files->d_name);
            atra_list_add(&result, &entry);
        }
    }
    closedir(dir);
#endif

    return result;
}

/* Removes the specified file from the filesystem */
ATRA_API
void
atra_util_deletefile(const char *path)
{
    remove(path);
}

/* Sets the default root working directory where the program should run.
 * This is so that relative file paths operate correctly.
 * NOTE: The default root directory is the path to the executable. */
ATRA_API
void
atra_util_setrootdir(const char *dir)
{
    ATRA_UNUSED(dir);
#ifdef _WIN32

#else

#endif
}

/* Returns a pointer to the executable's path string.
 * Returned memory must be freed. */
ATRA_API
char *
atra_util_getexecpath(void)
{
/*    static char result[MAX_FILEPATH_LENGTH] = {0};
#ifdef _WIN32
    GetModuleFileName(NULL, result, MAX_FILEPATH_LENGTH);
#else
    ssize_t count = readlink("/proc/self/exe", result, MAX_FILEPATH_LENGTH);
    
    if (count <= 0) {
        return NULL;
    }
#endif

    return result;*/

    char *path;
    int length, dirname_length;
    
    length = wai_getExecutablePath(NULL, 0, NULL);
    path = malloc((size_t)(length + 1) * sizeof(char));
    wai_getExecutablePath(path, length, &dirname_length);
    path[length] = '\0';

    return path;
}

/* Returns a pointer to the operating system's application data directory (internal)
 * ~/.local/share/ on Linux.
 * %appdata%/Roaming/ on Windows */
ATRA_API
const char *
atra_util_getappdatapath(void)
{
    static char path[MAX_FILEPATH_LENGTH] = {0};
    char *appdir;

#ifdef _WIN32
    SHGetSpecialFolderPathA(NULL, appdir, CSIDL_APPDATA, 1);
#else
    appdir = getenv("HOME");
    sprintf(path, "%s/.local", appdir);

    if (atra_util_direxists(path) == 0) {
        atra_util_createdir(path);
    }
    sprintf(path, "%s/.local/share", appdir);

    if (atra_util_direxists(path) == 0) {
        atra_util_createdir(path);
    }    
#endif

    return path;
}

/* Returns true if any file(s) have been dropped into the application window */
ATRA_API
int
atra_util_hasdroppedfiles(void)
{

#ifdef _WIN32

#else

#endif
    return 0;
}

/* Returns a list of file paths which have been dropped into the window */
ATRA_API
List
atra_util_getdroppedfiles(void)
{
    List files = {0};
#ifdef _WIN32

#else

#endif
    return files;
}

/* Returns the timestamp of the last time a file was modified */
ATRA_API
time_t
atra_util_getfilemodtime(void)
{

#ifdef _WIN32

#else

#endif
    return 0;
}

/* Compresses an array of bytes using the DEFLATE algorithm.
 * Returned memory address must be freed. */
ATRA_API
u8 *
atra_util_compressbytes(u8 *data, int length, int *comp_length)
{
    const uint QUALITY = 8;
    
    u8 *comp_data = NULL;
    struct sdefl sdefl = {0};
    int bounds = sdefl_bound(length);

    comp_data = (u8 *) calloc(bounds, 1);
    *comp_length = sdeflate(&sdefl, comp_data, data, length, QUALITY);

    atra_log_print(LOG_INFO, "util: Compression: %iB -> %iB", length, *comp_length);

    return comp_data;
}

/* Decompresses an array of bytes using the DEFLATE algorithm.
 * Returned memory address must be freed. */
ATRA_API
u8 *
atra_util_decompressbytes(u8 *data, int comp_length, int *length)
{
    u8 *decomp_data = NULL;
    u8 *temp;
    int decomp_length;

    decomp_data = calloc(MAX_DECOMPRESSION_SIZE * 1024 * 1024, 1);
    decomp_length = sinflate(decomp_data, MAX_DECOMPRESSION_SIZE, data, comp_length);
    temp = realloc(data, decomp_length);
    atra_util_checkalloc(temp);

    decomp_data = temp;
    *length = decomp_length;

    atra_log_print(LOG_INFO, "util: Decompression: %iB -> %iB", length, decomp_length);

    return decomp_data;
}


/*
 *  UTF-8 functions
 */

/* Translates a UTF-8 string to an array of codepoints.
 * Returned memory must be freed. */
ATRA_API
int *
atra_utf_tocodepoints(const char *str, int *count)
{
    int str_len = strlen(str);
    int bytes_processed = 0;
    int codepoint_count = 0;
    int *temp, *codepoints = calloc(str_len, sizeof(int));
    int i;

    for (i = 0; i < str_len; codepoint_count++) {
        codepoints[codepoint_count] = atra_utf_getcodepoint(str + i, &bytes_processed);
        i += bytes_processed;
    }
    temp = realloc(codepoints, codepoint_count * sizeof(int));

    if (temp != NULL) {
        codepoints = temp;
    }
    *count = codepoint_count;

    return codepoints;
}

/* Translates a UTF-8 string to a space-terminated array of codepoints.
 * Returned memory must be freed. */
ATRA_API
int *
atra_utf_tocodepointsterm(const char *str, int *count)
{
    int str_len = strlen(str);
    int bytes_processed = 0;
    int codepoint_count = 0;
    int *temp, *codepoints = calloc(str_len, sizeof(int));
    int i;

    for (i = 0; i < str_len; codepoint_count++) {
        codepoints[codepoint_count] = atra_utf_getcodepoint(str + i, &bytes_processed);
        i += bytes_processed;
    }
    codepoint_count++;
    temp = realloc(codepoints, codepoint_count * sizeof(int));

    if (temp != NULL) {
        codepoints = temp;
    }
    *count = codepoint_count;
    codepoints[codepoint_count - 1] = ' ';

    return codepoints;
}

/* Returns the total number of codepoints in a UTF-8 string */
ATRA_API
int
atra_utf_getcodepointcount(const char *str)
{
    uint length = 0;
    int next, letter;

    while (*str != '\0') {
        next = 0;
        letter = atra_utf_getcodepoint(str, &next);

        if (letter == 0x3f) {
            str += 1;
        }
        length++;
    }

    return length;
}

/* Returns the next codepoint in a UTF-8 string.
 * Returns the number of bytes processed through `pos`.
 * Returns '?' (0x3f) on failure. */
ATRA_API
int
atra_utf_getcodepoint(const char *str, int *pos)
{
    int code = 0x3f;
    int octet = (uchar)(str[0]);
    uchar octet1, octet2, octet3;
    *pos = 1;

    if (octet <= 0x7f) {
        code = str[0];
    } else if ((octet & 0xe0) == 0xc0) {
        octet1 = str[1];

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) {
            *pos = 2;
            return code;
        }

        if ((octet >= 0xc2) && (octet <= 0xdf)) {
            code = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            *pos = 2;
        }
    } else if ((octet & 0xf0) == 0xe0) {
        octet1 = str[1];
        octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) {
            *pos = 2;
            return code;
        }
        octet2 = str[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) {
            *pos = 3;
            return code;
        }

        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
                ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) {
            *pos = 2;
            return code;
        }

        if ((octet >= 0xe0) && (0 <= 0xef)) {
            code = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            *pos = 3;
        }
    } else if ((octet & 0xf8) == 0xf0) {
        if (octet > 0xf4) {
            return code;
        }
        octet1 = str[1];
        octet2 = '\0';
        octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) {
            *pos = 2;
            return code;
        }
        octet2 = pos[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) {
            *pos = 3;
            return code;
        }
        octet3 = str[3];

        if ((octet3 == '\0') || ((octet3 >> 6) != 2)) {
            *pos = 4;
            return code;
        }

        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
                ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) {
            *pos = 2;
            return code;
        }

        if (octet >= 0xf0)
        {
            code = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            *pos = 4;
        }
    }

    if (code > 0x10ffff) code = 0x3f;

    return code;
}

/* Translates a single codepoint to a UTF-8 string.  Returns size as parameter.
 * Subsequent calls clobber previous results. */
ATRA_API
const char *
atra_utf_fromcodepoint(int codepoint, int *size)
{
    static char utf8[6] = {0};
    int cp_size = 0;

    if (codepoint <= 0x7f) {
        utf8[0] = (char)codepoint;
        cp_size = 1;
    } else if (codepoint <= 0x7ff) {
        utf8[0] = (char)(((codepoint >> 6) & 0x1f) | 0xc0);
        utf8[1] = (char)((codepoint & 0x3f) | 0x80);
        cp_size = 2;
    } else if (codepoint <= 0xffff) {
        utf8[0] = (char)(((codepoint >> 12) & 0x0f) | 0xe0);
        utf8[1] = (char)(((codepoint >> 6) & 0x3f) | 0x80);
        utf8[2] = (char)((codepoint & 0x3f) | 0x80);
        cp_size = 3;
    } else if (codepoint <= 0x10ffff) {
        utf8[0] = (char)(((codepoint >> 18) & 0x07) | 0xf0);
        utf8[1] = (char)(((codepoint >> 12) & 0x3f) | 0x80);
        utf8[2] = (char)(((codepoint >> 6) & 0x3f) | 0x80);
        utf8[3] = (char)((codepoint & 0x3f) | 0x80);
        cp_size = 4;
    }
    *size = cp_size;

    return utf8;
}

/* Translates a codepoint array into a UTF-8 string.
 * Returned memory must be freed. */
ATRA_API
char *
atra_utf_fromcodepointstr(int *codepoints, int length)
{
    char *ptr, *str = calloc(length * 5, 1);
    const char *utf8 = NULL;
    int i, bytes, size = 0;

    for (i = 0, bytes = 0; i < length; i++) {
        utf8 = atra_utf_fromcodepoint(codepoints[i], &bytes);
        memcpy(str + size, utf8, bytes);
        size += bytes;
    }
    ptr = realloc(str, size + 1);

    if (ptr != NULL) {
        str = ptr;
    }

    return str;
}

/* Returns non-zero if the given codepoint matches any UTF-8 breaking characters
 * in which a word-wrap can occur */
ATRA_API
int
atra_utf_iscodepointbreaking(int codepoint)
{
    const int breakers[] = {
        0x0020, 0x00A0, 0x1680,
        0x2000, 0x2001, 0x2002,
        0x2003, 0x2004, 0x2005,
        0x2006, 0x2008, 0x2009,
        0x200A, 0x205F, 0x3000
    };
    
    int i, result = 0;

    for (i = 0; i < 15; i++) {
        result += (codepoint == breakers[i]);
    }

    return result;
}

/* Removes a codepoint from a UTF-8 codepoint string in range [start, end) */
ATRA_API
void
atra_utf_removecodepoints(int *codepoints, int *code_count, int start, int end)
{
    memmove(codepoints + start, codepoints + end, (*code_count - end) * sizeof(int));
    *code_count -= end - start;
}

/* Inserts a given string at the given position
 * Takes the current codepoint count and returns the new count through `code_count`
 * Returns a newly allocated codepoint array */
ATRA_API
int *
atra_utf_insertstr(int *codepoints, int *code_count, const char *str, int pos)
{
    int *str_codes, *result;
    int str_len;
    int result_len;

    result_len = *code_count;
    str_codes = atra_utf_tocodepoints(str, &str_len);
    result = realloc(codepoints, (result_len + str_len) * sizeof(int));

    memmove(result + pos + str_len, result + pos, (result_len - pos) * sizeof(int));
    memcpy(result + pos, str_codes, str_len * sizeof(int));
    *code_count += str_len;

    free(str_codes);

    return result;
}


/*
 *  Text manipulation functions
 */

/* Returns part of a string as a new null-terminated string. 
 * Returned string must be freed. */
ATRA_API
char *
atra_util_substr(const char *str, int pos, int len)
{
    char *result = malloc((len + 1) * sizeof(char));

    memcpy(result, str + pos, len);
    result[len] = '\0';

    return result;
}

/* Replaces occurrences of `replace` in `str` with `by`.
 * Returned string must be freed. */
ATRA_API
char *
atra_util_replacestr(char *str, const char *replace, const char *by)
{
    char *result;
    char *insert_point;
    char *temp;
    int replace_len;
    int by_len;
    int last_replace_pos;
    int count;

    replace_len = strlen(replace);
    by_len = strlen(by);

    insert_point = str;

    for (count = 0; (temp = strstr(insert_point, replace)); count++) {
        insert_point = temp + replace_len;
    }
    temp = result = malloc(strlen(str) + (by_len - replace_len) * count + 1);

    while(count--) {
        insert_point = strstr(str, replace);
        last_replace_pos = insert_point - str;
        temp = strncpy(temp, str, last_replace_pos) + last_replace_pos;
        temp = strcpy(temp, by) + by_len;
        str += last_replace_pos + replace_len;
    }
    strcpy(temp, str);

    return result;
}

/* Inserts a string in a specific position in another string.
 * Returned string must be freed. */
ATRA_API
char *
atra_util_insertstr(const char *str, const char *insert, int pos)
{
    const int str_len = strlen(str);
    const int insert_len = strlen(insert);
    const int post_size = pos + insert_len;

    char *insert_point;
    char *result = malloc(str_len + insert_len + 1);

    insert_point = result;

    memcpy(insert_point, str, pos * sizeof(char));
    memcpy(insert_point + pos, insert, insert_len * sizeof(char));
    memcpy(insert_point + post_size, str + pos, (str_len - post_size) * sizeof(char));
    result[str_len + insert_len] = '\0';

    return result;
}

/* Removes a set number of characters from a given string */
ATRA_API
void
atra_util_removerange(char *str, int pos, int length)
{
    const int str_len = strlen(str);

    for (; pos + length < str_len; pos++) {
        str[pos] = str[pos + length];
    }
    str[pos] = '\0';
}

/* Joins text with a delimiter.
 * Returned string must be freed. */
ATRA_API
char *
atra_util_joinstr(List *strs, const char *delim)
{
    const char delim_len = strlen(delim);

    char *curr_str, *result, *curr_pos;
    size_t total_len = 0;
    size_t curr_len, i;    

    for (i = 0; i < strs->size; i++) {
        curr_str = atra_list_get(strs, i);
        total_len += strlen(curr_str);
    }
    total_len += delim_len * (strs->size - 1);
    result = malloc((total_len + 1) * sizeof(char));
    curr_pos = result;

    for (i = 0; i < strs->size; i++) {
        curr_str = atra_list_get(strs, i);
        curr_len = strlen(curr_str);
        memcpy(curr_pos, curr_str, curr_len);

        if (i < strs->size - 1) {
            memcpy(curr_pos + curr_len, delim, delim_len);
        }
        curr_pos += curr_len + delim_len;
    }
    result[total_len] = '\0';

    return result;
}

/* Tokenises a string into a list of strings using a string of delimiters.
 * Returned list and contained strings must be freed. */
ATRA_API
List
atra_util_tokenisestr(const char *str, const char *delims)
{
    List result;
    char *token;
    char *str_cpy;
    char *entry;
    size_t str_len;

    str_len = strlen(str) + 1;
    str_cpy = malloc(str_len);
    memcpy(str_cpy, str, str_len);

    result = atra_list_create(sizeof(char *));
    token = strtok(str_cpy, delims);

    while (token != NULL) {
        str_len = strlen(token) + 1;
        entry = malloc(str_len * sizeof(char));
        memcpy(entry, token, str_len * sizeof(char));
        atra_list_add(&result, &entry);
        
        token = strtok(NULL, delims);
    }
    free(str_cpy);

    return result;
}

/* Appends text at a specific position and moves cursor. */
ATRA_API
void
atra_util_appendstr(char *str, const char *append, int *pos)
{
    strcpy(str + *pos, append);
    pos += strlen(append);
}

/* Returns the index of the first occurrence of one string within another */
ATRA_API
int
atra_util_findindex(const char *str, const char *find)
{
    int pos = -1;
    char *ptr = strstr(str, find);

    if (ptr != NULL) {
        pos = ptr - str;
    }

    return pos;
}

/* Converts a string to upper-case characters.
 * Returned string must be freed. */
ATRA_API
char *
atra_util_toupper(const char *str)
{
    size_t str_len = strlen(str);
    char *result = malloc((str_len + 1) * sizeof(char));
    size_t i;

    for (i = 0; i < str_len; i++) {
        result[i] = toupper(str[i]);
    }
    result[str_len] = '\0';

    return result;
}

/* Converts a string to lower-case characters.
 * Returned string must be freed. */
ATRA_API
char *
atra_util_tolower(const char *str)
{
    size_t str_len = strlen(str);
    char *result = malloc((str_len + 1) * sizeof(char));
    size_t i;

    for (i = 0; i < str_len; i++) {
        result[i] = tolower(str[i]);
    }
    result[str_len] = '\0';

    return result;
}




/*
 *  Random number generation
 */

/* Generates a pseudo-random 32-bit integer in range [min, max] */
ATRA_API
int
atra_util_randomint(int min, int max)
{
    const int diff = max - min;

    if (diff < 0) {
        atra_log_print(LOG_WARN, "util: Intersecting random number bounds");
    }

    if (!rand_init) {
        srand((unsigned) time(NULL));
        rand_init = 1;
    }
    
    return (rand() % diff) + min;
}

/* Generates a pseudo-random 32-bit floating point number in range [min, max] */
ATRA_API
float
atra_util_randomfloat(float min, float max)
{
    const float diff = max - min;

    if (diff < 0) {
        atra_log_print(LOG_WARN, "util: Intersecting random number bounds");
    }
    if (!rand_init) {
        srand((unsigned) time(NULL));
        rand_init = 1;
    }
    const float r = rand();

    return (diff * (r / (float)RAND_MAX)) + min;
}


/*
 *  Miscellaneous utilities
 */

/* An explicit definition of the non-standard `strdup` function 
 * Returns newly allocated memory, containing a copy of the given string */
ATRA_API
char *
atra_util_strdup(const char *str)
{
    size_t len;
    char *str_cpy;

    len = strlen(str) + 1;
    str_cpy = malloc(len * sizeof(char));
    memcpy(str_cpy, str, len);

    return str_cpy;
}

/* Terminates the application with exit code EXIT_FAILURE */
ATRA_API
void
atra_util_die(const char *msg)
{
    if (msg) {
        atra_log_print(LOG_FATAL, "A fatal error has occurred: %s", msg);
    } else {
        atra_log_print(LOG_FATAL, "A fatal error has occurred: Terminating");
    }
    exit(EXIT_FAILURE);
}

/* Halts the engine and gracefully exits on memory allocation error */
ATRA_API
void
atra_util_checkalloc(void *ptr)
{
    if (ptr == NULL) {
        atra_util_die("Memory allocation error");
    }
}

/* Prints SDL errors in a standardized fashion */
ATRA_API
void
atra_util_sdl_printerror(void)
{
    atra_log_print(LOG_ERROR, "SDL error: %s", SDL_GetError());
}

/* Opens the given URL in the default browser (if available) */
ATRA_API
void
atra_util_openurl(const char *url)
{   
    if (strchr(url, '\'') != NULL) {
        atra_log_print(LOG_INFO, "util: Unable to open URL: Invalid URL");
        return;
    }
    char *cmd = calloc(strlen(url) + 10, sizeof(char));

#ifdef _WIN32
    sprintf(cmd, "explorer %s", url);
#else
    sprintf(cmd, "xdg-open '%s'", url);
#endif
    system(cmd);
    free(cmd);
}

/* Returns the number of logical CPU cores */
ATRA_API
int
atra_util_getcorecount(void)
{
    return SDL_GetCPUCount();
}

#ifdef __cplusplus
}
#endif
