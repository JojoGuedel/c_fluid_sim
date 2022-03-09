#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Pixa/color.h"
#include "Pixa/core.h"
#include "Pixa/graphics.h"
#include "Pixa/input.h"
#include "Pixa/log.h"
#include "Pixa/scene.h"

#include "fluid.h"

#define min(a, b) a < b ? a : b;
#define PARTICLE_COUNT 100

#define VECTOR_IEQUALS(vec1, vec2) ((int)(vec1).x == (int)(vec2).x && (int)(vec1).y == (int)(vec2).y)

typedef struct {
    float x;
    float y;
} Vector;

Vector vector_add(Vector vec1, Vector vec2) {
    vec1.x += vec2.x;
    vec1.y += vec2.y;
    return vec1;
}

typedef struct {
    float mass;
    Vector position;
    Vector velocity;
} Particle;

Particle *p_front;
Particle *p_back;

// Particle particles[PARTICLE_COUNT];

void on_create() {

    p_front = malloc(sizeof(Particle) * PARTICLE_COUNT);

    // p_front[0] = (Particle){1.0f, (Vector){50.0f, 125.0f}, (Vector){1.0f, 0.0f}};
    // p_front[1] = (Particle){1.0f, (Vector){200.0f, 125.0f}, (Vector){0.0f, 0.0f}};

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        p_front[i] = (Particle){1.0f, (Vector){rand() % get_width(), rand() % get_height()}, (Vector){rand() % 1000 / 1000.0f - 0.5f, rand() % 1000 / 1000.0f - 0.5f}};
    }

    p_back = malloc(sizeof(Particle) * PARTICLE_COUNT);
    memcpy(p_back, p_front, sizeof(Particle) * PARTICLE_COUNT);
}

void on_update() {
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        if (p_front[i].position.x < 0) {
            p_back[i].position.x = 0;
            p_back[i].velocity.x *= -1;
            continue;
        } else if (p_front[i].position.x > get_width()) {
            p_back[i].position.x = get_width();
            p_back[i].velocity.x *= -1;
            continue;
        }

        else if (p_front[i].position.y < 0) {
            p_back[i].position.y = 0;
            p_back[i].velocity.y *= -1;
            continue;
        } else if (p_front[i].position.y > get_height()) {
            p_back[i].position.y = get_height();
            p_back[i].velocity.y *= -1;
            continue;
        }

        // TODO: implement quad-layer tree
        bool collision = false;
        for (int j = 0; j < PARTICLE_COUNT; j++) {
            if (i == j)
                continue;

            // Particle *other = &particles[j];

            // TODO: implement radius
            if (1.0f >= (p_front[i].position.x - p_front[j].position.x) * (p_front[i].position.x - p_front[j].position.x) + (p_front[i].position.y - p_front[j].position.y) * (p_front[i].position.y - p_front[j].position.y)) {
                // p_back[j].velocity.x = p_front[j].velocity.x * (p_front[j].mass - p_front[i].mass) * (2 * p_front[i].velocity.x * p_front[i].mass) / (p_front[i].mass + p_front[j].mass);
                // p_back[j].velocity.y = p_front[j].velocity.y * (p_front[j].mass - p_front[i].mass) * (2 * p_front[i].velocity.y * p_front[i].mass) / (p_front[i].mass + p_front[j].mass);
                
                // p_back[i].position = p_front[i].position;

                p_back[i].velocity.x = (p_front[i].velocity.x * (p_front[i].mass - p_front[j].mass) + 2.0f * p_front[j].mass * p_front[j].velocity.x) / (p_front[i].mass + p_front[j].mass);
                p_back[i].velocity.y = (p_front[i].velocity.y * (p_front[i].mass - p_front[j].mass) + 2.0f * p_front[j].mass * p_front[j].velocity.y) / (p_front[i].mass + p_front[j].mass);

                // printf("%f\n", p_back[i].velocity.x);

                collision = true;
                break;
            }
        }

        if (!collision)
            p_back[i] = p_front[i];
    }

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        // TODO: delta time... 
        p_back[i].position = vector_add(p_front[i].position, p_back[i].velocity);
    }

    // Swap buffers
    Particle *temp = p_front;
    p_front = p_back;
    p_back = temp;

    clear();
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        // int col = 255 * ((float)(i + 1) / (float)PARTICLE_COUNT);
        // color((Color){col, col, col, 255});
        draw_pixel(p_front[i].position.x, p_front[i].position.y);
    }
}

int main() {
    srand(time(NULL));

    engine_create(500, 500, 2, 2);
    scene_create(on_create, on_update, NULL);
    clear_color(COLOR_VERY_DARK_GREY);
    engine_start();
}