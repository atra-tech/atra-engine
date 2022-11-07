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

    HashMap buffers;
    HashMap buffer_paths;

    int send_max;               /* Number of auxiliary sends per source */

    int eax2_support;
    int efx_support;
} audio;

//ATRA_INLINE void atra_audio_al_errorcheck(void);

ATRA_INLINE void atra_audio_loadoutputdevicenames(void); 
ATRA_INLINE void atra_audio_loadinputdevicenames(void);
ATRA_INLINE void atra_audio_loaddefaultdevices(void);

ATRA_INLINE void atra_audio_efx_loadfunctions(void);

ATRA_INLINE size_t atra_audio_buffer_hash(const void *);
ATRA_INLINE int atra_audio_buffer_eq(const void *, const void *);

LPALGENEFFECTS atra_al_geneffects = NULL;
LPALDELETEEFFECTS atra_al_deleteeffects = NULL;
LPALISEFFECT atra_al_iseffect = NULL;
LPALEFFECTI atra_al_effect_i = NULL;
LPALEFFECTIV atra_al_effect_iv = NULL;
LPALEFFECTF atra_al_effect_f = NULL;
LPALEFFECTFV atra_al_effect_fv = NULL;
LPALGETEFFECTI atra_al_geteffect_i = NULL;
LPALGETEFFECTIV atra_al_geteffect_iv = NULL;
LPALGETEFFECTF atra_al_geteffect_f = NULL;
LPALGETEFFECTFV atra_al_geteffect_fv = NULL;

LPALGENFILTERS atra_al_genfilters = NULL;
LPALDELETEFILTERS atra_al_deletefilters = NULL;
LPALISFILTER atra_al_isfilter = NULL;
LPALFILTERI atra_al_filter_i = NULL;
LPALFILTERIV atra_al_filter_iv = NULL;
LPALFILTERF atra_al_filter_f = NULL;
LPALFILTERFV atra_al_filter_fv = NULL;
LPALGETFILTERI atra_al_getfilter_i = NULL;
LPALGETFILTERIV atra_al_getfilter_iv = NULL;
LPALGETFILTERF atra_al_getfilter_f = NULL;
LPALGETFILTERFV atra_al_getfilter_fv = NULL;

