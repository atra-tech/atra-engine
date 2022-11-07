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
#include <AL/alext.h>
#include <AL/efx.h>
#include <SDL2/SDL_audio.h>

#include "common.h"

enum atra_audio_effects {
    ATRA_SFX_NULL                       = AL_EFFECT_NULL,
    ATRA_SFX_EAXREVERB                  = AL_EFFECT_EAXREVERB,
    ATRA_SFX_REVERB                     = AL_EFFECT_REVERB,
    ATRA_SFX_CHORUS                     = AL_EFFECT_CHORUS,
    ATRA_SFX_DISTORTION                 = AL_EFFECT_DISTORTION,
    ATRA_SFX_ECHO                       = AL_EFFECT_ECHO,
    ATRA_SFX_FLANGER                    = AL_EFFECT_FLANGER,
    ATRA_SFX_FREQSHIFTER                = AL_EFFECT_FREQUENCY_SHIFTER,
    ATRA_SFX_VOCALMORPHER               = AL_EFFECT_VOCAL_MORPHER,
    ATRA_SFX_PITCHSHIFTER               = AL_EFFECT_PITCH_SHIFTER,
    ATRA_SFX_RINGMOD                    = AL_EFFECT_RING_MODULATOR,
    ATRA_SFX_AUTOWAH                    = AL_EFFECT_AUTOWAH,
    ATRA_SFX_COMPRESSOR                 = AL_EFFECT_COMPRESSOR,
    ATRA_SFX_EQUALIZER                  = AL_EFFECT_EQUALIZER
};

enum atra_audio_eaxreverb_params {
    ATRA_SFX_EAXREVERB_DENSITY          = AL_EAXREVERB_DENSITY,
    ATRA_SFX_EAXREVERB_DIFFUSION        = AL_EAXREVERB_DIFFUSION,
    ATRA_SFX_EAXREVERB_GAIN             = AL_EAXREVERB_GAIN,
    ATRA_SFX_EAXREVERB_GAINHF           = AL_EAXREVERB_GAINHF,
    ATRA_SFX_EAXREVERB_GAINLF           = AL_EAXREVERB_GAINLF,
    ATRA_SFX_EAXREVERB_DECAY_TIME       = AL_EAXREVERB_DECAY_TIME,
    ATRA_SFX_EAXREVERB_DECAY_HFRATIO    = AL_EAXREVERB_DECAY_HFRATIO,
    ATRA_SFX_EAXREVERB_DECAY_LFRATIO    = AL_EAXREVERB_DECAY_LFRATIO,
    ATRA_SFX_EAXREVERB_REFL_GAIN        = AL_EAXREVERB_REFLECTIONS_GAIN,
    ATRA_SFX_EAXREVERB_REFL_DELAY       = AL_EAXREVERB_REFLECTIONS_DELAY,
    ATRA_SFX_EAXREVERB_REFL_PAN         = AL_EAXREVERB_REFLECTIONS_PAN,
    ATRA_SFX_EAXREVERB_LATE_GAIN        = AL_EAXREVERB_LATE_REVERB_GAIN,
    ATRA_SFX_EAXREVERB_LATE_DELAY       = AL_EAXREVERB_LATE_REVERB_DELAY,
    ATRA_SFX_EAXREVERB_LATE_PAN         = AL_EAXREVERB_LATE_REVERB_PAN,
    ATRA_SFX_EAXREVERB_ECHO_TIME        = AL_EAXREVERB_ECHO_TIME,
    ATRA_SFX_EAXREVERB_ECHO_DEPTH       = AL_EAXREVERB_ECHO_DEPTH,
    ATRA_SFX_EAXREVERB_MOD_TIME         = AL_EAXREVERB_MODULATION_TIME,
    ATRA_SFX_EAXREVERB_MOD_DEPTH        = AL_EAXREVERB_MODULATION_DEPTH,
    ATRA_SFX_EAXREVERB_AIR_GAINHF       = AL_EAXREVERB_AIR_ABSORPTION_GAINHF,
    ATRA_SFX_EAXREVERB_HFREFERENCE      = AL_EAXREVERB_HFREFERENCE,
    ATRA_SFX_EAXREVERB_LFREFERENCE      = AL_EAXREVERB_LFREFERENCE,
    ATRA_SFX_EAXREVERB_ROOM_ROLLOFF     = AL_EAXREVERB_ROOM_ROLLOFF_FACTOR,
    ATRA_SFX_EAXREVERB_DECAYHF_LIMIT    = AL_EAXREVERB_DECAY_HFLIMIT
};

