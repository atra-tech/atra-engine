/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  physics.c - Simple, low-cost physics utilities for collision detection of basic shape types.
 *
 *  */

#include "atra.h"

static void atra_vec3_reflect(float *v, float *n);
static void atra_vec2_reflect(float *v, float *n);


/*
 *  2D Axis-aligned bounding quad functions
 */

/* Returns an AABQ with the given minimum and maximum coordinate positions */
ATRA_API
AABQ
atra_aabq_create(vec2 min, vec2 max)
{
    return (AABQ) {
        .min = {min[0], min[1]},
        .max = {max[0], max[1]}
    };
}

/* Returns true if two AABQs intersect */
ATRA_API
int
atra_aabq_aabq_collision(AABQ a, AABQ b)
{
    return ((a.min[0] < b.max[0]) && (a.max[0] > b.min[0]) &&
            (a.min[1] < b.max[1]) && (a.max[1] > b.min[1]));
}

/* Returns true if the two given AABBs intersect.
 * Writes collision resolution vectors to `rvec_a` and `rvec_b`. */
ATRA_API
int
atra_aabq_aabq_resolve(AABQ a, AABQ b,
        float *rvec_a, float *rvec_b, int static_a, int static_b)
{
    if (atra_aabq_aabq_collision(a, b) == 0) {
        return 0;
    }

    const vec2 a_diff = {
        a.max[0] - b.min[0],
        a.max[1] - b.min[1],
    };

    const vec2 b_diff = {
        b.max[0] - a.min[0],
        b.max[1] - a.min[1],
    };

    vec2 diff;
    float offset;

    diff[0] = MIN(a_diff[0], b_diff[0]);
    diff[1] = MIN(a_diff[1], b_diff[1]);

    memset(rvec_a, 0, sizeof(vec2));
    memset(rvec_b, 0, sizeof(vec2));

    if (diff[0] > diff[1]) {
        offset = diff[0] * DECIDE(a_diff[0] < b_diff[0], -1, 1);
        offset = DECIDE(static_a || static_b, offset, offset / 2.0f);
        rvec_a[0] = DECIDE(static_b, offset, 0.0f);
        rvec_b[0] = DECIDE(static_a, -offset, 0.0f);
    } else {
        offset = diff[1] * DECIDE(a_diff[1] < b_diff[1], -1, 1);
        offset = DECIDE(static_a || static_b, offset, offset / 2.0f);
        rvec_a[1] = DECIDE(static_b, offset, 0.0f);
        rvec_b[1] = DECIDE(static_a, -offset, 0.0f);
    }
    
    return 1;
}

/* Returns true if the two given AABBs intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API
int
atra_aabq_aabq_reflect(AABQ a, AABQ b,
        float *rvec_a, float *rvec_b, float *dir_a, float *dir_b,
        int static_a, int static_b)
{
    if (atra_aabq_aabq_resolve(a, b, rvec_a, rvec_b, static_a, static_b) == 0) {
        return 0;
    }
    atra_vec2_reflect(dir_a, rvec_a);
    atra_vec2_reflect(dir_b, rvec_b);

    return 1;
}


/*
 *  2D Circle functions
 */

/* Returns an AABQ with the given minimum and maximum coordinate positions */
ATRA_API
Circle
atra_circle_create(vec2 pos, float r)
{
    return (Circle) {
        .pos = {pos[0], pos[1]},
        .r = r
    };
}

/* Returns true if two AABQs intersect */
ATRA_API
int
atra_circle_circle_collision(Circle a, Circle b)
{
    const vec2 diff = {
        a.pos[0] - b.pos[0],
        a.pos[1] - b.pos[1],
    };

    const float sq_dist = diff[0] * diff[0] + diff[1] * diff[1];
    const float r = a.r + b.r;

    return (sq_dist < r * r);
}

/* Returns true if the two given AABBs intersect.
 * Writes collision resolution vectors to `rvec_a` and `rvec_b`. */
