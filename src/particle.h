/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved
 *
 *  particle.h - A module providing a set of tools for managing and rendering
 *               particles.
 *
 * */

#ifndef ATRA_PARTICLE_H
#define ATRA_PARTICLE_H

#include "common.h"

#include "model.h"

#ifdef __cplusplus
extern "C" {
#endif

enum particle_emitter_type {
    ATRA_PARTICLE_EMITTER_CONSTANT,
    ATRA_PARTICLE_EMITTER_BURST
};

/* A store of common particle information used in the initialisation and
 * updating of particles.
 */
typedef struct ParticleData {
    Material material;

    vec4 start_color;
    vec4 mid_color;
    vec4 end_color;

    vec4 start_dir;
    vec4 mid_dir;
    vec4 end_dir;

    float start_speed;
    float mid_speed;
    float end_speed;

    float start_size;
    float mid_size;
    float end_size;

    float lifetime;
    float gravity;
    vec3 rotation_speed;
    
    int randomise_rotation;
    int face_camera;

    void (*update)(float dt);
} ParticleData;

typedef struct Particle {
    int is_active;
    vec4 color;
    vec3 position;
    vec3 direction;
    float speed;
    float size;
} Particle;

/* A single point of origin spawner for particle effects */
typedef struct ParticleEmitter {
    ParticleData *prt_data; /* Common particle data */

    int type;               /* Style in which emitter produces particles */
    int active;             /* Emitter is active */

    vec3 position;          /* World-space position of the emitter */
    vec3 direction;         /* Direction vector aiming the emission cone */
    float radius;           /* Spherical area the particles can spawn in */
    float cone_angle;       /* Radius of the emission cone (0 - 360) */
    float duration;         /* Time to emit for */
    float start_time;       /* Time after activation to begin emitting */

    float min_lifetime;     /* Minimum lifetime of particles */
    float max_lifetime;     /* Maximum lifetime of particles */

    float min_size;         /* Minimum size of particles */
    float max_size;         /* Maximum size of particles */

    uint max_particles;     /* Maximum number of particles allowed at once */
    uint particle_count;    /* Current number of particles */
    uint loop;              /* Emitter should repeat */

    Particle *particles;    /* Array of particles currently in this emitter */
    mat4 *model_mats;       /* Array of particle model matrices */
} ParticleEmitter;

/* A 3D area in which particles can randomly spawn */
typedef struct ParticleField {
    ParticleData *data;

    vec3 cube_min;
    vec3 cube_max;

    float duration;
    float start_time;

    float min_lifetime;
    float max_lifetime;

    float min_size;
    float max_size;

    uint max_particles;
    uint particle_count;
    uint loop;

    Particle *particles;
    mat4 *model_mats;
} ParticleField;

void atra_particle_init(void);


/* Emitter functions */

ParticleEmitter atra_particle_emitter_create(uint type, uint max_particles);

void atra_particle_emitter_start(ParticleEmitter emitter);

void atra_particle_emitter_update(ParticleEmitter emitter, float dt);

void atra_particle_emitter_render(ParticleEmitter emitter);


/* Field functions */

ParticleField atra_particle_field_create(uint type, uint max_particles);

void atra_particle_field_start(ParticleField field);

void atra_particle_field_update(ParticleField field, float dt);

void atra_particle_field_render(ParticleField field);

#ifdef __cplusplus
}
#endif

#endif /* ATRA_PARTICLE_H */