enum atra_audio_reverb_params {
    ATRA_SFX_REVERB_DENSITY             = AL_REVERB_DENSITY,
    ATRA_SFX_REVERB_DIFFUSION           = AL_REVERB_DIFFUSION,
    ATRA_SFX_REVERB_GAIN                = AL_REVERB_GAIN,
    ATRA_SFX_REVERB_GAINHF              = AL_REVERB_GAINHF,
    ATRA_SFX_REVERB_DECAY_TIME          = AL_REVERB_DECAY_TIME,
    ATRA_SFX_REVERB_DECAY_HFRATIO       = AL_REVERB_DECAY_HFRATIO,
    ATRA_SFX_REVERB_REFL_GAIN           = AL_REVERB_REFLECTIONS_GAIN,
    ATRA_SFX_REVERB_REFL_DELAY          = AL_REVERB_REFLECTIONS_DELAY,
    ATRA_SFX_REVERB_LATE_GAIN           = AL_REVERB_LATE_REVERB_GAIN,
    ATRA_SFX_REVERB_LATE_DELAY          = AL_REVERB_LATE_REVERB_DELAY,
    ATRA_SFX_REVERB_AIR_GAINHF          = AL_REVERB_AIR_ABSORPTION_GAINHF,
    ATRA_SFX_REVERB_ROOM_ROLLOFF        = AL_REVERB_ROOM_ROLLOFF_FACTOR,
    ATRA_SFX_REVERB_DECAY_HFLIMIT       = AL_REVERB_DECAY_HFLIMIT
};

enum atra_audio_chorus_params {
    ATRA_SFX_CHORUS_WAVEFORM            = AL_CHORUS_WAVEFORM,
    ATRA_SFX_CHORUS_PHASE               = AL_CHORUS_PHASE,
    ATRA_SFX_CHORUS_RATE                = AL_CHORUS_RATE,
    ATRA_SFX_CHORUS_DEPTH               = AL_CHORUS_DEPTH,
    ATRA_SFX_CHORUS_FEEDBACK            = AL_CHORUS_FEEDBACK,
    ATRA_SFX_CHORUS_DELAY               = AL_CHORUS_DELAY
};

enum atra_audio_distortion_params {
    ATRA_SFX_DISTORTION_EDGE            = AL_DISTORTION_EDGE,
    ATRA_SFX_DISTORTION_GAIN            = AL_DISTORTION_GAIN,
    ATRA_SFX_DISTORTION_LOWPASS_CUTOFF  = AL_DISTORTION_LOWPASS_CUTOFF,
    ATRA_SFX_DISTORTION_EQCENTER        = AL_DISTORTION_EQCENTER,
    ATRA_SFX_DISTORTION_EQBANDWIDTH     = AL_DISTORTION_EQBANDWIDTH
};

enum atra_audio_echo_params {
    ATRA_SFX_ECHO_DELAY                 = AL_ECHO_DELAY,
    ATRA_SFX_ECHO_LRDELAY               = AL_ECHO_LRDELAY,
    ATRA_SFX_ECHO_DAMPING               = AL_ECHO_DAMPING,
    ATRA_SFX_ECHO_FEEDBACK              = AL_ECHO_FEEDBACK,
    ATRA_SFX_ECHO_SPREAD                = AL_ECHO_SPREAD
};

enum atra_audio_flanger_params {
    ATRA_SFX_FLANGER_WAVEFORM           = AL_FLANGER_WAVEFORM,
    ATRA_SFX_FLANGER_PHASE              = AL_FLANGER_PHASE,
    ATRA_SFX_FLANGER_RATE               = AL_FLANGER_RATE,
    ATRA_SFX_FLANGER_DEPTH              = AL_FLANGER_DEPTH,
    ATRA_SFX_FLANGER_FEEDBACK           = AL_FLANGER_FEEDBACK,
    ATRA_SFX_FLANGER_DELAY              = AL_FLANGER_DELAY
};