ATRA_API
int
atra_circle_circle_resolve(Circle a, Circle b,
        float *rvec_a, float *rvec_b, int static_a, int static_b)
{
    vec2 diff = {
        a.pos[0] - b.pos[0],
        a.pos[1] - b.pos[1],
    };
 
    const float dist = sqrt(diff[0] * diff[0] + diff[1] * diff[1]);
    const float r = a.r + b.r;

    if (dist < r) {
        return 0;
    }
    const float c_overlap = r - dist;

    float offset;
    vec2 norm;

    memset(rvec_a, 0, sizeof(vec3));
    memset(rvec_b, 0, sizeof(vec3));

    offset = DECIDE(static_a || static_b, c_overlap, c_overlap / 2);
    glm_vec2_normalize_to(diff, norm);
    norm[0] *= offset;
    norm[1] *= offset;

    rvec_a[0] = DECIDE(static_a, norm[0], 0.0f);
    rvec_a[1] = DECIDE(static_a, norm[1], 0.0f);
    rvec_b[0] = DECIDE(static_b, -norm[0], 0.0f);
    rvec_b[1] = DECIDE(static_b, -norm[1], 0.0f);

    return 1;
}

/* Returns true if the two given AABBs intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API
int
atra_circle_circle_reflect(Circle a, Circle b,
        float *rvec_a, float *rvec_b, float *dir_a, float *dir_b,
        int static_a, int static_b)
{
    if (atra_circle_circle_resolve(a, b, rvec_a, rvec_b, static_a, static_b) == 0) {
        return 0;
    }
    atra_vec2_reflect(dir_a, rvec_a);
    atra_vec2_reflect(dir_b, rvec_b);

    return 1;
}


/* 
 *  Axis-aligned bounding box functions
 */

/* Returns an AABB with the given minimum and maximum coordinate positions */
ATRA_API
AABB
atra_aabb_create(vec3 min, vec3 max)
{
    AABB box;

    box.min[0] = min[0];
    box.min[1] = min[1];
    box.min[2] = min[2];
    
    box.max[0] = max[0];
    box.max[1] = max[1];
    box.max[2] = max[2];

    return box;
}

/* Returns true if two AABBs intersect */
ATRA_API
int
atra_aabb_aabb_collision(AABB a, AABB b)
{
    return (((a.max[0] > b.min[0]) && (a.min[0] < b.max[0])) &&
            ((a.max[1] > b.min[1]) && (a.min[1] < b.max[1])) &&
            ((a.max[2] > b.min[2]) && (a.min[2] < b.max[2])));
}

/* Checks for collision between two AABBs and writes a resolution vector for
 * each AABB to `rvec_a` and `rvec_b`.
 * Returns true if any collision occured, otherwise false. */
ATRA_API 
int
atra_aabb_aabb_collision_resolve(AABB a, AABB b,
        float *rvec_a, float *rvec_b, int static_a, int static_b)
{
    if (atra_aabb_aabb_collision(a, b) == 0) {
        return 0;
    }

    const vec3 a_diff = {
        a.max[0] - b.min[0],
        a.max[1] - b.min[1],
        a.max[2] - b.min[2]
    };

    const vec3 b_diff = {
        b.max[0] - a.min[0],
        b.max[1] - a.min[1],
        b.max[2] - a.min[2]
    };

    ivec3 pick;
    vec3 diff;
    float offset;

    pick[0] = a_diff[0] < b_diff[0];
    pick[1] = a_diff[1] < b_diff[1];
    pick[2] = a_diff[2] < b_diff[2];

    diff[0] = DECIDE(pick[0], a_diff[0], b_diff[0]);
    diff[1] = DECIDE(pick[1], a_diff[1], b_diff[1]);
    diff[2] = DECIDE(pick[2], a_diff[2], b_diff[2]);

    memset(rvec_a, 0, sizeof(vec3));
    memset(rvec_b, 0, sizeof(vec3));

    if (diff[0] > diff[1] && diff[0] > diff[2]) {
        offset = diff[0] * DECIDE(pick[0], -1, 1);
        offset = DECIDE(static_a || static_b, offset, offset / 2.0f);
        rvec_a[0] = DECIDE(static_b, offset, 0.0f);
        rvec_b[0] = DECIDE(static_a, -offset, 0.0f);
    } else if (diff[1] > diff[0] && diff[1] > diff[2]) {
        offset = diff[1] * DECIDE(pick[1], -1, 1);
        offset = DECIDE(static_a || static_b, offset, offset / 2.0f);
        rvec_a[1] = DECIDE(static_b, offset, 0.0f);
        rvec_b[1] = DECIDE(static_a, -offset, 0.0f);
    } else {
        offset = diff[2] * DECIDE(pick[2], -1, 1);
        offset = DECIDE(static_a || static_b, offset, offset / 2.0f);
        rvec_a[2] = DECIDE(static_b, offset, 0.0f);
        rvec_b[2] = DECIDE(static_a, -offset, 0.0f);
    }

    return 1;
}

