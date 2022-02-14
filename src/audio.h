/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved
 *
 *  audio.h - A set of tools for loading, manipulating and playing audio
 *
 * */

#ifndef ATRA_AUDIO_H
#define ATRA_AUDIO_H

#include <AL/al.h>
#include <AL/alc.h>

#include "common.h"

typedef ALCdevice AudioDevice;
typedef ALCcontext AudioContext;

/* An analog to the OpenAL buffer object.
 * Stores raw PCM data from which bound `source` objects get their data.
 */
typedef struct AudioBuffer {
    uint buffer_id;       /* OpenAL buffer object ID */

} AudioBuffer;

/* The world-space point from which sounds originated.
 * Sources have buffers bound to them, where their audio data is obtained
 */
typedef struct AudioSource {
    float pitch;            /* Pitch multiplier */
    float gain;             /* Source gain */
    float max_radius;       /* Maximum distance */
    float rolloff;          /*  */
    float ref_dist;         /**/
    float min_gain;         /**/
    float max_gain;         /**/

    float cone_outer_gain;  /**/
    float cone_inner_angle; /**/
    float cone_outer_angle; /**/

    vec3 position;          /**/
    vec3 velocity;          /**/
    vec3 direction;         /**/

    int source_relative;    /**/
    int source_type;        /**/
    int source_state;       /**/

    int looping;            /**/
    int buffer_id;          /**/
    int buffers_queued;     /**/
    int buffers_processed;  /**/
} AudioSource;

/*
    Features:
        Find and store audio device names
        Selection of output/input device to use
        Create device handle from device name
        Visual representation of sound coverage (debug info)

*/

/*
    TODO:
        Call audio initialisation in engine startup

*/

/* Set up initial audio subsystem state, and load the default devices */
ATRA_API void atra_audio_init(void);

/* Shut down the audio subsystem */
ATRA_API void atra_audio_stop(void);

/* Opens the given audio device name and returns a pointer to the device buffer */
ATRA_API AudioDevice * atra_audio_device_open(const char *device_id);

/* Closes the given device buffer pointer */
ATRA_API int atra_audio_device_close(AudioDevice *device);

ATRA_API 

#endif /* ATRA_AUDIO_H */
