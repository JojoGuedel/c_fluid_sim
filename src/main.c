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
// this is needed because of floating point precision errors
#define TOLERANCE 0.0000001f

Particle *p_buffer;

QuadTree *p_tree;
Particle **p_collision_b;

float vel_after_collision(float v1, float m1, float v2, float m2) {
    return (v1 * (m1 - m2) + 2.0f * m2 * v2) / (m1 + m2);
}

void evaluate(float delta_t) {
    // TODO: cache dt for every particle
    float sim_t = 0;
    float min_dt = delta_t;

    Particle *p1 = NULL;
    Particle *p2 = NULL;

    // simulate for the durration of delta_t  
    while (sim_t < delta_t) {
        // TODO: just modify tree instead of rebuilding it every time
        quad_tree_clear(p_tree);
        for (int i = 0; i < PARTICLE_COUNT; i++)
            quad_tree_add_element(p_tree, &p_buffer[i], (Area){p_buffer[i].pos, (Vector){p_buffer[i].vel.x * sim_t, p_buffer[i].vel.y * sim_t}});

        for (int i = 0; i < PARTICLE_COUNT; i++) {
            int p_collision_count = 0; 
            quad_tree_get_elements(p_tree, (Area){p_buffer[i].pos, (Vector){p_buffer[i].vel.x * sim_t, p_buffer[i].vel.y * sim_t}}, (void **)p_collision_b, &p_collision_count);

            // skip if no other particles is in the same quadrant
            if (p_collision_count <= 1)
                continue;

            // find particles that collide before the next time step
            for (int j = 0; j < p_collision_count; j++) {
                if (i <= p_collision_b[j] - p_buffer)
                    continue;

                // (x(vb) - x(va))² + (y(vb) - y(va))²
                float a = (p_collision_b[j]->vel.x - p_buffer[i].vel.x) * (p_collision_b[j]->vel.x - p_buffer[i].vel.x)
                        + (p_collision_b[j]->vel.y - p_buffer[i].vel.y) * (p_collision_b[j]->vel.y - p_buffer[i].vel.y);
                // ((x(B) - x(A)) (x(vb) - x(va)) + (y(B) - y(A)) (y(vb) - y(va))) * 2
                float b = 2 * ((p_collision_b[j]->pos.x - p_buffer[i].pos.x) * (p_collision_b[j]->vel.x - p_buffer[i].vel.x)
                             + (p_collision_b[j]->pos.y - p_buffer[i].pos.y) * (p_collision_b[j]->vel.y - p_buffer[i].vel.y));
                // (x(B) - x(A))² + (y(B) - y(A))² - (ra + rb)²
                float c = (p_collision_b[j]->pos.x - p_buffer[i].pos.x) * (p_collision_b[j]->pos.x - p_buffer[i].pos.x)
                        + (p_collision_b[j]->pos.y - p_buffer[i].pos.y) * (p_collision_b[j]->pos.y - p_buffer[i].pos.y)
                        - (p_buffer[i].r + p_collision_b[j]->r) * (p_buffer[i].r + p_collision_b[j]->r);
                
                // only real numbers are important
                float D = b * b - 4 * a * c;
                if (D < 0)
                    continue;

                // (-b + sqrt(b² - 4a c)) / (2a)
                float tc1 = (-b + sqrt(D)) / (2 * a);
                // (-b - sqrt(b² - 4a c)) / (2a)
                float tc2 = (-b - sqrt(D)) / (2 * a);

                // determine the nearest collision in the future
                float tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;

                if (tc < TOLERANCE)
                    continue;

                color(COLOR_DARK_CYAN);
                draw_circle(p_buffer[i].pos.x + p_buffer[i].vel.x * tc1, p_buffer[i].pos.y + p_buffer[i].vel.y * tc1, p_buffer[i].r);
                draw_circle(p_collision_b[j]->pos.x + p_collision_b[j]->vel.x * tc1, p_collision_b[j]->pos.y + p_collision_b[j]->vel.y * tc1, p_collision_b[j]->r);

                color(COLOR_DARK_RED);
                draw_circle(p_buffer[i].pos.x + p_buffer[i].vel.x * tc2, p_buffer[i].pos.y + p_buffer[i].vel.y * tc2, p_buffer[i].r);
                draw_circle(p_collision_b[j]->pos.x + p_collision_b[j]->vel.x * tc2, p_collision_b[j]->pos.y + p_collision_b[j]->vel.y * tc2, p_collision_b[j]->r);
                color(COLOR_WHITE);
                
                if (tc < min_dt) {
                    min_dt = tc;

                    p1 = &p_buffer[i];
                    p2 = p_collision_b[j];
                }
            }
        }

        // update position of all particles
        for (int i = 0; i < PARTICLE_COUNT; i++) {
            p_buffer[i].pos.x += p_buffer[i].vel.x * min_dt;
            p_buffer[i].pos.y += p_buffer[i].vel.y * min_dt;
        }

        // update particles with the new velocities
        if (p1 != NULL) {
            printf("collision!\n");
            // Vector v1 = p1->vel;
            // Vector v2 = p2->vel;

            // p1->vel.x = (v1.x * (p1->mass - p2->mass) + 2.0 * p2->mass * v2.x) / (p1->mass + p2->mass);
            // p1->vel.y = (v1.y * (p1->mass - p2->mass) + 2.0 * p2->mass * v2.y) / (p1->mass + p2->mass);

            // p2->vel.x = (v2.x * (p2->mass - p1->mass) + 2.0 * p1->mass * v1.x) / (p1->mass + p2->mass);
            // p2->vel.y = (v2.y * (p2->mass - p1->mass) + 2.0 * p1->mass * v1.y) / (p1->mass + p2->mass);
            
            // TODO: do this without vectors but with trigonometry
            Vector n = vector_sub(p2->pos, p1->pos);
            Vector un = vector_div(n, vector_len(n));
            Vector ut = (Vector){-un.y , un.x};

            float n1 = vector_dot(un, p1->vel);
            float t1 = vector_dot(ut, p1->vel);
            float n2 = vector_dot(un, p2->vel);
            float t2 = vector_dot(ut, p2->vel);

            float n1_a = vel_after_collision(n1, p1->mass, n2, p2->mass);
            float n2_a = vel_after_collision(n2, p2->mass, n1, p1->mass);

            Vector v1n = vector_mlt(un, n1_a);
            Vector v1t = vector_mlt(ut, t1);
            Vector v2n = vector_mlt(un, n2_a);
            Vector v2t = vector_mlt(ut, t2);

            p1->vel = vector_add(v1n, v1t);
            p2->vel = vector_add(v2n, v2t);

            // clear particle pointers
            p1 = NULL;
            p2 = NULL;
        }

        // keep track of the simulated time
        sim_t += min_dt;
        min_dt = delta_t - sim_t;
    }
}

void on_create() {
    p_buffer = malloc(sizeof(Particle) * PARTICLE_COUNT);

    p_tree = quad_tree_create((Area){(Vector){0, 0}, (Vector){get_width(), get_height()}}, 0);
    p_collision_b = malloc(sizeof(Particle *) * PARTICLE_COUNT);

    p_buffer[0] = (Particle){3.0, 4.0, (Vector){70, 43}, (Vector){2.0, 0}};
    p_buffer[1] = (Particle){1.0, 4.0, (Vector){80, 50}, (Vector){0.0, 0}};
}

void on_update() {
    clear();
    evaluate(delta_time);
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        draw_circle(p_buffer[i].pos.x, p_buffer[i].pos.y, p_buffer[i].r);
    }
}

void key_cb(int key, int action, int flags) {
    if (key == KEY_SPACE && action == KEY_PRESS)
        evaluate(1.0);
}

int main() {
    srand(time(NULL));

    engine_create(500, 500, 5, 5);
    // engine_set_user_input(key_cb, NULL);
    scene_create(on_create, on_update, NULL);
    clear_color(COLOR_VERY_DARK_GREY);
    engine_start();
}