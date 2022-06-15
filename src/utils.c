#define _USE_MATH_DEFINES
#include <math.h>
#include <stdbool.h>

#include "utils.h"

// Vector
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

Vector vector_mirror(Vector vec, Vector straight_vec) {
    float a = atan2(straight_vec.y, straight_vec.x);
    float s = straight_vec.y / straight_vec.x;

    // check special cases
    if (s == 0.0)
        return (Vector){vec.x, -vec.y};

    // (x(v) - y(v) / s) sin²(a)
    float dx = (vec.x - vec.y / s) * sin(a) * sin(a);
    // (x(v) - y(v) / s) sin(a) sin(π / 2 - a)
    float dy = (vec.x - vec.y / s) * sin(a) * sin(M_PI / 2 - a);

    return (Vector){vec.x - 2 * dx, vec.y + 2 * dy};
}

Vector vector_rotate(Vector vec, float ang) {
    // x' = x cos θ − y sin θ
    // y' = x sin θ + y cos θ
    return (Vector){vec.x * cos(ang) - vec.y * sin(ang), vec.x * sin(ang) + vec.y * cos(ang)};
}

Vector vector_from_angle(float ang) {
    return (Vector){cos(ang), sin(ang)};
}

float vector_to_angle(Vector vec) {
    return atan2(vec.y, vec.x);
}

// Area
bool area_contains(Area a1, Area a2) {
    return (a2.pos.x >= a1.pos.x) && (a2.pos.x + a2.size.x < a1.pos.x + a1.size.x) && (a2.pos.y >= a1.pos.y) && (a2.pos.y + a2.size.y < a1.pos.y + a1.size.y);
}

bool area_overlaps(Area a1, Area a2) {
    return (a1.pos.x < a2.pos.x + a2.size.x && a1.pos.x + a1.size.x >= a2.pos.x && a1.pos.y < a2.pos.y + a2.size.y && a1.pos.y + a1.size.y >= a2.pos.y);
}

// Straight
Straight straight_create_v(Vector vec, float b) {
    // slope of the straight
    float a = vec.y / vec.x;
    // angle to the x-axis
    float alpha = atan2(vec.y, vec.x);

    return (Straight){a, b, alpha};
}

Straight straight_create_p(Vector p1, Vector p2) {
    // slope of the straight
    float a = (p2.y - p1.y) / (p2.x - p1.x);
    // y-axis offset
    float b = p1.y - a * p1.x;
    // angle to the x-axis
    float alpha = atan2(p2.y - p1.y, p2.x - p1.x);

    return (Straight){a, b, alpha};
}

float straight_f(Straight s, float x) {
    return s.a * x + s.b;
}

Vector straight_lerp_v(Straight s, Vector p1, Vector p2, float val) {
    float f1 = sqrt(p1.x * p1.x + (p1.y - s.b) * (p1.y - s.b));
    float f2 = sqrt(p2.x * p2.x + (p2.y - s.b) * (p2.y - s.b));

    return straight_lerp_f(s, f1, f2, val);
}

Vector straight_lerp_f(Straight s, float p1, float p2, float val) {
    return vector_rotate((Vector){val * (p2 - p1) + p1, 0.0f}, s.alpha);
}

float straight_inv_lerp(Straight s, Vector vec, Vector p1, Vector p2) {
    float val = sqrt(vec.x * vec.x + (vec.y - s.b) * (vec.y - s.b));
    float f1 = sqrt(p1.x * p1.x + (p1.y - s.b) * (p1.y - s.b));
    float f2 = sqrt(p2.x * p2.x + (p2.y - s.b) * (p2.y - s.b));

    return inv_lerp(val, f1, f2);
}

float inv_lerp(float val, float p1, float p2) {
    return (val - p1) / (p2 - p1);
}