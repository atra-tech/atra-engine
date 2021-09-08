/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  state.h - Engine execution state management. Describes a set of flags and
 *          callbacks which are set by the user to execute on certain events.
 *          Provides a state stack which is used to separate input handling,
 *          update and render functions which are different between separate
 *          parts of the program. This allows for earlier states to be returned
 *          to upon popping the stack. For instance: Quitting a game, and
 *          returning to the main menu.
 *
 *  */

#ifndef ATRA_STATE_H
#define ATRA_STATE_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ProgState {
    void (*init)(void);         /* Function run immediately upon creation */
    void (*handle)(void);       /* Function that handles user input */
    void (*update)(u32 dt);     /* Function that updates program state */
    void (*render)(void);       /* Function that draws visuals to the screen */
    void (*stop)(void);         /* Function run before the state is deleted */
} ProgState;

/* Pushes a new state on to the stack with the given callback functions.
 * The `init` and `stop` callbacks are not required and can be set to NULL */
ATRA_API void atra_state_push(
        void (*init)(void),
        void (*handle)(void),
        void (*update)(u32 dt),
        void (*render)(void),
        void (*stop)(void));

/* Calls the handle function of the current state */
ATRA_API void atra_state_handle(void);

/* Calls the update function of the current state */
ATRA_API void atra_state_update(void);

/* Calls the render function of the current state */
ATRA_API void atra_state_render(void);

/* Pops the top state of the stack */
ATRA_API void atra_state_pop(void);

/* Returns a pointer to the top state on the stack */
ATRA_API ProgState * atra_state_top(void);

/* Executes the stop function of all states on the stack and clears it */
ATRA_API void atra_state_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* ATRA_STATE_H */
