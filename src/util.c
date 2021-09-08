/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  util.h - A collection of miscellaneous utility functions.
 *
 *  */

#include "atra.h"

#include <string.h>
#include <SDL2/SDL.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

static int rand_init = 0;

/* Loads a file into memory as an array of bytes */
ATRA_API
i8 *
atra_util_loadfile(const char *path, size_t *size)
{
    FILE *file = fopen(path, "rb");
    size_t length;
    i8 *data;

    if (!file) {
        atra_log_print(LOG_ERROR, "util: Failed to load file \"%s\": " \
                "Unable to open file", path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    data = malloc((length + 2) * sizeof(char));
    fread(data, sizeof(char), length, file);
    data[length + 0] = 0;
    data[length + 1] = EOF;
    fclose(file);

    if (size) {
        *size = length;
    }

    return data;
}

/* Saves an array of bytes to a file */
ATRA_API
void
atra_util_savefile(const char *path, const i8 *data, size_t size)
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

/* Returns a List of strings, containing the names of all files in the given
 * directory.  Gives the option of filtering by file extention. */
ATRA_API
List
atra_util_listdir(const char *path, char *ext)
{
    List result;
    char *entry;
    
#ifdef _WIN32
    WIN32_FIND_DATA fd_file;
    HANDLE find = NULL;
    char fname[2048];

    sprintf(fname, "%s\\*.%s", path, ext);

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
            atra_list_add(&result, entry);
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
        entry = strrchr(files->d_name, '.');
        
        if (entry && !strcmp(++entry, ext)) {
            entry = malloc((strlen(files->d_name) + 1) * sizeof(char));
            strcpy(entry, files->d_name);
            atra_list_add(&result, entry);
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

    return (diff * ((float)rand() / (float)RAND_MAX)) + min;
}

#ifdef __cplusplus
}
#endif
