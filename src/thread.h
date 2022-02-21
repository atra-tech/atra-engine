/*
 *  ATRA ENGINE (2019)
 *
 *  Copyright Robert Saarloos, All rights reserved.
 *
 *  Description:
 *  Cross-platform thread pool system using SDL threads.
 *  Defines types and functions for easily creating and
 *  managing a flexible thread worker pool.
 *
 *  */

#ifndef ATRA_THREAD_H
#define ATRA_THREAD_H

#include "common.h"

struct TPool;
typedef struct TPool TPool;

typedef void (*ThreadFunc)(void *arg);

TPool *atra_tpool_create(uint tcount);
void atra_tpool_delete(TPool *pool);

int atra_tpool_addjob(TPool *pool, ThreadFunc func, void *arg);
void atra_tpool_wait(TPool *pool);

#endif /* ATRA_THREAD_H */
