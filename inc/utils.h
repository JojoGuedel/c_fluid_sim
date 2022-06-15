#ifndef H_UTILS
#define H_UTILS

#include <stdbool.h>

#define SWAP(a, b) { __typeof__(a) temp = a; a = b; b = temp; }
#define LERP(s, d, i) s + d*i
#define CLAMP(val, min, max) max(min(val, max), min)

// Vector
typedef struct {
    double x;
    double y;
} Vector;

Vector vector_add(Vector vec1, Vector vec2);
Vector vector_sub(Vector vec1, Vector vec2);
Vector vector_mlt(Vector vec, float a);
Vector vector_div(Vector vec, float a);
float vector_dot(Vector vec1, Vector vec2);
float vector_len(Vector vec);

Vector vector_mirror(Vector vec, Vector straight_vec);
Vector vector_rotate(Vector vec, float ang);
Vector vector_from_angle(float ang);
float vector_to_angle(Vector vec);

// Area
typedef struct {
    Vector pos;
    Vector size;
} Area;

bool area_contains(Area a1, Area a2);
bool area_overlaps(Area a1, Area a2);

// Straight
// f(x) = ax + b 
// with angle alpha
typedef struct{
    float a;
    float b;
    float alpha;
} Straight;

Straight straight_create_v(Vector vec, float b);
Straight straight_create_p(Vector p1, Vector p2);

float straight_f(Straight s, float x);

Vector straight_lerp_v(Straight s, Vector p1, Vector p2, float val);
Vector straight_lerp_f(Straight s, float p1, float p2, float val);
float straight_inv_lerp(Straight s, Vector vec, Vector p1, Vector p2);
float inv_lerp(float val, float p1, float p2);

#endif