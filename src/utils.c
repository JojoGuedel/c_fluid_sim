#include "utils.h"
#include <corecrt_math.h>
#include <stdbool.h>

Vector vector_add(Vector vec1, Vector vec2) {
    return (Vector){vec1.x + vec2.x, vec1.y + vec2.y};
}

Vector vector_sub(Vector vec1, Vector vec2) {
    return (Vector){vec1.x - vec2.x, vec1.y - vec2.y};
}

Vector vector_mlt(Vector vec, float a) {
    return (Vector){vec.x * a, vec.y * a};
}

Vector vector_div(Vector vec, float a) {
    return (Vector){vec.x / a, vec.y / a};
}

float vector_dot(Vector vec1, Vector vec2) {
    return vec1.x * vec2.x + vec1.y * vec2.y;
}

float vector_len(Vector vec) {
    return sqrt(vec.x * vec.x + vec.y * vec.y);
}

bool area_contains(Area a1, Area a2) {
    return (a2.pos.x >= a1.pos.x) && (a2.pos.x + a2.size.x < a1.pos.x + a1.size.x) && (a2.pos.y >= a1.pos.y) && (a2.pos.y + a2.size.y < a1.pos.y + a1.size.y);
}

bool area_overlaps(Area a1, Area a2) {
    return (a1.pos.x < a2.pos.x + a2.size.x && a1.pos.x + a1.size.x >= a2.pos.x && a1.pos.y < a2.pos.y + a2.size.y && a1.pos.y + a1.size.y >= a2.pos.y);
}