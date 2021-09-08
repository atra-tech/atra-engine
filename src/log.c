/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  log.c - Atra Engine logging system
 *
 *  */

#include "atra.h"

#include <string.h>
#include <stdarg.h>
#include <time.h>

#define LOG_MAX_ENTRY_SIZE  1024
#define LOG_DEFAULT_LEVEL   LOG_INFO

#ifdef _WIN32
    #define LOG_ATTR(n)
#else
    #define LOG_ATTR(n) printf("\033[%dm", n)
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum log_attr {
    ATTR_NONE       = 0,
    ATTR_BOLDON     = 1,
    ATTR_BLINKON    = 5,
    ATTR_INVERT     = 7,
    ATTR_BOLDOFF    = 22,
    ATTR_BLINKOFF   = 25
};

enum log_bgcolor {
    BGCOLOR_RED     = 41,
    BGCOLOR_GREEN   = 42,
    BGCOLOR_YELLOW  = 43,
    BGCOLOR_BLUE    = 44,
    BGCOLOR_MAGENTA = 45,
    BGCOLOR_CYAN    = 46,
    BGCOLOR_WHITE   = 47,
    BGCOLOR_DEFAULT = 48
};

enum log_fgcolor {
    FGCOLOR_RED     = 91,
    FGCOLOR_GREEN   = 92,
    FGCOLOR_YELLOW  = 93,
    FGCOLOR_BLUE    = 94,
    FGCOLOR_MAGENTA = 95,
    FGCOLOR_CYAN    = 96,
    FGCOLOR_WHITE   = 97,
    FGCOLOR_DEFAULT = 98
};

/* Struct for keeping record of entry data */
typedef struct LogRecord {
    int ready;
    char **entries;         /* Stored record of log entries */
    size_t entry_count;     /* Number of entries */
    size_t size;            /* Size cap of entry list */
    int autosave;           /* Log overflow autosave flag */
    char *savepath;         /* File path to save log records to */
} LogRecord;

static const char *log_prefixes[] = {
    "[DEBUG] ",
    "[INFO] ",
    "[WARN] ",
    "[ERROR] ",
    "[FATAL] "
};

static const size_t log_prefix_sizes[] = {
    8, 7, 7, 8, 8
};

static const int log_mode_colors[] = {
    FGCOLOR_CYAN,       /* DEBUG */
    FGCOLOR_WHITE,      /* INFO */
    FGCOLOR_YELLOW,     /* WARN */
    FGCOLOR_RED,        /* ERROR */
    FGCOLOR_MAGENTA     /* FATAL */
};

static const char time_format[] = "[%02d:%02d:%02d] ";
static const size_t time_size = 11;  /* Timestamp output will always be this size */

static LogRecord log_rec = {0};


/* Initialise the logging system, allocating space for log entries */
ATRA_API
void
atra_log_start(size_t size, const char *savepath)
{
    log_rec.entries = malloc(size * sizeof(char *));
    log_rec.entry_count = 0;
    log_rec.size = size;
    log_rec.autosave = savepath != NULL;

    if (log_rec.autosave) {
        atra_util_deletefile(savepath);
        log_rec.savepath = atra_util_strdup(savepath);
    } else {
        log_rec.savepath = NULL;
    }
    log_rec.ready = 1;
}

/* Shutdown and clean up logging system, saving if autosave is enabled */
ATRA_API
void
atra_log_stop(void)
{
    size_t i;

    if (log_rec.autosave) {
        atra_log_save(log_rec.savepath, 1);
    }

    for (i = 0; i < log_rec.entry_count; i++) {
        free(log_rec.entries[i]);
    }
    free(log_rec.entries);
    log_rec.ready = 0;
}

/* Writes a log entry to the log record and outputs to terminal. */
ATRA_API
void
atra_log_print(int mode, const char *msg, ...)
{
    size_t msg_len, prefix_len;
    va_list args, args_cpy;
    time_t rawtime;
    struct tm *tinfo;
    char *head, *output = NULL;

    if (!log_rec.ready) {
        LOG_ATTR(ATTR_BOLDON);
        LOG_ATTR(FGCOLOR_YELLOW);
        printf("[WARN] Logging system was invoked before initialization. " \
                "Initializing with default arguments...\n");
        LOG_ATTR(ATTR_NONE);
        atra_log_start(256, NULL);
    }

    /* Obtain formatted timestamp */
    time(&rawtime);
    tinfo = localtime(&rawtime);

    /* Get formatted output length */
    va_start(args, msg);
    va_copy(args_cpy, args);
    msg_len = vsnprintf(output, 0, msg, args) + 1;
    va_end(args);

    /* Calculate final buffer size */
    prefix_len = log_prefix_sizes[mode];
    
    /* Allocate, and write formatted message to buffer.*/
    output = malloc((time_size + prefix_len + msg_len + 1) * sizeof(char));
    head = output;
    sprintf(head, time_format, tinfo->tm_hour, tinfo->tm_min, tinfo->tm_sec);
    head += time_size;
    memcpy(head, log_prefixes[mode], prefix_len);
    head += prefix_len;

    va_start(args_cpy, msg);
    vsnprintf(head, msg_len, msg, args_cpy);
    va_end(args_cpy);
    head += msg_len;
    *head = '\0';

    /* Print result to standard output */
    LOG_ATTR(ATTR_BOLDON);
    LOG_ATTR(log_mode_colors[mode]);
    printf("%s\n", output);
    LOG_ATTR(FGCOLOR_DEFAULT);
    LOG_ATTR(ATTR_NONE);

    /* Save buffer to log record */
    log_rec.entries[log_rec.entry_count++] = output;

    if (log_rec.entry_count >= log_rec.size) {
        if (log_rec.autosave) {
            atra_log_save(log_rec.savepath, 1);
        }
        atra_log_clear();
    }
}

/* Clears the log of all entries */
ATRA_API
void
atra_log_clear(void)
{
    size_t i;

    for (i = 0; i < log_rec.entry_count; i++) {
        free(log_rec.entries[i]);
    }
    log_rec.entry_count = 0;
}

/* Saves the log to a file, with the option to append to previous writes */
ATRA_API
void
atra_log_save(const char *path, int append)
{
    size_t i;
    FILE *file;

    if (append) {
        file = fopen(path, "a");
    } else {
        file = fopen(path, "w");
    }

    if (file == NULL) {
        atra_log_print(LOG_ERROR, "log: Error writing log file \"%s\": " \
                "Unable to open log file", path);
        return;
    } else {
        for (i = 0; i < log_rec.size; i++) {
            if (log_rec.entries[i] == NULL)
                break;
            fprintf(file, "%s\n", log_rec.entries[i]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

#ifdef __cplusplus
}
#endif
