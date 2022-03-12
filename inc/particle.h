#ifndef H_PARTICLE
#define H_PARTICLE

#include "utils.h"

typedef struct {
    float mass;
    Vector position;
    Vector velocity;
} Particle;

#endif