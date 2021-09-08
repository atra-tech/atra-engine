/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  physics.h - Physics utilities for collision detection of basic shape types,
 *              including rays, spheres, and axis aligned bounding boxes.
 *
 *  */

#ifndef ATRA_PHYSICS_H
#define ATRA_PHYSICS_H

#include "common.h"

/* Axis-aligned bounding box structure */
typedef struct AABB {
    vec3 min_pos;       /* Position of minimum corner of bounding box */
    vec3 max_pos;       /* Position of maximum corner of bounding box */
} AABB;

typedef struct Sphere {
    vec3 centre;
    float radius;
} Sphere;

/* Ray structure for use in raycasting */
typedef struct Ray {
    vec3 pos;           /* Position of ray origin */
    vec3 dir;           /* Direction vector of ray */
} Ray;

/* Ray collision information, useful for progressively stepped collisions */
typedef struct RayHit {
    int hit;            /* True when ray collision is detected */
    float dist;         /* Distance of collision from origin */
    vec3 hit_pos;       /* Position of collision */
    vec3 hit_norm;      /* Surface normal of collision */
} RayHit;

/* 
 *  Axis-aligned bounding box functions
 */

/* Returns an AABB with the given minimum and maximum coordinate positions */
ATRA_API AABB atra_aabb_create(vec3 min, vec3 max);

/* Returns true if two AABBs intersect */
ATRA_API int atra_aabb_aabb_collision(AABB a, AABB b);

/* Returns true if two spheres intersect */
ATRA_API int atra_sphere_sphere_collision(Sphere a, Sphere b);

/* Returns true if a sphere and AABB intersect */
ATRA_API int atra_aabb_sphere_collision(AABB box, Sphere sphere);

/* Returns true if a ray intersects a sphere */
ATRA_API int atra_ray_sphere_collision(Ray ray, Sphere sphere, vec3 *collision);

/* Returns true if a ray intersects an AABB */
ATRA_API int atra_ray_aabb_collision(Ray ray, AABB box);


#endif /* ATRA_PHYSICS_H */
