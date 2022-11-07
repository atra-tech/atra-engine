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

enum physbody_shape_type {
    ATRA_PHYS_SHAPE_AABB,
    ATRA_PHYS_SHAPE_SPHERE,
    ATRA_PHYS_SHAPE_CYLINDER,
};

enum collision_mode {
    ATRA_PHYS_COLLIDE_DETECT,
    ATRA_PHYS_COLLIDE_HARD,
    ATRA_PHYS_COLLIDE_SOFT
};

enum physbody_behavior {
    ATRA_PHYS_AXISALIGNED,      /* Shapes are axis aligned */
    ATRA_PHYS_DYNAMIC
};

enum physbody_collision_combo_type {
    ATRA_PHYS_COLLIDE_AABB_AABB,
};

/* Axis-aligned bounding quad structure */
typedef struct AABQ {
    vec2 min;
    vec2 max;
} AABQ;

/* 2D circle structure */
typedef struct Circle {
    vec2 pos;
    float r;
} Circle;

/* Axis-aligned bounding box structure */
typedef struct AABB {
    vec3 min;       /* Position of minimum corner of bounding box */
    vec3 max;       /* Position of maximum corner of bounding box */
} AABB;

typedef struct Sphere {
    vec3 pos;
    float r;
} Sphere;

typedef struct Cylinder {
    vec3 pos;
    float h;
    float r;
} Cylinder;

typedef union PhysShape {
    AABB aabb;
    Sphere sphere;
    Cylinder cylinder;
} PhysShape;

/* Physics body for holding dynamic physics data complimentary to a shape primitive */
typedef struct PhysBody {
    PhysShape shape;    /* Shape data union */
    uint shape_type;    /* Type of data the shape union contains */

    vec3 force_dir;     /* The direction that force is applied in */
    float force;        /* The amount of force applied */
} PhysBody;

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
 *  2D Axis-aligned bounding quad functions
 */

/* Returns an AABQ with the given minimum and maximum coordinate positions */
ATRA_API AABQ atra_aabq_create(vec2 min, vec2 max);

/* Returns true if two AABQs intersect */
ATRA_API int atra_aabq_aabq_collision(AABQ a, AABQ b);

/* Returns true if the two given AABBs intersect.
 * Writes collision resolution vectors to `rvec_a` and `rvec_b`. */
ATRA_API int atra_aabq_aabq_resolve(AABQ a, AABQ b,
        float *rvec_a, float *rvec_b, int static_a, int static_b);

/* Returns true if the two given AABBs intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API int atra_aabq_aabq_reflect(AABQ a, AABQ b,
        float *rvec_a, float *rvec_b, float *dir_a, float *dir_b,
        int static_a, int static_b);

/*
 *  2D Circle functions
 */

/* Returns an AABQ with the given minimum and maximum coordinate positions */
ATRA_API Circle atra_circle_create(vec2 pos, float radius);

/* Returns true if two AABQs intersect */
ATRA_API int atra_circle_circle_collision(Circle a, Circle b);

/* Returns true if the two given AABBs intersect.
 * Writes collision resolution vectors to `rvec_a` and `rvec_b`. */
ATRA_API int atra_circle_circle_resolve(Circle a, Circle b,
        float *rvec_a, float *rvec_b, int static_a, int static_b);

/* Returns true if the two given AABBs intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API int atra_circle_circle_reflect(Circle a, Circle b,
        float *rvec_a, float *rvec_b, float *dir_a, float *dir_b,
        int static_a, int static_b);


/* 
 *  Axis-aligned bounding box functions
 */

/* Returns an AABB with the given minimum and maximum coordinate positions */
ATRA_API AABB atra_aabb_create(vec3 min, vec3 max);

/* Returns true if two AABBs intersect */
ATRA_API int atra_aabb_aabb_collision(AABB a, AABB b);

/* Returns true if the two given AABBs intersect.
 * Writes collision resolution vectors to `rvec_a` and `rvec_b`. */
ATRA_API int atra_aabb_aabb_resolve(AABB a, AABB b,
        float *rvec_a, float *rvec_b, int static_a, int static_b);

