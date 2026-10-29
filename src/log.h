/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  log.h - Atra Engine logging system
 *
 *  */

#ifndef ATRA_LOG_H
#define ATRA_LOG_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

enum log_level {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
};

/* Initialise the logging system, allocating space for log entries */
ATRA_API void atra_log_start(size_t size, const char *savepath);

/* Shutdown and clean up logging system, saving if autosave is enabled */
ATRA_API void atra_log_stop(void);

/* Writes a log entry to the log record and outputs to terminal. */
ATRA_API void atra_log_print(int mode, const char *msg, ...);

/* Clears the log of all entries */
ATRA_API void atra_log_clear(void);

/* Saves the log to a file, with the option to append to previous writes */
ATRA_API void atra_log_save(const char *path, int append);


#ifdef __cplusplus
}
#endif

#endif /* ATRA_LOG_H */