enum atra_audio_freqshifter_params {
    ATRA_SFX_FREQSHIFTER_FREQ           = AL_FREQUENCY_SHIFTER_FREQUENCY,
    ATRA_SFX_FREQSHIFTER_LEFT_DIR       = AL_FREQUENCY_SHIFTER_LEFT_DIRECTION,
    ATRA_SFX_FREQSHIFTER_RIGHT_DIR      = AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION
};

enum atra_audio_vocalmorpher_params {
    ATRA_SFX_VOCALMORPHER_PHONEME_A     = AL_VOCAL_MORPHER_PHONEMEA,
    ATRA_SFX_VOCALMORPHER_PHONEME_B     = AL_VOCAL_MORPHER_PHONEMEB,
    ATRA_SFX_VOCALMORPHER_PHONEME_A_COARSETUNING = AL_VOCAL_MORPHER_PHONEMEA_COARSE_TUNING,
    ATRA_SFX_VOCALMORPHER_PHONEME_B_COARSETUNING = AL_VOCAL_MORPHER_PHONEMEB_COARSE_TUNING,
    ATRA_SFX_VOCALMORPHER_WAVEFORM      = AL_VOCAL_MORPHER_WAVEFORM,
    ATRA_SFX_VOCALMORPHER_MORPHER_RATE  = AL_VOCAL_MORPHER_RATE
};

enum atra_audio_pitchshifter_params {
    ATRA_SFX_PITCHSHIFTER_COARSETUNE    = AL_PITCH_SHIFTER_COARSE_TUNE,
    ATRA_SFX_PITCHSHIFTER_FINETUNE      = AL_PITCH_SHIFTER_FINE_TUNE
};

enum atra_audio_ringmodulator_params {
    ATRA_SFX_RINGMOD_FREQUENCY          = AL_RING_MODULATOR_FREQUENCY,
    ATRA_SFX_RINGMOD_HIGHPASS_CUTOFF    = AL_RING_MODULATOR_HIGHPASS_CUTOFF,
    ATRA_SFX_RINGMOD_WAVEFORM           = AL_RING_MODULATOR_WAVEFORM

};

enum atra_audio_autowah_params {
    ATRA_SFX_AUTOWAH_ATTACK_TIME        = AL_AUTOWAH_ATTACK_TIME,
    ATRA_SFX_AUTOWAH_RELEASE_TIME       = AL_AUTOWAH_RELEASE_TIME,
    ATRA_SFX_AUTOWAH_RESONANCE          = AL_AUTOWAH_RESONANCE,
    ATRA_SFX_AUTOWAH_PEAK_GAIN          = AL_AUTOWAH_PEAK_GAIN
};

enum atra_audio_compressor_params {
    ATRA_SFX_COMPRESSOR_ONOFF           = AL_COMPRESSOR_ONOFF
};

enum atra_audio_equaliser_params {
    ATRA_SFX_EQUALIZER_LOW_GAIN         = AL_EQUALIZER_LOW_GAIN,
    ATRA_SFX_EQUALIZER_LOW_CUTOFF       = AL_EQUALIZER_LOW_CUTOFF,
    ATRA_SFX_EQUALIZER_MID1_GAIN        = AL_EQUALIZER_MID1_GAIN,
    ATRA_SFX_EQUALIZER_MID1_CENTER      = AL_EQUALIZER_MID1_CENTER,
    ATRA_SFX_EQUALIZER_MID1_WIDTH       = AL_EQUALIZER_MID1_WIDTH,
    ATRA_SFX_EQUALIZER_MID2_GAIN        = AL_EQUALIZER_MID2_GAIN,
    ATRA_SFX_EQUALIZER_MID2_CENTER      = AL_EQUALIZER_MID2_CENTER,
    ATRA_SFX_EQUALIZER_MID2_WIDTH       = AL_EQUALIZER_MID2_WIDTH,
    ATRA_SFX_EQUALIZER_HIGH_GAIN        = AL_EQUALIZER_HIGH_GAIN,
    ATRA_SFX_EQUALIZER_HIGH_CUTOFF      = AL_EQUALIZER_HIGH_CUTOFF
};

