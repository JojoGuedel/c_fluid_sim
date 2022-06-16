#include <corecrt_math_defines.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

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
Straight straight_create_ab(float a, float b) {
    float alpha = atan(a);
    return (Straight){a, b, alpha};
}

Straight straight_create_alphab(float alpha, float b) {
    float a = tan(alpha);
    return (Straight){a, b, alpha};
}

Straight straight_create_pp(Vector p1, Vector p2) {
    // slope of the straight
    float a = (p2.y - p1.y) / (p2.x - p1.x);
    // y-axis offset
    float b = p1.y - a * p1.x;
    // angle to the x-axis
    float alpha = atan2(p2.y - p1.y, p2.x - p1.x);

    return (Straight){a, b, alpha};
}

float straight_func(Straight s, float x) {
    return s.a * x + s.b;
}

Vector straight_lerp(Straight s, Vector p1, Vector p2, float lerp) {
    return (Vector){lerp * (p2.x - p1.x) + p1.x, lerp * (p2.y - p1.y) + p1.y};
}

float straight_inv_lerp(Straight s, Vector p1, Vector p2, Vector vec) {
    float y = straight_func(s, vec.x) - vec.y;
    if(y < -10 || y > 10)
        printf("not on the straight\n");

    if (fmod(s.alpha, M_PI / 2.0f) > M_PI / 4.0f && fmod(s.alpha, M_PI_2 / 2.0f) < M_PI / 3.0f)
        return (vec.y - p1.y) / (p2.y - p1.y);
    else
        return (vec.x - p1.x) / (p2.x - p1.x);
}