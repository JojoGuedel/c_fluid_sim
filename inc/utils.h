#ifndef H_UTILS
#define H_UTILS

#include <stdbool.h>
typedef struct {
    float x;
    float y;
} Vector;

Vector vector_add(Vector vec1, Vector vec2);

typedef struct {
    Vector pos;
    Vector size;
} Area;

bool area_contains(Area a1, Area a2);
bool area_overlaps(Area a1, Area a2);

#endif