/* Checks for collision between two AABBs and writes a resolution vector for
 * each AABB to `rvec_a` and `rvec_b`, while reflecting given direction vectors
 * by the resulting collision normal.
 * Returns true if any collision occured, otherwise false. */
ATRA_API
int
atra_aabb_aabb_collision_reflect(AABB a, AABB b,
        float *rvec_a, float *rvec_b, float *dir_a, float *dir_b,
        int static_a, int static_b)
{
    if (atra_aabb_aabb_collision_resolve(a, b, rvec_a, rvec_b, static_a, static_b) == 0) {
        return 0;
    }
    atra_vec3_reflect(dir_a, rvec_a);
    atra_vec3_reflect(dir_b, rvec_b);

    return 1;
}


/*
 *  Sphere functions
 * */

/* Returns a new sphere with the given attributes */
ATRA_API
Sphere
atra_sphere_create(vec3 pos, float r)
{
    Sphere s;

    s.pos[0] = pos[0];
    s.pos[1] = pos[1];
    s.pos[2] = pos[2];

    s.r = r;

    return s;
}

/* Returns true if two spheres intersect */
ATRA_API
int
atra_sphere_sphere_collision(Sphere a, Sphere b)
{
    const vec3 diff = {
            a.pos[0] - b.pos[0],
            a.pos[1] - b.pos[1],
            a.pos[2] - b.pos[2]};

    const float sq_dist = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
    const float r = a.r + b.r;

    return (sq_dist < r * r);
}


/*
 *  Cylinder functions
 * */

/* Returns a new cylinder with the given attributes */
ATRA_API
Cylinder
atra_cylinder_create(vec3 pos, float h, float r)
{
    Cylinder c;

    c.pos[0] = pos[0];
    c.pos[1] = pos[1];
    c.pos[2] = pos[2];

    c.h = h;
    c.r = r;

    return c;
}

/* Returns true if two cylinders intersect */
ATRA_API
int
atra_cylinder_cylinder_collision(Cylinder a, Cylinder b)
{
    const vec2 diff = {
        a.pos[0] - b.pos[0],
        a.pos[2] - b.pos[2],
    };

    const float sq_dist = diff[0] * diff[0] + diff[1] * diff[1];
    const float r = a.r + b.r;
    const int y_collide = (a.pos[1] + a.h > b.pos[1]) && (a.pos[1] < b.pos[1] + b.h);
    
    return (sq_dist < r * r) && y_collide;
}


/*
 *  Inter-shape Collision Functions
 * */

/* Returns true if a sphere and AABB intersect */
ATRA_API
int
atra_aabq_circle_collision(AABQ q, Circle c)
{
    float sq_dist = 0.0f;
    float p;
    uint i;

    /* Get the closest point on the box to the sphere */
    for (i = 0; i < 2; i++) {
        p = c.pos[i];
        if (p < q.min[i]) {
            sq_dist += (q.min[i] - p) * (q.min[i] - p);
        } else if (p > q.max[i]) {
            sq_dist += (p - q.max[i]) * (p - q.max[i]);
        }
    }

    return (sq_dist < c.r * c.r);
}

