/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  state.c - Engine execution state management. Describes a set of flags and
 *          callbacks which are set by the user to execute on certain events.
 *          Provides a state stack which is used to separate input handling,
 *          update and render functions which are different between separate
 *          parts of the program. This allows for earlier states to be returned
 *          to upon popping the stack. For instance: Quitting a game, and
 *          returning to the main menu.
 *
 *  */

#include "atra.h"

#define STATE_STACK_MAX 32

static ProgState state_stack[STATE_STACK_MAX];
static int stack_size = 0;

/* Pushes a new state on to the stack with the given callback functions.
 * The `init` and `stop` callbacks are not required and can be set to NULL */
ATRA_API
void
atra_state_push(
        void (*init)(void),
        void (*handle)(void),
        void (*update)(u32 dt),
        void (*render)(void),
        void (*stop)(void))
{
    ProgState *state = &state_stack[stack_size];

    state->init = init;
    state->handle = handle;
    state->update = update;
    state->render = render;
    state->stop = stop;

    stack_size++;

    if (state->init) {
        state->init();
    }
}

/* Calls the handle function of the current state */
ATRA_API
void
atra_state_handle(void)
{
    atra_engine_update();
    atra_gui_handle();
    state_stack[stack_size - 1].handle();
}

/* Calls the update function of the current state */
ATRA_API
void
atra_state_update(void)
{
    state_stack[stack_size - 1].update(atra_time_getdelta());
}

/* Calls the render function of the current state */
ATRA_API
void
atra_state_render(void)
{
    state_stack[stack_size - 1].render();
    atra_window_swapbuffers();
}

/* Pops the top state of the stack */
ATRA_API
void
atra_state_pop(void)
{
    if (state_stack[stack_size - 1].stop) {
        state_stack[stack_size - 1].stop();
    }
    stack_size--;

    if (stack_size <= 0) {
        atra_engine_stop();
    }
}

/* Returns a pointer to the top state on the stack */
ATRA_API
ProgState *
atra_state_top(void)
{
    return state_stack + stack_size - 1;
}

/* Executes the stop function of all states on the stack and clears it */
ATRA_API
void
atra_state_stop(void)
{
    int i;

    for (i = stack_size - 1; i >= 0; i--) {
        if (state_stack[i].stop) {
            state_stack[i].stop();
        }
    }
    stack_size = 0;
}
