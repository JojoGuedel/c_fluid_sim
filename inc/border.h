#ifndef LINE_H
#define LINE_H

#include "utils.h"

typedef struct {
    Area area;

    float r;

    float a;
    float s;
    float m;

    // Vector delta; // the offset is constant if all radii (of the Particles) are equal
} Border;

Border border_create(Vector pos1, Vector pos2, float thickness);

#endif