/* Returns true if an AABQ and circle intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s` */
ATRA_API
int
atra_aabq_circle_resolve(AABQ q, Circle c,
        float *rvec_q, float *rvec_c, int static_q, int static_c)
{
    const vec2 q_diff = {
        q.max[0] - q.min[0],
        q.max[1] - q.min[1]
    };
    const vec2 diff = {
        c.pos[0] - (q.min[0] + q_diff[0] / 2),
        c.pos[1] - (q.min[1] + q_diff[1] / 2)
    };
    const vec2 abs_diff = {fabsf(diff[0]), fabsf(diff[1])};

    vec2 c_point, norm;
    ivec2 out_flags = {0};
    float sq_dist = 0.0f;
    float p, offset;
    uint i;

    memset(rvec_q, 0, sizeof(vec2));
    memset(rvec_c, 0, sizeof(vec2));

    /* Get the closest point on the box to the sphere */
    for (i = 0; i < 2; i++) {
        p = c.pos[i];

        if (p < q.min[i]) {
            c_point[i] = p - q.min[i];
            sq_dist += c_point[i] * c_point[i];
            out_flags[i] = 1;
        } else if (p > q.max[i]) {
            c_point[i] = p - q.max[i];
            sq_dist += c_point[i] * c_point[i];
            out_flags[i] = 1;
        }
    }

    if (sq_dist >= c.r * c.r) {
        return 0;
    }
    
    #define PLANE_PUSH_AABQ_CIRCLE(axis) \
            offset = c.pos[axis] - DECIDE(diff[axis] < 0.0f, q.min[axis] - c.r, q.max[axis] + c.r); \
            offset = DECIDE(static_q || static_c, offset, offset / 2.0f); \
            rvec_q[axis] = DECIDE(static_q, 0.0f, -offset); \
            rvec_c[axis] = DECIDE(static_c, 0.0f, offset)

    if (out_flags[0] + out_flags[1] == 0) {
        /* Quick, forced resolution for deep collision */
        if (abs_diff[0] > abs_diff[1]) {
            PLANE_PUSH_AABQ_CIRCLE(0);
        } else {
            PLANE_PUSH_AABQ_CIRCLE(1);
        }
    } else {
        /* Accurate vector resolution for shallow collision */
        c_point[0] = CLAMP(c.pos[0], q.min[0], q.max[0]);
        c_point[1] = CLAMP(c.pos[1], q.min[1], q.max[1]);

        if (out_flags[0] + out_flags[1] > 1) {
            /* Edge or corner collision */
            glm_vec2_sub(c.pos, c_point, rvec_c);
            glm_vec2_normalize_to(rvec_c, norm);
            norm[0] *= c.r;
            norm[1] *= c.r;
            glm_vec2_sub(rvec_c, norm, rvec_c);
        } else if (out_flags[0]) {
            PLANE_PUSH_AABQ_CIRCLE(0);
        } else {
            PLANE_PUSH_AABQ_CIRCLE(1);
        }
    }

    return 1;

}

