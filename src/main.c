/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  main.c - A temporary entrypoint for debugging purposes.
 *
 *  */

#include "atra.h"

int
main(int argc, char **argv)
{
    ATRA_UNUSED(argc);
    ATRA_UNUSED(argv);

    atra_log_start(1024, NULL);
    atra_log_print(LOG_INFO, "ATRA ENGINE TEST :: %s", ATRA_VERSION);
    atra_log_stop();

    return 0;
}