/* Returns true if the two given AABBs intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API int atra_aabb_aabb_reflect(AABB a, AABB b,
        float *rvec_a, float *rvec_b, float *dir_a, float *dir_b,
        int static_a, int static_b);


/*
 *  Sphere functions
 * */

/* Returns a new sphere with the given attributes */
ATRA_API Sphere atra_sphere_create(vec3 centre, float radius);

/* Returns true if two spheres intersect */
ATRA_API int atra_sphere_sphere_collision(Sphere a, Sphere b);


/*
 *  Cylinder functions
 * */

/* Returns a new cylinder with the given attributes */
ATRA_API Cylinder atra_cylinder_create(vec3 centre, float height, float radius);

/* Returns true if two cylinders intersect */
ATRA_API int atra_cylinder_cylinder_collision(Cylinder a, Cylinder b);


/*
 *  Inter-shape Collision Functions
 * */

/* Returns true if a sphere and AABB intersect */
ATRA_API int atra_aabq_circle_collision(AABQ q, Circle c);

/* Returns true if an AABQ and circle intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s` */
ATRA_API int atra_aabq_circle_resolve(AABQ q, Circle c,
        float *rvec_q, float *rvec_c, int static_q, int static_c);

/* Returns true if an AABQ and circle intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API int atra_aabq_circle_reflect(AABQ q, Circle c,
        float *rvec_q, float *rvec_c, float *dir_q, float *dir_c,
        int static_q, int static_c);


/* Returns true if a sphere and AABB intersect */
ATRA_API int atra_aabb_sphere_collision(AABB b, Sphere s);

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s` */
ATRA_API int atra_aabb_sphere_resolve(AABB b, Sphere s,
        float *rvec_b, float *rvec_s, int static_b, int static_s);

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API int atra_aabb_sphere_reflect(AABB b, Sphere s,
        float *rvec_b, float *rvec_s, float *dir_b, float *dir_s,
        int static_b, int static_s);


/* Returns true if a cylinder and AABB intersect */
ATRA_API int atra_aabb_cylinder_collision(AABB b, Cylinder c);

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s` */
ATRA_API int atra_aabb_cylinder_resolve(AABB b, Cylinder c,
        float *rvec_b, float *rvec_c, int static_b, int static_c);

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API int atra_aabb_cylinder_reflect(AABB b, Cylinder c,
        float *rvec_b, float *rvec_c, float *dir_b, float *dir_c,
        int static_b, int static_c);


/* Returns true if a sphere and cylinder intersect */
ATRA_API int atra_sphere_cylinder_collision(Sphere s, Cylinder c);

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s` */
ATRA_API int atra_sphere_cylinder_resolve(Sphere s, Cylinder c,
        float *rvec_s, float *rvec_c, int static_s, int static_c);

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API int atra_sphere_cylinder_reflect(Sphere s, Cylinder c,
        float *rvec_s, float *rvec_c, float *dir_s, float *dir_c,
        int static_s, int static_c);


/* Returns true if a ray intersects a sphere */
ATRA_API int atra_ray_sphere_collision(Ray ray, Sphere sphere, vec3 *collide_pos);

/* Returns true if a ray intersects an AABB */
//ATRA_API int atra_ray_aabb_collision(Ray ray, AABB box, vec3 *collide_pos);


/*
 *  Physics-Body Functions
 * */

/* Creates a new AABB as a physics shape */
ATRA_API PhysShape atra_physshape_create_aabb(vec3 min, vec3 max);

/* Creates a new sphere as a physics shape */
ATRA_API PhysShape atra_physshape_create_sphere(vec3 centre, float radius);

/* Creates a new cylinder as a physics shape */
ATRA_API PhysShape atra_physshape_create_cylinder(vec3 centre, float height, float radius);

/* Creates a new PhysShape with the given type */
ATRA_API PhysBody atra_physbody_create(uint shape_type, PhysShape shape);

/* Returns true if two physics bodies are intersecting */
//ATRA_API int atra_physbody_collision(PhysBody a, PhysBody b);


#endif /* ATRA_PHYSICS_H */
