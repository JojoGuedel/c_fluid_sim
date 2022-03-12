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

#include "particle.h"
#include "quad_tree.h"
#include "utils.h"

#define min(a, b) a < b ? a : b;
#define PARTICLE_COUNT 10000

Particle *p_front;
Particle *p_back;

Particle **p_collision;

QuadTree *tree;

// Particle particles[PARTICLE_COUNT];

void on_create() {
    p_front = malloc(sizeof(Particle) * PARTICLE_COUNT);
    p_back = malloc(sizeof(Particle) * PARTICLE_COUNT);
    p_collision = malloc(sizeof(Particle *) * PARTICLE_COUNT);

    tree = quad_tree_create((Area){(Vector){0, 0}, (Vector){get_width(), get_height()}}, 0);

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        p_front[i] = (Particle){1.0f, (Vector){(float)(rand() % get_width()), (float)(rand() % get_height())}, (Vector){rand() % 1000 / 1000.0f - 0.5f, rand() % 1000 / 1000.0f - 0.5f}};
    }

    // p_front[0] = (Particle){1.0f, (Vector){50, 125}, (Vector){0.5f, 0}};
    // p_front[1] = (Particle){1.0f, (Vector){200, 125}, (Vector){0.0f, 0}};

    // memcpy(p_back, p_front, sizeof(Particle) * PARTICLE_COUNT);
}

void collide_bad() {
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

        bool collision = false;
        for (int j = 0; j < PARTICLE_COUNT; j++) {
            if (i == j)
                continue;

            if (1.0f >= (p_front[i].position.x - p_front[j].position.x) * (p_front[i].position.x - p_front[j].position.x) +
                            (p_front[i].position.y - p_front[j].position.y) * (p_front[i].position.y - p_front[j].position.y)) {

                // p_back[j].velocity.x = p_front[j].velocity.x * p_front[j].mass - p_front[i].mass) * (2 * _front[i].velocity.x * p_front[i].mass) / (p_front[i].mass + p_front[j].mass);
                // p_back[j].velocity.y = p_front[j].velocity.y * (p_front[j].mass - p_front[i].mass) * (2 * p_front[i].velocity.y * p_front[i].mass) (p_front[i].mass + p_front[j].mass);

                p_back[i].velocity.x =
                    (p_front[i].velocity.x * (p_front[i].mass - p_front[j].mass) + 2.0f * p_front[j].mass * p_front[j].velocity.x) / (p_front[i].mass + p_front[j].mass);

                p_back[i].velocity.y =
                    (p_front[i].velocity.y * (p_front[i].mass - p_front[j].mass) + 2.0f * p_front[j].mass * p_front[j].velocity.y) / (p_front[i].mass + p_front[j].mass);

                collision = true;
                break;
            }
        }
        if (!collision)
            p_back[i] = p_front[i];
    }
}

void collide() {
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

        int p_count = 0;
        // Particle **particles = malloc(sizeof(void *) * quad_tree_get_size_in_area(tree, (Area){p_front[i].position, (Vector){1, 1}}));
        quad_tree_get_elements(tree, (Area){p_front[i].position, (Vector){1, 1}}, (void **)p_collision, &p_count);

        for (int j = 0; j < p_count; j++) {

            if (&p_front[i] == p_collision[j])
                continue;

            if (1.0f >= (p_front[i].position.x - p_collision[j]->position.x) * (p_front[i].position.x - p_collision[j]->position.x) +
                            (p_front[i].position.y - p_collision[j]->position.y) * (p_front[i].position.y - p_collision[j]->position.y)) {

                p_back[i].velocity.x = (p_front[i].velocity.x * (p_front[i].mass - p_collision[j]->mass) + 2.0f * p_collision[j]->mass * p_collision[j]->velocity.x) /
                                       (p_front[i].mass + p_collision[j]->mass);
                p_back[i].velocity.y = (p_front[i].velocity.y * (p_front[i].mass - p_collision[j]->mass) + 2.0f * p_collision[j]->mass * p_collision[j]->velocity.y) /
                                       (p_front[i].mass + p_collision[j]->mass);
                                    
                break;
            }
        }

    }
}


void on_update() {
    clear();

    memcpy(p_back, p_front, sizeof(Particle) * PARTICLE_COUNT);
    quad_tree_clear(tree);
    for (int i = 0; i < PARTICLE_COUNT; i++)
        quad_tree_add_element(tree, &p_front[i], (Area){p_front[i].position, (Vector){1, 1}});    

    // collide_bad();
    collide();
    printf("dt: %f\n", delta_time);

    for (int i = 0; i < PARTICLE_COUNT; i++)
        p_back[i].position = vector_add(p_front[i].position, p_back[i].velocity);

    // Swap buffers
    Particle *temp = p_front;
    p_front = p_back;
    p_back = temp;

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        // int col = 255 * ((float)(i + 1) / (float)PARTICLE_COUNT);
        // color((Color){col, col, col, 255});
        draw_pixel(p_front[i].position.x, p_front[i].position.y);
    }
}

int main() {
    srand(time(NULL));

    engine_create(1000, 500, 1, 1);
    scene_create(on_create, on_update, NULL);
    clear_color(COLOR_VERY_DARK_GREY);
    engine_start();
}