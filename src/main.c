#include <corecrt_math.h>
#include <corecrt_memory.h>
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

#define PARTICLE_COUNT 2

Particle *p_front_b;
Particle *p_back_b;

QuadTree *p_tree;
Particle **p_collision_b;

void update_pos(int delta_t) {
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        p_back_b[i].pos.x += p_back_b[i].vel.x * delta_t;
        p_back_b[i].pos.y += p_back_b[i].vel.y * delta_t;
    }
}

void evaluate(float delta_t) {
    // TODO: cache dt for every particle
    float sim_t = 0;
    float min_dt = delta_t;

    int p1 = -1;
    Particle *p2 = NULL;

    // simulate for the durration of delta_t  
    while (sim_t < delta_t) {
        // TODO: just modify tree instead of rebuilding it every time
        quad_tree_clear(p_tree);
        for (int i = 0; i < PARTICLE_COUNT; i++)
            quad_tree_add_element(p_tree, &p_front_b[i], (Area){p_front_b[i].pos, (Vector){p_front_b[i].vel.x * sim_t, p_front_b[i].vel.y * sim_t}});

        for (int i = 0; i < PARTICLE_COUNT; i++) {
            int p_collision_count = 0; 
            quad_tree_get_elements(p_tree, (Area){p_front_b[i].pos, (Vector){p_front_b[i].vel.x * sim_t, p_front_b[i].vel.y * sim_t}}, (void **)p_collision_b, &p_collision_count);

            // skip if no other particles is in the same quadrant
            if (p_collision_count <= 1)
                continue;

            // find particles that collide before the next time step
            for (int j = 0; j < p_collision_count; j++) {
                if (&p_front_b[i] == p_collision_b[j])
                    continue;

                // (x(vb) - x(va))² + (y(vb) - y(va))²
                float a = (p_collision_b[i]->vel.x - p_front_b[i].vel.x) * (p_collision_b[i]->vel.x - p_front_b[i].vel.x)
                    + (p_collision_b[i]->vel.y - p_front_b[i].vel.y) * (p_collision_b[i]->vel.y - p_front_b[i].vel.y);
                // ((x(B) - x(A)) (x(vb) - x(va)) + (y(B) - y(A)) (y(vb) - y(va))) * 2
                float b = 2 * ((p_collision_b[i]->pos.x - p_front_b[i].pos.x) * (p_collision_b[i]->vel.x - p_front_b[i].vel.x)
                        + (p_collision_b[i]->pos.y - p_front_b[i].pos.y) * (p_collision_b[i]->vel.y - p_front_b[i].vel.y));
                // (x(B) - x(A))² + (y(B) - y(A))² - (ra + rb)²
                float c = (p_collision_b[i]->pos.x - p_front_b[i].pos.x) * (p_collision_b[i]->pos.x - p_front_b[i].pos.x)
                    + (p_collision_b[i]->pos.y - p_front_b[i].pos.y) * (p_collision_b[i]->pos.y - p_front_b[i].pos.y)
                    - (p_front_b[i].radius + p_collision_b[i]->radius) * (p_front_b[i].radius + p_collision_b[i]->radius);
                
                // only real numbers are important
                float D = b * b - 4 * a * c;
                if (D < 0)
                    continue;

                // (-b + sqrt(b² - 4a c)) / (2a)
                float tc1 = (-b + sqrt(D)) / (2 * a);
                // (-b + sqrt(b² - 4a c)) / (2a)
                float tc2 = (-b + sqrt(D)) / (2 * a);

                float tc;
                if (tc1 >= 0.0f)
                    if (tc2 >= 0 && tc2 < tc1)
                        tc = tc2;
                    else
                        tc = tc1;
                else if (tc2 >= 0.0f)
                    tc = tc2;
                else
                    continue;
                
                
                if (tc < min_dt) {
                    min_dt = tc;

                    p1 = i;
                    p2 = p_collision_b[j];
                }
            }
        }

        update_pos(min_dt);

        if (p1 != -1) {
            printf("collision!\n");
            // update backbuffer with the new velocities
            p_back_b[p1].vel.x = (p_front_b[p1].vel.x * (p_front_b[p1].mass - p2->mass) + 2.0f * p2->mass * p2->vel.x) / (p_front_b[p1].mass + p2->mass);
            p_back_b[p1].vel.y = (p_front_b[p1].vel.y * (p_front_b[p1].mass - p2->mass) + 2.0f * p2->mass * p2->vel.y) / (p_front_b[p1].mass + p2->mass);
        }

        SWAP(p_front_b, p_back_b);

        sim_t += min_dt;
        min_dt = delta_t - sim_t;
    }
}

void on_create() {
    p_front_b = malloc(sizeof(Particle) * PARTICLE_COUNT);
    p_back_b = malloc(sizeof(Particle) * PARTICLE_COUNT);

    p_tree = quad_tree_create((Area){(Vector){0, 0}, (Vector){get_width(), get_height()}}, 0);

    p_front_b[0] = (Particle){1.0f, 1.0f, (Vector){70, 50}, (Vector){1.002f, 0}};
    p_front_b[1] = (Particle){2.0f, 1.0f, (Vector){80, 50}, (Vector){0.0f, 0}};

    memcpy(p_back_b, p_front_b, sizeof(Particle) * PARTICLE_COUNT);
}

void on_update() {
    clear();
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        draw_circle(p_front_b[i].pos.x, p_front_b[i].pos.y, 0.0f);
    }
}

void key_cb(int key, int action, int flags) {
    if (key == KEY_SPACE && action == KEY_PRESS)
        evaluate(1.0f);
}

int main() {
    srand(time(NULL));

    engine_create(500, 500, 5, 5);
    engine_set_user_input(key_cb, NULL);
    scene_create(on_create, on_update, NULL);
    clear_color(COLOR_VERY_DARK_GREY);
    engine_start();
}