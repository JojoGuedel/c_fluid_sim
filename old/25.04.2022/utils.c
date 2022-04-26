#include "utils.h"
#include <stdbool.h>

Vector vector_add(Vector vec1, Vector vec2) {
    vec1.x += vec2.x;
    vec1.y += vec2.y;
    return vec1;
}

bool area_contains(Area a1, Area a2) {
    return (a2.pos.x >= a1.pos.x) && (a2.pos.x + a2.size.x < a1.pos.x + a1.size.x) && (a2.pos.y >= a1.pos.y) && (a2.pos.y + a2.size.y < a1.pos.y + a1.size.y);
}

bool area_overlaps(Area a1, Area a2) {
    return (a1.pos.x < a2.pos.x + a2.size.x && a1.pos.x + a1.size.x >= a2.pos.x && a1.pos.y < a2.pos.y + a2.size.y && a1.pos.y + a1.size.y >= a2.pos.y);
}