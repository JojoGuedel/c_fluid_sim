#include <corecrt_math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "Pixa/color.h"
#include "Pixa/core.h"
#include "Pixa/graphics.h"
#include "Pixa/input.h"
#include "Pixa/log.h"
#include "Pixa/scene.h"

#include "particle.h"
#include "quad_tree.h"
#include "utils.h"

// #define min(a, b) a < b ? a : b;
#define PARTICLE_COUNT 2

Particle *p_front;
Particle *p_back;

Particle **p_collision;

QuadTree *P_tree;

// Particle particles[PARTICLE_COUNT];

void on_create() {
    p_front = malloc(sizeof(Particle) * PARTICLE_COUNT);
    p_back = malloc(sizeof(Particle) * PARTICLE_COUNT);
    p_collision = malloc(sizeof(Particle *) * PARTICLE_COUNT);

    P_tree = quad_tree_create((Area){(Vector){0, 0}, (Vector){get_width(), get_height()}}, 0);

    // for (int i = 0; i < PARTICLE_COUNT; i++) {
    //     p_front[i] = (Particle){1.0f, (Vector){(float)(rand() % get_width()), (float)(rand() % get_height())}, (Vector){((rand() % 1000) / 1000.0f - 0.5f) / 50.0f, ((rand() % 1000) / 1000.0f - 0.5f) / 50.0f}};
    // }

    p_front[0] = (Particle){1.0f, (Vector){70, 50}, (Vector){0.002f, 0}};
    p_front[1] = (Particle){2.0f, (Vector){80, 50}, (Vector){0.0f, 0}};

    memcpy(p_back, p_front, sizeof(Particle) * PARTICLE_COUNT);
}

void collide_bad() {
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        if (p_front[i].pos.x < 0) {
            p_back[i].pos.x = 0;
            p_back[i].vel.x *= -1;
            continue;
        } else if (p_front[i].pos.x > get_width()) {
            p_back[i].pos.x = get_width();
            p_back[i].vel.x *= -1;
            continue;
        }

        else if (p_front[i].pos.y < 0) {
            p_back[i].pos.y = 0;
            p_back[i].vel.y *= -1;
            continue;
        } else if (p_front[i].pos.y > get_height()) {
            p_back[i].pos.y = get_height();
            p_back[i].vel.y *= -1;
            continue;
        }

        bool collision = false;
        for (int j = 0; j < PARTICLE_COUNT; j++) {
            if (i == j)
                continue;

            if (1.0f >= (p_front[i].pos.x - p_front[j].pos.x) * (p_front[i].pos.x - p_front[j].pos.x) +
                            (p_front[i].pos.y - p_front[j].pos.y) * (p_front[i].pos.y - p_front[j].pos.y)) {

                // p_back[j].velocity.x = p_front[j].velocity.x * p_front[j].mass - p_front[i].mass) * (2 * _front[i].velocity.x * p_front[i].mass) / (p_front[i].mass + p_front[j].mass);
                // p_back[j].velocity.y = p_front[j].velocity.y * (p_front[j].mass - p_front[i].mass) * (2 * p_front[i].velocity.y * p_front[i].mass) (p_front[i].mass + p_front[j].mass);

                p_back[i].vel.x =
                    (p_front[i].vel.x * (p_front[i].mass - p_front[j].mass) + 2.0f * p_front[j].mass * p_front[j].vel.x) / (p_front[i].mass + p_front[j].mass);

                p_back[i].vel.y =
                    (p_front[i].vel.y * (p_front[i].mass - p_front[j].mass) + 2.0f * p_front[j].mass * p_front[j].vel.y) / (p_front[i].mass + p_front[j].mass);

                collision = true;
                break;
            }
        }
        if (!collision)
            p_back[i] = p_front[i];
    }
}

void collide_bad2() {
    float total_impuls = 0;

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        total_impuls += fabs(p_front[i].mass * p_front[i].vel.x);

        if (p_front[i].pos.x < 0) {
            p_back[i].pos.x = 0;
            p_back[i].vel.x *= -1;
            continue;
        } else if (p_front[i].pos.x > get_width()) {
            p_back[i].pos.x = get_width();
            p_back[i].vel.x *= -1;
            continue;
        }

        else if (p_front[i].pos.y < 0) {
            p_back[i].pos.y = 0;
            p_back[i].vel.y *= -1;
            continue;
        } else if (p_front[i].pos.y > get_height()) {
            p_back[i].pos.y = get_height();
            p_back[i].vel.y *= -1;
            continue;
        }

        int p_count = 0;
        // Particle **particles = malloc(sizeof(void *) * quad_tree_get_size_in_area(tree, (Area){p_front[i].position, (Vector){1, 1}}));
        quad_tree_get_elements(P_tree, (Area){p_front[i].pos, (Vector){1, 1}}, (void **)p_collision, &p_count);

        if (p_count == 1)
            continue;

        for (int j = 0; j < p_count; j++) {
            if (&p_front[i] == p_collision[j])
                continue;


            if (1.0f >= (p_front[i].pos.x - p_collision[j]->pos.x) * (p_front[i].pos.x - p_collision[j]->pos.x) +
                            (p_front[i].pos.y - p_collision[j]->pos.y) * (p_front[i].pos.y - p_collision[j]->pos.y)) {

                p_back[i].vel.x = (p_front[i].vel.x * (p_front[i].mass - p_collision[j]->mass) + 2.0f * p_collision[j]->mass * p_collision[j]->vel.x) /
                                       (p_front[i].mass + p_collision[j]->mass);
                p_back[i].vel.y = (p_front[i].vel.y * (p_front[i].mass - p_collision[j]->mass) + 2.0f * p_collision[j]->mass * p_collision[j]->vel.y) /
                                       (p_front[i].mass + p_collision[j]->mass);
                                    
                break;
            }
        }
    }

    // printf("impuls: %f\n", total_impuls);
}