typedef ALCdevice AudioDevice;
typedef ALCcontext AudioContext;

/* An analog to the OpenAL buffer object.
 * Stores raw PCM data from which bound `source` objects get their data.
 */
typedef struct AudioBuffer {
    uint id;                /* OpenAL buffer object ID */
    char *file_path;        /* Path to the buffer data file */

    uint frequency;         /* Frequency of the buffer in hertz */
    uint bit_depth;         /* Bit depth of buffer */
    uint channels;          /* Number of channels (must be 1 for sound positioning) */
    size_t size;            /* Size of buffer in bytes */
} AudioBuffer;

/* The world-space point from which sounds originated.
 * Sources have buffers bound to them, where their audio data is obtained
 */
typedef struct AudioSource {
    uint id;                /* OpenAL audio source ID */

    float pitch;            /* Pitch multiplier */
    float gain;             /* Source gain */
    float max_radius;       /* Maximum distance */
    float rolloff;          /* Rolloff rate of source (default 1.0) */
    float ref_dist;         /* Distance the volume would normally drop by half */
    float min_gain;         /* Minimum gain of source */
    float max_gain;         /* Maximum gain of source */

    float cone_outer_gain;  /* Gain when outside the oriented cone */
    float cone_inner_angle; /* Gain when inside the oriented cone */
    float cone_outer_angle; /* Outer angle of the sound cone (default 360) */

    vec3 position;          /* Source world-space position */
    vec3 velocity;          /* Speed of the sound source */
    vec3 direction;         /* Direction the source is facing */

    int source_relative;    /* Set when positions are relative to listener (default FALSE) */
    int source_type;        /* Source type (UNDETERMINED/STATIC/STREAMING) */
    int looping;            /* Set when source is looping */
} AudioSource;

/* Container for a sound effect/filter which modifies the final mix of a
 * source's output
 */
typedef struct AudioEffect {
    uint id;
    uint type;
} AudioEffect;

/*  */

/* An audio auxiliary effect slot which binds sources and actuates an associated
 * audio effect on the source outputs
 */
typedef struct AudioAuxSlot {
    uint id;
} AudioAuxSlot;


/* Set up initial audio subsystem state, and load the default devices */
ATRA_API void atra_audio_init(void);

/* Shut down the audio subsystem */
ATRA_API void atra_audio_stop(void);

/* Open the given audio device name and returns a pointer to the device buffer */
ATRA_API AudioDevice * atra_audio_device_open(const char *device_id);

/* Close the given device buffer pointer */
ATRA_API int atra_audio_device_close(AudioDevice *device);


/* Audio buffer functions */

/* Create and initialise a new audio buffer.  Buffers are managed by internal database */
ATRA_API AudioBuffer * atra_audio_buffer_create(void);

/* Delete an audio buffer and it's associated audio data */
ATRA_API void atra_audio_buffer_delete(AudioBuffer *buf);

/* Load the given file into the buffer */
ATRA_API void atra_audio_buffer_loadfile(AudioBuffer *buf, const char *path);

/* Load raw PCM data into the given buffer */
ATRA_API void atra_audio_buffer_loadrawdata(AudioBuffer *buf, uint format,
        const void *data, size_t size, uint freq);

/* Retrieve an audio buffer by its ID */
ATRA_API AudioBuffer * atra_audio_buffer_get(uint id);

/* Retrieve an audio buffer's ID by it's data file path.
 * Implicitly loads new buffer at given path if no matching buffer exists */
ATRA_API uint atra_audio_buffer_getid_bypath(const char *path);

ATRA_API void atra_al_buffer_i(AudioBuffer *buf, int param, const int value);
ATRA_API void atra_al_buffer_iv(AudioBuffer *buf, int param, const int *values);
ATRA_API void atra_al_buffer_f(AudioBuffer *buf, int param, const float value);
ATRA_API void atra_al_buffer_fv(AudioBuffer *buf, int param, const float *values);

ATRA_API void atra_al_getbuffer_i(AudioBuffer *buf, int param, int *value);
ATRA_API void atra_al_getbuffer_iv(AudioBuffer *buf, int param, int *values);
ATRA_API void atra_al_getbuffer_f(AudioBuffer *buf, int param, float *value);
ATRA_API void atra_al_getbuffer_fv(AudioBuffer *buf, int param, float *values);


