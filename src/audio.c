/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved
 *
 *  audio.h - A set of tools for loading, manipulating and playing audio
 *
 * */

#include "audio.h"
#include "util.h"
#include "log.h"

/* Collection of all data relevant to audio engine operation */
static struct ALState {
    char *default_out_name;
    char *default_in_name;

    char **out_names;
    char **in_names;
    uint out_count;
    uint in_count;

    AudioDevice *out_device;
    AudioDevice *in_device;

    AudioContext *context;

    int eax2_support;
} audio;

ATRA_INLINE void atra_audio_loadoutputdevicenames(void); 
ATRA_INLINE void atra_audio_loadinputdevicenames(void);
ATRA_INLINE void atra_audio_loaddefaultdevices(void);

/* Set up initial audio subsystem state, and load the default devices */
ATRA_API
void
atra_audio_init(void)
{
    AudioContext *context;

    /* Retrieve audio device names and intialise default devices */
    atra_audio_loadoutputdevicenames();
    atra_audio_loadinputdevicenames(); 
    atra_audio_loaddefaultdevices();

    /* Initialise context */
    context = alcCreateContext(audio.out_device, NULL);
    alcMakeContextCurrent(context);

    /* Check for EAX 2.0 support */
    audio.eax2_support = alIsExtensionPresent("EAX2.0");
    
    if (audio.eax2_support) {
        atra_log_print(LOG_INFO, "audio: Detected EAX 2.0 support");
    }
    
}

/* Shut down the audio subsystem */
ATRA_API
void
atra_audio_stop(void)
{
    alcMakeContextCurrent(NULL);
    alcDestroyContext(audio.context);
    audio.context = NULL;

    if (audio.out_device != NULL) {
        atra_audio_device_close(audio.out_device);
    }

    if (audio.in_device != NULL) {
        atra_audio_device_close(audio.in_device);
    }
}

/* Populates the audio subsystem output device name list */
ATRA_INLINE
void
atra_audio_loadoutputdevicenames(void)
{
    const char *out_devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    const char *device;

    int device_count = 0;
    int length;

    if (*out_devices == '\0') {
        atra_log_print(LOG_WARN, "audio: Unable to find any audio output devices");
        return;
    }

    /* Get number of devices */
    for (device = out_devices; ; device++) {
        if (*device == '\0') {
            device_count++;

            if (*(device + 1) == '\0') {
                break;
            }
        }
    }
    audio.out_names = malloc(device_count * sizeof(char *));

    for (device = out_devices; *device != '\0'; device += length) {
        length = strlen(device) + 1;
        audio.out_names[audio.out_count++] = atra_util_strdup(device);
        atra_log_print(LOG_INFO, "audio: Detected output device: %s", device);
    }
}

/* Populates the audio subsystem input device name list */
ATRA_INLINE
void
atra_audio_loadinputdevicenames(void)
{
    const char *in_devices = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
    const char *device;

    int device_count = 0;
    int length;

    if (*in_devices == '\0') {
        atra_log_print(LOG_WARN, "audio: Unable to find any audio capture devices");
        return;
    }

    /* Get number of devices */
    for (device = in_devices; ; device++) {
        if (*device == '\0') {
            device_count++;

            if (*(device + 1) == '\0') {
                break;
            }
        }
    }
    audio.in_names = malloc(device_count * sizeof(char *));

    for (device = in_devices; *device != '\0'; device += length) {
        length = strlen(device) + 1;
        audio.in_names[audio.in_count++] = atra_util_strdup(device);
        atra_log_print(LOG_INFO, "audio: Detected capture device: %s", device);
    }
}

/* Opens the default audio device */
ATRA_INLINE
void
atra_audio_loaddefaultdevices(void)
{
    const char *out = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    const char *in = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);

    if (*out == '\0') {
        atra_log_print(LOG_WARN, "audio: Unable to detect a default audio output device");
    } else {
        audio.out_device = atra_audio_device_open(out);
    }

    if (*in == '\0') {
        atra_log_print(LOG_WARN, "audio: Unable to detect a default audio capture device");
    } else {
        audio.in_device = atra_audio_device_open(in);
    }
}

/* Opens the given audio device name and returns a pointer to the device buffer */
ATRA_API
AudioDevice *
atra_audio_device_open(const char *name)
{
    return alcOpenDevice(name);
}

/* Closes the given device buffer pointer */
ATRA_API
int
atra_audio_device_close(AudioDevice *device)
{
    return alcCloseDevice(device);
}