LPALGENAUXILIARYEFFECTSLOTS atra_al_genauxiliaryeffectslots = NULL;
LPALDELETEAUXILIARYEFFECTSLOTS atra_al_deleteauxiliaryeffectslots = NULL;
LPALISAUXILIARYEFFECTSLOT atra_al_isauxiliaryeffectslot = NULL;
LPALAUXILIARYEFFECTSLOTI atra_al_auxiliaryeffectslot_i = NULL;
LPALAUXILIARYEFFECTSLOTIV atra_al_auxiliaryeffectslot_iv = NULL;
LPALAUXILIARYEFFECTSLOTF atra_al_auxiliaryeffectslot_f = NULL;
LPALAUXILIARYEFFECTSLOTFV atra_al_auxiliaryeffectslot_fv = NULL;
LPALGETAUXILIARYEFFECTSLOTI atra_al_getauxiliaryeffectslot_i = NULL;
LPALGETAUXILIARYEFFECTSLOTIV atra_al_getauxiliaryeffectslot_iv = NULL;
LPALGETAUXILIARYEFFECTSLOTF atra_al_getauxiliaryeffectslot_f = NULL;
LPALGETAUXILIARYEFFECTSLOTFV atra_al_getauxiliaryeffectslot_fv = NULL;


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
    audio.buffers = atra_hashmap_create(sizeof(uint), sizeof(AudioBuffer), atra_audio_buffer_hash, atra_audio_buffer_eq);
    audio.buffer_paths = atra_hashmap_create(sizeof(char *), sizeof(uint), atra_hashmap_str_hash, atra_hashmap_str_eq);

    /* Check for EAX 2.0 support */
    audio.eax2_support = alIsExtensionPresent("EAX2.0");
    
    if (audio.eax2_support) {
        atra_log_print(LOG_INFO, "audio: Detected EAX 2.0 support");
    } else {
        atra_log_print(LOG_WARN, "audio: EAX 2.0 support not detected");
    }
    
    /* Check for EFX support */
    audio.efx_support = alcIsExtensionPresent(audio.out_device, "ALC_EXT_EFX");

    if (audio.efx_support) {
        atra_log_print(LOG_INFO, "audio: Detected EFX support");
        atra_audio_efx_loadfunctions();
        
        /* Get number of aux sends available to each source */
        alcGetIntegerv(audio.out_device, ALC_MAX_AUXILIARY_SENDS, 1, &audio.send_max);
        atra_log_print(LOG_INFO, "audio: Device supports %d aux sends per source", audio.send_max);
    } else {
        atra_log_print(LOG_ERROR, "audio: EFX support not detected");
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

    atra_hashmap_delete(&audio.buffers, 0);
    atra_hashmap_delete(&audio.buffer_paths, HASH_FREE_KEY);
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


/* Audio buffer functions */

/* Create and initialise a new audio buffer */
ATRA_API
AudioBuffer *
atra_audio_buffer_create(void)
{
    AudioBuffer buffer = {0};

    alGenBuffers(1, &buffer.id);
    atra_hashmap_add(&audio.buffers, &buffer.id, &buffer);
    
    return atra_hashmap_get(&audio.buffers, &buffer.id);
}

/* Delete an audio buffer and it's associated audio data */
ATRA_API
void
atra_audio_buffer_delete(AudioBuffer *buf)
{
    uint id = buf->id;

    alDeleteBuffers(1, &buf->id);
    atra_hashmap_remove(&audio.buffers, &id, 0);
    atra_hashmap_remove(&audio.buffer_paths, &buf->file_path, HASH_FREE_KEY);
    free(buf->file_path);
}

/* Load the given file into a buffer */
ATRA_API
void atra_audio_buffer_loadfile(AudioBuffer *buf, const char *path)
{
    SDL_AudioSpec spec;
    char *key;
    u8 *buffer;
    uint format;
    uint size;

    SDL_LoadWAV(path, &spec, &buffer, &size);

    buf->frequency = spec.freq;
    buf->size = size;
    buf->channels = spec.channels;

    switch (spec.format) {
    case AUDIO_S8:
    case AUDIO_U8:
        buf->bit_depth = 8;
        break;
    case AUDIO_S16:
    case AUDIO_U16:
        buf->bit_depth = 16;
        break;
    case AUDIO_S32:
    case AUDIO_F32:
        buf->bit_depth = 32;
        break;
    default:
        atra_log_print(LOG_WARN, "error: Unknown audio format in file: %s", path);
        SDL_FreeWAV(buffer);
        return;
    }

    if (buf->bit_depth == 8) {
        format = DECIDE(buf->channels > 1, AL_FORMAT_STEREO8, AL_FORMAT_MONO8);
    } else if (buf->bit_depth == 16) {
        format = DECIDE(buf->channels > 1, AL_FORMAT_STEREO16, AL_FORMAT_MONO16);
    } else if (buf->bit_depth == 32) {
        format = DECIDE(buf->channels > 1, AL_FORMAT_STEREO_FLOAT32, AL_FORMAT_MONO_FLOAT32);
    } else {
        format = AL_NONE;
    }
    alBufferData(buf->id, format, buffer, size, spec.freq);
    SDL_FreeWAV(buffer);

    key = atra_util_strdup(path);
    buf->file_path = atra_util_strdup(key);
    atra_hashmap_add(&audio.buffer_paths, &key, &buf->id);
}

/* Load raw PCM data into the given buffer */
ATRA_API
void
atra_audio_buffer_loadrawdata(AudioBuffer *buf, uint format, const void *data, size_t size, uint freq)
{
    alBufferData(buf->id, format, data, size, freq);
}

/* Retrieve an audio buffer by its ID */
ATRA_API
AudioBuffer *
atra_audio_buffer_get(uint id)
{
    return atra_hashmap_get(&audio.buffers, &id);
}

/* Retrieve an audio buffer's ID by it's data file path */
ATRA_API
uint
atra_audio_buffer_getid_bypath(const char *path)
{
    AudioBuffer *buffer;
    uint *p_id;
    uint id;

    p_id = atra_hashmap_get(&audio.buffer_paths, &path);
    
    if (p_id == NULL) {
        buffer = atra_audio_buffer_create();
        atra_audio_buffer_loadfile(buffer, path);
        id = buffer->id;
    } else {
        id = *p_id;
    }

    return id;
}

ATRA_API
void
atra_al_buffer_i(AudioBuffer *buf, int param, const int value)
{
    alBufferi(buf->id, param, value);
}

ATRA_API
void
atra_al_buffer_iv(AudioBuffer *buf, int param, const int *values)
{
    alBufferiv(buf->id, param, values);
}

ATRA_API
void
atra_al_buffer_f(AudioBuffer *buf, int param, const float value)
{
    alBufferf(buf->id, param, value);
}

ATRA_API
void
atra_al_buffer_fv(AudioBuffer *buf, int param, const float *values)
{
    alBufferfv(buf->id, param, values);
}

ATRA_API
void
atra_al_getbuffer_i(AudioBuffer *buf, int param, int *value)
{
    alGetBufferi(buf->id, param, value);
}

ATRA_API
void
atra_al_getbuffer_iv(AudioBuffer *buf, int param, int *values)
{
    alGetBufferiv(buf->id, param, values);
}

ATRA_API
void
atra_al_getbuffer_f(AudioBuffer *buf, int param, float *value)
{
    alGetBufferf(buf->id, param, value);
}

ATRA_API
void
atra_al_getbuffer_fv(AudioBuffer *buf, int param, float *values)
{
    alGetBufferfv(buf->id, param, values);
}


/* Audio source functions */

/* Create a new, default audio source */
ATRA_API
AudioSource atra_audio_source_create(void)
{
    AudioSource source = {0};

    /* TODO: Initialise source defaults */

    alGenSources(1, &source.id);

    return source;
}

/* Delete an audio source */
ATRA_API
void
atra_audio_source_delete(AudioSource *src)
{
    alDeleteSources(1, &src->id);
}

/* Set the world-space position of an audio source */
ATRA_API
void
atra_audio_source_setpos(AudioSource *src, const vec3 pos)
{
    alSourcefv(src->id, AL_POSITION, pos);
}

/* Set the velocity vector of an audio source */
ATRA_API
void
atra_audio_source_setvel(AudioSource *src, const vec3 vel)
{
    alSourcefv(src->id, AL_VELOCITY, vel);
}

/* Set the direction vector of an audio source */
ATRA_API
void
atra_audio_source_setdir(AudioSource *src, const vec3 dir)
{
    alSourcefv(src->id, AL_DIRECTION, dir);
}

/* Set the pitch multiplier of an audio source */
ATRA_API
void
atra_audio_source_setpitch(AudioSource *src, float pitch)
{
    alSourcef(src->id, AL_PITCH, pitch);
}

/* Set the gain of an audio source */
ATRA_API
void
atra_audio_source_setgain(AudioSource *src, float gain)
{
    alSourcef(src->id, AL_GAIN, gain);
}

/* Set the max radius of an audio source */
ATRA_API
void
atra_audio_source_setradius(AudioSource *src, float radius)
{
    alSourcef(src->id, AL_MAX_DISTANCE, radius);
}

/* Set the rolloff of an audio source */
ATRA_API
void
atra_audio_source_setrolloff(AudioSource *src, float rolloff)
{
    alSourcef(src->id, AL_ROLLOFF_FACTOR, rolloff);
}

/* Set the reference distance of an audio source */
ATRA_API
void
atra_audio_source_setrefdist(AudioSource *src, float refdist)
{
    alSourcef(src->id, AL_REFERENCE_DISTANCE, refdist);
}

/* Set the minimum gain of an audio source */
ATRA_API
void
atra_audio_source_setmingain(AudioSource *src, float gain)
{
    alSourcef(src->id, AL_MIN_GAIN, gain);
}

/* Set the maximum gain of an audio source */
ATRA_API
void
atra_audio_source_setmaxgain(AudioSource *src, float gain)
{
    alSourcef(src->id, AL_MAX_GAIN, gain);
}

/* Set the outer gain of a source's sound cone */
ATRA_API
void
atra_audio_source_setoutergain(AudioSource *src, float gain)
{
    alSourcef(src->id, AL_CONE_OUTER_GAIN, gain);
}

/* Set the inner angle of a source's sound cone */
ATRA_API
void
atra_audio_source_setinnerangle(AudioSource *src, float angle)
{
    alSourcef(src->id, AL_CONE_INNER_ANGLE, angle);
}

/* Set the outer angle of a source's sound cone */
ATRA_API
void
atra_audio_source_setouterangle(AudioSource *src, float angle)
{
    alSourcef(src->id, AL_CONE_OUTER_ANGLE, angle);
}

/* Set whether or not the source's position is relative to the listener */
ATRA_API
void
atra_audio_source_setlistenerrelativity(AudioSource *src, int relative)
{
    alSourcei(src->id, AL_SOURCE_RELATIVE, relative);
}

/* Set the type of an audio source (STATIC/STREAMING) */
ATRA_API
void
atra_audio_source_settype(AudioSource *src, uint type)
{

    alSourcei(src->id, AL_SOURCE_TYPE, type);
}

/* Set an audio source to looping mode */
ATRA_API
void
atra_audio_source_enablelooping(AudioSource *src)
{
    alSourcei(src->id, AL_LOOPING, AL_TRUE);
}

/* Set an audio source to looping mode */
ATRA_API
void
atra_audio_source_disablelooping(AudioSource *src)
{
    alSourcei(src->id, AL_LOOPING, AL_FALSE);
}

/* Play sound data through an audio source */
ATRA_API
void
atra_audio_source_play(AudioSource *src)
{
    alSourcePlay(src->id);
}

/* Pause playback of an audio source */
ATRA_API
void
atra_audio_source_pause(AudioSource *src)
{
    alSourcePause(src->id);
}

/* Stop playback of an audio source */
ATRA_API
void
atra_audio_source_stop(AudioSource *src)
{
    alSourceStop(src->id);
}

/* Stop playback of an audio source and reset it to the initial state */
ATRA_API
void
atra_audio_source_rewind(AudioSource *src)
{
    alSourceRewind(src->id);
}

/* Attaches a buffer to the audio source */
ATRA_API
void
atra_audio_source_attachbuffer(AudioSource *src, AudioBuffer *buf)
{
    alSourcei(src->id, AL_BUFFER, buf->id);
}

/* Queue a number of audio buffers for the source to play in sequence */
ATRA_API
void
atra_audio_source_queuebuffers(AudioSource *src, uint n, AudioBuffer *bufs)
{
    uint *buffer_ids;
    uint i;

    buffer_ids = malloc(n * sizeof(uint));

    for (i = 0; i < n; i++) {
        buffer_ids[i] = bufs[i].id;
    }
    alSourceQueueBuffers(src->id, n, buffer_ids);
    free(buffer_ids);
}

/* Unqueue a set of audio buffers from a source.
 * Only completes when all buffers can be removed from queue */
ATRA_API
void
atra_audio_source_unqueuebuffers(AudioSource *src, uint n, AudioBuffer *bufs)
{
    uint *buffer_ids;
    uint i;

    buffer_ids = malloc(n * sizeof(uint));

    for (i = 0; i < n; i++) {
        buffer_ids[i] = bufs[i].id;
    }
    alSourceUnqueueBuffers(src->id, n, buffer_ids);
    free(buffer_ids);
}

ATRA_API
void
atra_al_source_i(AudioSource *src, int param, const int value)
{
    alSourcei(src->id, param, value);
}

ATRA_API
void
atra_al_source_iv(AudioSource *src, int param, const int *values)
{
    alSourceiv(src->id, param, values);
}

ATRA_API
void
atra_al_source_f(AudioSource *src, int param, const float value)
{
    alSourcef(src->id, param, value);
}

ATRA_API
void
atra_al_source_fv(AudioSource *src, int param, const float *values)
{
    alSourcefv(src->id, param, values);
}

ATRA_API
void
atra_al_getsource_i(AudioSource *src, int param, int *value)
{
    alGetSourcei(src->id, param, value);
}

ATRA_API
void
atra_al_getsource_iv(AudioSource *src, int param, int *values)
{
    alGetSourcei(src->id, param, values);
}

ATRA_API
void
atra_al_getsource_f(AudioSource *src, int param, float *value)
{
    alGetSourcef(src->id, param, value);
}

ATRA_API
void
atra_al_getsource_fv(AudioSource *src, int param, float *values)
{
    alGetSourcefv(src->id, param, values);
}


/* Audio listener functions */

/* Set the master gain of the audio context's listener */
ATRA_API
void
atra_audio_listener_setgain(float gain)
{
    alListenerf(AL_GAIN, gain);
}

/* Set the world-space position of the audio context's listener */
ATRA_API
void
atra_audio_listener_setpos(const vec3 pos)
{
    alListenerfv(AL_POSITION, pos);
}

/* Set the velocity vector of the audio context's listener */
ATRA_API
void
atra_audio_listener_setvel(const vec3 vel)
{
    alListenerfv(AL_VELOCITY, vel);
}

/* Set the orientation of the audio context's listener (at, up vectors) */
ATRA_API
void
atra_audio_listener_setorientation(const vec3 at, const vec3 up)
{
    float data[6] = {
        at[0], at[1], at[2],
        up[0], up[1], up[2]
    };
    alListenerfv(AL_ORIENTATION, data);
}

ATRA_API
void
atra_al_listener_i(int param, const int value)
{
    alListeneri(param, value);
}

ATRA_API
void atra_al_listener_iv(int param, const int *values)
{
    alListeneriv(param, values);
}

ATRA_API
void
atra_al_listener_f(int param, const float value)
{
    alListenerf(param, value);
}

ATRA_API
void
atra_al_listener_fv(int param, const float *values)
{
    alListenerfv(param, values);
}

ATRA_API
void
atra_al_getlistener_i(int param, int *value)
{
    alGetListeneri(param, value);
}

ATRA_API
void
atra_al_getlistener_iv(int param, int *values)
{
    alGetListeneriv(param, values);
}

ATRA_API
void
atra_al_getlistener_f(int param, float *value)
{
    alGetListenerf(param, value);
}

ATRA_API
void
atra_al_getlistener_fv(int param, float *values)
{
    alGetListenerfv(param, values);
}


ATRA_INLINE
void
atra_audio_efx_loadfunctions(void)
{
    atra_al_geneffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
    atra_al_deleteeffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
    atra_al_iseffect = (LPALISEFFECT)alGetProcAddress("alIsEffect");
    atra_al_effect_i = (LPALEFFECTI)alGetProcAddress("alEffecti");
    atra_al_effect_iv = (LPALEFFECTIV)alGetProcAddress("alEffectiv");
    atra_al_effect_f = (LPALEFFECTF)alGetProcAddress("alEffectf");
    atra_al_effect_fv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
    atra_al_geteffect_i = (LPALGETEFFECTI)alGetProcAddress("alGetEffecti");
    atra_al_geteffect_iv = (LPALGETEFFECTIV)alGetProcAddress("alGetEffectiv");
    atra_al_geteffect_f = (LPALGETEFFECTF)alGetProcAddress("alGetEffectf");
    atra_al_geteffect_fv = (LPALGETEFFECTFV)alGetProcAddress("alGetEffectfv");

    atra_al_genfilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
    atra_al_deletefilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
    atra_al_isfilter = (LPALISFILTER)alGetProcAddress("alIsFilter");
    atra_al_filter_i = (LPALFILTERI)alGetProcAddress("alFilteri");
    atra_al_filter_iv = (LPALFILTERIV)alGetProcAddress("alFilteriv");
    atra_al_filter_f = (LPALFILTERF)alGetProcAddress("alFilterf");
    atra_al_filter_fv = (LPALFILTERFV)alGetProcAddress("alFilterfv");
    atra_al_getfilter_i = (LPALGETFILTERI)alGetProcAddress("alGetFilteri");
    atra_al_getfilter_iv = (LPALGETFILTERIV)alGetProcAddress("alGetFilteriv");
    atra_al_getfilter_f = (LPALGETFILTERF)alGetProcAddress("alGetFilterf");
    atra_al_getfilter_fv = (LPALGETFILTERFV)alGetProcAddress("alGetFilterfv");

    atra_al_genauxiliaryeffectslots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
    atra_al_deleteauxiliaryeffectslots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    atra_al_isauxiliaryeffectslot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress("alIsAuxiliaryEffectSlot");
    atra_al_auxiliaryeffectslot_i = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
    atra_al_auxiliaryeffectslot_iv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
    atra_al_auxiliaryeffectslot_f = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
    atra_al_auxiliaryeffectslot_fv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
    atra_al_getauxiliaryeffectslot_i = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
    atra_al_getauxiliaryeffectslot_iv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
    atra_al_getauxiliaryeffectslot_f = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
    atra_al_getauxiliaryeffectslot_fv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress("alGetAuxiliaryEffectSlotfv");
}

ATRA_INLINE
size_t
atra_audio_buffer_hash(const void *a)
{
    const uint * const id = a;
    size_t hash = *id;

    return hash; 
}

ATRA_INLINE
int
atra_audio_buffer_eq(const void *a, const void *b)
{
    const uint * const id_a = a;
    const uint * const id_b = b;

    return (*id_a == *id_b);
}

/*
ATRA_INLINE
void
atra_audio_al_errorcheck(void)
{
    ALenum error;

    if ((error = alGetError()) != AL_NO_ERROR) {
        switch (error) {
        case AL_INVALID_NAME:
            atra_log_print(LOG_ERROR, "audio: AL: Invalid name");
            break;
        case AL_INVALID_ENUM:
            atra_log_print(LOG_ERROR, "audio: AL: Invalid enum");
            break;
        case AL_INVALID_VALUE:
            atra_log_print(LOG_ERROR, "audio: AL: Invalid value");
            break;
        case AL_INVALID_OPERATION:
            atra_log_print(LOG_ERROR, "audio: AL: Invalid operation");
            break;
        case AL_OUT_OF_MEMORY:
            atra_log_print(LOG_ERROR, "audio: AL: Out of memory");
            break;
        default:
            atra_log_print(LOG_ERROR, "audio: AL: Unknown error code");
            break;
        }
    }
}
*/