/* Audio source functions */

/* Create a new, default audio source */
ATRA_API AudioSource atra_audio_source_create(void);

/* Delete an audio source */
ATRA_API void atra_audio_source_delete(AudioSource *src);

/* Set the world-space position of an audio source */
ATRA_API void atra_audio_source_setpos(AudioSource *src, const vec3 pos);

/* Set the velocity vector of an audio source */
ATRA_API void atra_audio_source_setvel(AudioSource *src, const vec3 vel);

/* Set the direction vector of an audio source */
ATRA_API void atra_audio_source_setdir(AudioSource *src, const vec3 dir);

/* Set the pitch multiplier of an audio source */
ATRA_API void atra_audio_source_setpitch(AudioSource *src, float pitch);

/* Set the gain of an audio source */
ATRA_API void atra_audio_source_setgain(AudioSource *src, float gain);

/* Set the max radius of an audio source */
ATRA_API void atra_audio_source_setradius(AudioSource *src, float radius);

/* Set the rolloff of an audio source */
ATRA_API void atra_audio_source_setrolloff(AudioSource *src, float rolloff);

/* Set the reference distance of an audio source */
ATRA_API void atra_audio_source_setrefdist(AudioSource *src, float refdist);

/* Set the minimum gain of an audio source */
ATRA_API void atra_audio_source_setmingain(AudioSource *src, float gain);

/* Set the maximum gain of an audio source */
ATRA_API void atra_audio_source_setmaxgain(AudioSource *src, float gain);

/* Set the outer gain of a source's sound cone */
ATRA_API void atra_audio_source_setoutergain(AudioSource *src, float gain);

/* Set the inner angle of a source's sound cone */
ATRA_API void atra_audio_source_setinnerangle(AudioSource *src, float angle);

/* Set the outer angle of a source's sound cone */
ATRA_API void atra_audio_source_setouterangle(AudioSource *src, float angle);

/* Set whether or not the source's position is relative to the listener */
ATRA_API void atra_audio_source_setlistenerrelativity(AudioSource *src, int relative);

/* Set the type of an audio source (STATIC/STREAMING) */
ATRA_API void atra_audio_source_settype(AudioSource *src, uint type);

/* Set an audio source to looping mode */
ATRA_API void atra_audio_source_enablelooping(AudioSource *src);

/* Set an audio source to looping mode */
ATRA_API void atra_audio_source_disablelooping(AudioSource *src);

/* Play sound data through an audio source */
ATRA_API void atra_audio_source_play(AudioSource *src);

/* Pause playback of an audio source */
ATRA_API void atra_audio_source_pause(AudioSource *src);

/* Stop playback of an audio source */
ATRA_API void atra_audio_source_stop(AudioSource *src);

/* Stop playback of an audio source and reset it to the initial state */
ATRA_API void atra_audio_source_rewind(AudioSource *src);

/* Attaches a buffer to the audio source */
ATRA_API void atra_audio_source_attachbuffer(AudioSource *src, AudioBuffer *buf);

/* Queue a number of audio buffers for the source to play in sequence */
ATRA_API void atra_audio_source_queuebuffers(AudioSource *src, uint n, AudioBuffer *bufs);

/* Unqueue a set of audio buffers from a source.
 * Only completes when all buffers can be removed from queue */
ATRA_API void atra_audio_source_unqueuebuffers(AudioSource *src, uint n, AudioBuffer *bufs);

ATRA_API void atra_al_source_i(AudioSource *src, int param, const int value);
ATRA_API void atra_al_source_iv(AudioSource *src, int param, const int *values);
ATRA_API void atra_al_source_f(AudioSource *src, int param, const float value);
ATRA_API void atra_al_source_fv(AudioSource *src, int param, const float *values);

ATRA_API void atra_al_getsource_i(AudioSource *src, int param, int *value);
ATRA_API void atra_al_getsource_iv(AudioSource *src, int param, int *values);
ATRA_API void atra_al_getsource_f(AudioSource *src, int param, float *value);
ATRA_API void atra_al_getsource_fv(AudioSource *src, int param, float *values);