/* Returns true if an AABQ and circle intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API
int
atra_aabq_circle_reflect(AABQ q, Circle c,
        float *rvec_q, float *rvec_c, float *dir_q, float *dir_c,
        int static_q, int static_c)
{
    if (atra_aabq_circle_resolve(q, c, rvec_q, rvec_c, static_q, static_c) == 0) {
        return 0;
    }
    atra_vec2_reflect(dir_q, rvec_q);
    atra_vec2_reflect(dir_c, rvec_c);

    return 1;
}


/* Returns true if a sphere and AABB intersect */
ATRA_API
int
atra_aabb_sphere_collision(AABB b, Sphere s)
{
    float sq_dist = 0.0f;
    float c;
    uint i;

    /* Get the closest point on the box to the sphere */
    for (i = 0; i < 3; i++) {
        c = s.pos[i];
        if (c < b.min[i]) {
            sq_dist += (b.min[i] - c) * (b.min[i] - c);
        } else if (c > b.max[i]) {
            sq_dist += (c - b.max[i]) * (c - b.max[i]);
        }
    }

    return (sq_dist < s.r * s.r);
}

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s` */
ATRA_API int atra_aabb_sphere_resolve(AABB b, Sphere s,
        float *rvec_b, float *rvec_s, int static_b, int static_s)
{
    const vec3 b_diff = {
        b.max[0] - b.min[0],
        b.max[1] - b.min[1],
        b.max[2] - b.min[2]
    };
    const vec3 diff = {
        s.pos[0] - (b.min[0] + b_diff[0] / 2),
        s.pos[1] - (b.min[1] + b_diff[1] / 2),
        s.pos[2] - (b.min[2] + b_diff[2] / 2)
    };
    const vec3 abs_diff = {fabsf(diff[0]), fabsf(diff[1]), fabsf(diff[2])};

    vec3 c_point, norm;
    ivec3 out_flags = {0};
    float sq_dist = 0.0f;
    float c, offset;
    uint i;

    memset(rvec_b, 0, sizeof(vec3));
    memset(rvec_s, 0, sizeof(vec3));

    /* Get the closest point on the box to the sphere */
    for (i = 0; i < 3; i++) {
        c = s.pos[i];

        if (c < b.min[i]) {
            c_point[i] = c - b.min[i];
            sq_dist += c_point[i] * c_point[i];
            out_flags[i] = 1;
        } else if (c > b.max[i]) {
            c_point[i] = c - b.max[i];
            sq_dist += c_point[i] * c_point[i];
            out_flags[i] = 1;
        }
    }

    if (sq_dist >= s.r * s.r) {
        return 0;
    }
    
    #define PLANE_PUSH_AABB_SPHERE(axis) \
            offset = s.pos[axis] - DECIDE(diff[axis] < 0.0f, \
                    b.min[axis] - s.r, \
                    b.max[axis] + s.r); \
            offset = DECIDE(static_b || static_s, offset, offset / 2.0f); \
            rvec_b[axis] = DECIDE(static_s, -offset, 0.0f); \
            rvec_s[axis] = DECIDE(static_b, offset, 0.0f)

    if (out_flags[0] + out_flags[1] + out_flags[2] == 0) {
        /* Quick, forced resolution for deep collision */
        if (abs_diff[0] > abs_diff[1] && abs_diff[0] > abs_diff[2]) {
            PLANE_PUSH_AABB_SPHERE(0);
        } else if (abs_diff[1] > abs_diff[0] && abs_diff[1] > abs_diff[2]) {
            PLANE_PUSH_AABB_SPHERE(1);
        } else {
            PLANE_PUSH_AABB_SPHERE(2);
        }
    } else {
        /* Accurate vector resolution for shallow collision */
        c_point[0] = CLAMP(s.pos[0], b.min[0], b.max[0]);
        c_point[1] = CLAMP(s.pos[1], b.min[1], b.max[1]);
        c_point[2] = CLAMP(s.pos[2], b.min[2], b.max[2]);

        if (out_flags[0] + out_flags[1] + out_flags[2] > 1) {
            /* Edge or corner collision */
            glm_vec3_sub(s.pos, c_point, rvec_s);
            glm_vec3_normalize_to(rvec_s, norm);
            norm[0] *= s.r;
            norm[1] *= s.r;
            norm[2] *= s.r;
            glm_vec3_sub(norm, rvec_s, rvec_s);
        } else if (out_flags[0]) {
            PLANE_PUSH_AABB_SPHERE(0);
        } else if (out_flags[1]) {
            PLANE_PUSH_AABB_SPHERE(1);
        } else {
            PLANE_PUSH_AABB_SPHERE(2);
        }
    }

    return 1;
}

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API int atra_aabb_sphere_reflect(AABB b, Sphere s,
        float *rvec_b, float *rvec_s, float *dir_b, float *dir_s,
        int static_b, int static_s)
{
    if (atra_aabb_sphere_resolve(b, s, rvec_b, rvec_s, static_b, static_s) == 0) {
        return 0;
    }
    atra_vec3_reflect(dir_b, rvec_b); 
    atra_vec3_reflect(dir_s, rvec_s);

    return 1;
}


/* Returns true if a cylinder and AABB intersect */
ATRA_API
int
atra_aabb_cylinder_collision(AABB b, Cylinder c)
{
    float sq_dist = 0.0f;
    float p;
    uint i;

    const int y_collide = (c.pos[1] + c.h > b.min[1]) && (c.pos[1] < b.max[1]);

    for (i = 0; i < 2; i++) {
        p = c.pos[i];
        if (p < b.min[i]) {
            sq_dist += (b.min[i] - p) * (b.min[i] - p);
        } else if (p > b.max[i]) {
            sq_dist += (p - b.max[i]) * (p - b.max[i]);
        }
    }

    return (sq_dist < c.r * c.r) && y_collide;
}

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s` */
/*ATRA_API int atra_aabb_cylinder_resolve(AABB b, Cylinder c,
        float *rvec_b, float *rvec_c, int static_b, int static_c)
{

}*/

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
/*ATRA_API int atra_aabb_cylinder_reflect(AABB b, Cylinder c,
        float *rvec_b, float *rvec_c, float *dir_b, float *dir_c,
        int static_b, int static_c)
{

}*/