void move() {
    float total_impuls = 0;

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        total_impuls += fabs(p_front[i].mass * p_front[i].vel.x);

        if (p_front[i].pos.x < 0) {
            p_back[i].pos.x = 0;
            p_back[i].vel.x *= -1;
            
            p_back[i].pos.x = p_front[i].pos.x + p_front[i].velocity.xvel
            p_back[i].pos.y = p_front[i].pos.y + p_front[i].velocity.yvel
        } else if (p_front[i].pos.x > get_width()) {
            p_back[i].pos.x = get_width();
            p_back[i].vel.x *= -1;

            p_back[i].pos.x = p_front[i].pos.x + p_back[i].velocity.xvel
            p_back[i].pos.y = p_front[i].pos.y + p_back[i].velocity.yvel
            continue;
        }

        else if (p_front[i].pos.y < 0) {
            p_back[i].pos.y = 0;
            p_back[i].vel.y *= -1;

            p_back[i].pos.x = p_front[i].pos.x + p_back[i].velocity.xvel
            p_back[i].pos.y = p_front[i].pos.y + p_back[i].velocity.yvel
            continue;
        } else if (p_front[i].pos.y > get_height()) {
            p_back[i].pos.y = get_height();
            p_back[i].vel.y *= -1;

            p_back[i].pos.x = p_front[i].pos.x + p_back[i].velocity.xvel
            p_back[i].pos.y = p_front[i].pos.y + p_back[i].velocity.yvel
            continue;
        }

        int p_count = 0;
        // Particle **particles = malloc(sizeof(void *) * quad_tree_get_size_in_area(tree, (Area){p_front[i].position, (Vector){1, 1}}));
        quad_tree_get_elements(P_tree, (Area){p_front[i].pos, (Vector){1, 1}}, (void **)p_collision, &p_count);

        if (p_count == 1) {
            p_back[i].pos.x = p_front[i].pos.x + p_back[i].velocity.xvel
            p_back[i].pos.y = p_front[i].pos.y + p_back[i].velocity.yvel
            continue;
        }

        bool handled = false;

        for (int j = 0; j < p_count; j++) {
            if (&p_front[i] == p_collision[j])
                continue;

            float tx_c = 0;
            float ty_c = 0;
            
            if (p_front[j].vel.x - p_front[i].vel.x == 0) {
                if (fabs(p_front[i].pos.x - p_collision[j]->pos.x) > 1.0f)
                    continue;
            }
            else
                tx_c = (1 + p_front[i].pos.x - p_front[j].pos.x) / (p_front[j].velocity.xvelnt[i].velocity.xvel
            
            if (p_front[j].vel.y - p_front[i].vel.y == 0) {
                if (fabs(p_front[i].pos.y - p_collision[j]->pos.y) > 1.0f)
                    continue;
            }
            else
                ty_c = (1 + p_front[i].pos.y - p_front[j].pos.y) / (p_front[j].velocity.yvelnt[i].velocity.yvel


            if (tx_c * tx_c + ty_c * ty_c <= 1.0f) {
                p_back[i].vel.x = (p_front[i].vel.x * (p_front[i].mass - p_collision[j]->mass) + 2.0f * p_collision[j]->mass * p_collision[j]->vel.x) /
                                       (p_front[i].mass + p_collision[j]->mass);
                p_back[i].vel.y = (p_front[i].vel.y * (p_front[i].mass - p_collision[j]->mass) + 2.0f * p_collision[j]->mass * p_collision[j]->vel.y) /
                                       (p_front[i].mass + p_collision[j]->mass);

                p_back[i].pos.x = p_front[i].pos.x + p_front[i].velocity.xvel+ p_back[i].velocity.xvel - tx_c);
                p_back[i].pos.y = p_front[i].pos.y + p_front[i].velocity.yvel+ p_back[i].velocity.yvel - ty_c);
                handled = true;
                break;
            }
        }

        if (!handled) {
            p_back[i].pos.x = p_front[i].pos.x + p_back[i].velocity.xvel
            p_back[i].pos.y = p_front[i].pos.y + p_back[i].velocity.yvel
        }
    }

    // printf("impuls: %f\n", total_impuls);
}


void on_update() {
    clear();

    memcpy(p_back, p_front, sizeof(Particle) * PARTICLE_COUNT);
    quad_tree_clear(P_tree);
    for (int i = 0; i < PARTICLE_COUNT; i++)
        quad_tree_add_element(P_tree, &p_front[i], (Area){p_front[i].pos, (Vector){1, 1}});    

    // collide_bad();
    collide_bad2();

    for (int i = 0; i < PARTICLE_COUNT; i++)
        p_back[i].pos = vector_add(p_front[i].pos, p_back[i].velocity);vel

    // Swap buffers
    Particle *temp = p_front;
    p_front = p_back;
    p_back = temp;

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        // int col = 255 * ((float)(i + 1) / (float)PARTICLE_COUNT);
        // color((Color){col, col, col, 255});
        color(COLOR_RED);
        draw_pixel(p_back[i].pos.x, p_back[i].pos.y);
        color(COLOR_WHITE);
        draw_pixel(p_front[i].pos.x, p_front[i].pos.y);
    }
    printf("dt: %f\n", delta_time);
}

int main() {
    srand(time(NULL));

    engine_create(500, 500, 5, 5);
    scene_create(on_create, on_update, NULL);
    clear_color(COLOR_VERY_DARK_GREY);
    engine_start();
}