/* Audio listener functions */

/* Set the master gain of the audio context's listener */
ATRA_API void atra_audio_listener_setgain(float gain);

/* Set the world-space position of the audio context's listener */
ATRA_API void atra_audio_listener_setpos(const vec3 pos);

/* Set the velocity vector of the audio context's listener */
ATRA_API void atra_audio_listener_setvel(const vec3 vel);

/* Set the orientation of the audio context's listener (at, up vectors) */
ATRA_API void atra_audio_listener_setorientation(const vec3 at, const vec3 up);

ATRA_API void atra_al_listener_i(int param, const int value);
ATRA_API void atra_al_listener_iv(int param, const int *values);
ATRA_API void atra_al_listener_f(int param, const float value);
ATRA_API void atra_al_listener_fv(int param, const float *values);

ATRA_API void atra_al_getlistener_i(int param, int *value);
ATRA_API void atra_al_getlistener_iv(int param, int *values);
ATRA_API void atra_al_getlistener_f(int param, float *value);
ATRA_API void atra_al_getlistener_fv(int param, float *values);


ATRA_API extern LPALGENEFFECTS atra_al_geneffects;
ATRA_API extern LPALDELETEEFFECTS atra_al_deleteeffects;
ATRA_API extern LPALISEFFECT atra_al_iseffect;
ATRA_API extern LPALEFFECTI atra_al_effect_i;
ATRA_API extern LPALEFFECTIV atra_al_effect_iv;
ATRA_API extern LPALEFFECTF atra_al_effect_f;
ATRA_API extern LPALEFFECTFV atra_al_effect_fv;
ATRA_API extern LPALGETEFFECTI atra_al_geteffect_i;
ATRA_API extern LPALGETEFFECTIV atra_al_geteffect_iv;
ATRA_API extern LPALGETEFFECTF atra_al_geteffect_f;
ATRA_API extern LPALGETEFFECTFV atra_al_geteffect_fv;

ATRA_API extern LPALGENFILTERS atra_al_genfilters;
ATRA_API extern LPALDELETEFILTERS atra_al_deletefilters;
ATRA_API extern LPALISFILTER atra_al_isfilter;
ATRA_API extern LPALFILTERI atra_al_filter_i;
ATRA_API extern LPALFILTERIV atra_al_filter_iv;
ATRA_API extern LPALFILTERF atra_al_filter_f;
ATRA_API extern LPALFILTERFV atra_al_filter_fv;
ATRA_API extern LPALGETFILTERI atra_al_getfilter_i;
ATRA_API extern LPALGETFILTERIV atra_al_getfilter_iv;
ATRA_API extern LPALGETFILTERF atra_al_getfilter_f;
ATRA_API extern LPALGETFILTERFV atra_al_getfilter_fv;

ATRA_API extern LPALGENAUXILIARYEFFECTSLOTS atra_al_genauxiliaryeffectslots;
ATRA_API extern LPALDELETEAUXILIARYEFFECTSLOTS atra_al_deleteauxiliaryeffectslots;
ATRA_API extern LPALISAUXILIARYEFFECTSLOT atra_al_isauxiliaryeffectslot;
ATRA_API extern LPALAUXILIARYEFFECTSLOTI atra_al_auxiliaryeffectslot_i;
ATRA_API extern LPALAUXILIARYEFFECTSLOTIV atra_al_auxiliaryeffectslot_iv;
ATRA_API extern LPALAUXILIARYEFFECTSLOTF atra_al_auxiliaryeffectslot_f;
ATRA_API extern LPALAUXILIARYEFFECTSLOTFV atra_al_auxiliaryeffectslot_fv;
ATRA_API extern LPALGETAUXILIARYEFFECTSLOTI atra_al_getauxiliaryeffectslot_i;
ATRA_API extern LPALGETAUXILIARYEFFECTSLOTIV atra_al_getauxiliaryeffectslot_iv;
ATRA_API extern LPALGETAUXILIARYEFFECTSLOTF atra_al_getauxiliaryeffectslot_f;
ATRA_API extern LPALGETAUXILIARYEFFECTSLOTFV atra_al_getauxiliaryeffectslot_fv;


#endif /* ATRA_AUDIO_H */
