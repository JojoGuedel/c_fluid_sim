#ifndef H_PARTICLE
#define H_PARTICLE

#include "utils.h"

typedef struct {
    float mass;
    float r;
    Vector pos;
    Vector vel;
} Particle;

#endif