/* Returns true if a sphere and cylinder intersect */
ATRA_API
int
atra_sphere_cylinder_collision(Sphere s, Cylinder c)
{
    const vec2 diff = {
        s.pos[0] - c.pos[0],
        s.pos[2] - c.pos[2],
    };

    const float sq_dist = diff[0] * diff[0] + diff[1] * diff[1];
    const float r = s.r + c.r;
    const int y_collide = (c.pos[1] + c.h > s.pos[1] - s.r) && (c.pos[1] < s.pos[1] + s.r);

    return (sq_dist < r * r) && y_collide;
}

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s` */
ATRA_API int atra_sphere_cylinder_resolve(Sphere s, Cylinder c,
        float *rvec_s, float *rvec_c, int static_s, int static_c)
{
    /* TODO: Add additional collision clause for cylinder rim */

    vec3 diff = {
        s.pos[0] - c.pos[0],
        s.pos[1] - c.pos[1],
        s.pos[2] - c.pos[2]
    };
 
    const float dist = sqrt(diff[0] * diff[0] + diff[1] * diff[1]);
    const float r = s.r + c.r;
    const int y_collide = (c.pos[1] + c.h > s.pos[1] - s.r) && (c.pos[1] < s.pos[1] + s.r);

    if ((dist < r) && y_collide == 0) {
        return 0;
    }
    const float y_cross_t = (s.pos[1] + s.r) - c.pos[1];
    const float y_cross_b = (c.pos[1] + c.h) - (s.pos[1] - s.r);
    const float y_overlap = MIN(y_cross_t, y_cross_b);
    const float c_overlap = r - dist;

    float offset;
    vec3 norm;

    memset(rvec_s, 0, sizeof(vec3));
    memset(rvec_c, 0, sizeof(vec3));

    if (c_overlap > y_overlap) {
        if (y_cross_t > y_cross_b) {
            /* Push down */
            offset = DECIDE(static_s || static_c, y_cross_b, y_cross_b / 2);
            rvec_s[1] = DECIDE(static_s, offset, 0.0f);
            rvec_c[1] = DECIDE(static_c, -offset, 0.0f);
        } else {
            /* Push up */
            offset = DECIDE(static_s || static_c, y_cross_t, y_cross_t / 2);
            rvec_s[1] = DECIDE(static_s, -offset, 0.0f);
            rvec_c[1] = DECIDE(static_c, offset, 0.0f);
        }
    } else {
        /* Push radially */
        offset = DECIDE(static_s || static_c, c_overlap, c_overlap / 2);
        glm_vec3_normalize_to(diff, norm);
        norm[0] *= offset;
        norm[2] *= offset;

        rvec_s[0] = DECIDE(static_s, norm[0], 0.0f);
        rvec_s[2] = DECIDE(static_s, norm[2], 0.0f);
        rvec_c[0] = DECIDE(static_c, -norm[0], 0.0f);
        rvec_c[2] = DECIDE(static_c, -norm[2], 0.0f);
    }

    return 1;
}

/* Returns true if a sphere and AABB intersect.
 * Writes collision resolution vectors to `rvec_b` and `rvec_s`.
 * Reflects the given direction vectors along the collision normal */
ATRA_API int atra_sphere_cylinder_reflect(Sphere s, Cylinder c,
        float *rvec_s, float *rvec_c, float *dir_s, float *dir_c,
        int static_s, int static_c)
{
    if (atra_sphere_cylinder_resolve(s, c, rvec_s, rvec_c, static_s, static_c) == 0) {
        return 0;
    }
    atra_vec3_reflect(dir_s, rvec_s);
    atra_vec3_reflect(dir_c, rvec_c);

    return 1;
}


/* Returns true if a ray intersects a sphere */
ATRA_API
int
atra_ray_sphere_collision(Ray ray, Sphere sphere, vec3 *collision)
{
    vec3 m;
    float b, c, t, discr;

    glm_vec3_sub(ray.pos, sphere.pos, m);
    b = glm_dot(m, ray.dir);
    c = glm_dot(m, m) - sphere.r * sphere.r;

    if (c > 0.0f && b > 0.0f) {
        return 0;
    }
    discr = b * b - c;

    if (discr < 0.0f) {
        return 0;
    }
    t = -b - sqrtf(discr);
    t = (t < 0.0f) ? 0.0f : t;
    glm_vec3_copy(ray.pos, *collision);
    glm_vec3_muladds(ray.dir, t, *collision);

    return 1;
}

/* Returns true if a ray intersects an AABB */
/*ATRA_API
int
atra_ray_aabb_collision(Ray ray, AABB box)
{
    
}*/


/*
 *  Physics-Body Functions
 * */

/* Creates a new AABB as a physics shape */
ATRA_API
PhysShape
atra_physshape_create_aabb(vec3 min, vec3 max)
{
    PhysShape shape;

    shape.aabb = atra_aabb_create(min, max);

    return shape;
}

/* Creates a new sphere as a physics shape */
ATRA_API
PhysShape
atra_physshape_create_sphere(vec3 pos, float r)
{
    PhysShape shape;

    shape.sphere = atra_sphere_create(pos, r);

    return shape;
}

/* Creates a new cylinder as a physics shape */
ATRA_API
PhysShape
atra_physshape_create_cylinder(vec3 pos, float h, float r)
{
    PhysShape shape;

    shape.cylinder = atra_cylinder_create(pos, h, r);

    return shape;
}

/* Creates a new PhysShape with the given type */
ATRA_API
PhysBody
atra_physbody_create(uint shape_type, PhysShape shape)
{
    PhysBody pb = {0};

    pb.shape = shape;
    pb.shape_type = shape_type;

    return pb;
}

static void
atra_vec3_reflect(float *v, float *n)
{
    vec3 norm;

    glm_vec3_normalize_to(n, norm);

    const float dot = 2 * glm_vec3_dot(v, norm);

    norm[0] *= dot;
    norm[1] *= dot;
    norm[2] *= dot;
    
    glm_vec3_sub(v, norm, v);
}

static void
atra_vec2_reflect(float *v, float *n)
{
    vec2 norm;

    glm_vec2_normalize_to(n, norm);

    const float dot = 2 * glm_vec2_dot(v, norm);

    norm[0] *= dot;
    norm[1] *= dot;
    
    glm_vec2_sub(v, norm, v);
}

/* Returns true if two physics bodies are intersecting */
/*ATRA_API
int
atra_physbody_collision(PhysBody a, PhysBody b)
{
    
}*/
