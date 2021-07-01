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

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Loads a file into memory as an array of bytes */
ATRA_API
i8 *
atra_util_loadfile(const char *path)
{
    FILE *file = fopen(path, "rb");
    int length;
    i8 *data;

    if (!file) {
        atra_log_print(LOG_ERROR, "util: Failed to load file \"$s\": " \
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

    return data;
}

/* Saves an array of bytes to a file */
ATRA_API
void
atra_util_savefile(const char *path, const char *data, size_t nitems)
{
    FILE *file = fopen(path, "wb");

    if (!file) {
        atra_log_print(LOG_ERROR, "util: Failed to save file \"$s\": " \
                "Unable to open file", path);
        return;
    }
    fwrite(data, sizeof(char), nitems, file);
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
    atra_log_print(LOG_FATAL, "A fatal error has occurred: %s", msg);
}

/* Halts the engine and gracefully exits on memory allocation error */
ATRA_API
void
atra_util_checkalloc(void *ptr)
{
    if (ptr == NULL) {
        atra_util_die("Memory allocation error");
        exit(EXIT_FAILURE);
    }
}

#ifdef __cplusplus
}
#endif
