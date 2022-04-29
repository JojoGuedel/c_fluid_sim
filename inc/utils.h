#ifndef H_UTILS
#define H_UTILS

#include <stdbool.h>

#define SWAP(a, b) { __typeof__(a) temp = a; a = b; b = temp; }
#define LERP(s, d, i) s + d*i

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


typedef struct {
    Vector pos;
    Vector size;
} Area;

bool area_contains(Area a1, Area a2);
bool area_overlaps(Area a1, Area a2);


#endif