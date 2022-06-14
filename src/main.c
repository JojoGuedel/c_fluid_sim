#include <corecrt_math.h>
#include <corecrt_math_defines.h>
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
#include "border.h"
#define _USE_MATH_DEFINES
#include "math.h"

#define PARTICLE_COUNT 2
// this is needed because of floating point precision errors
#define TOLERANCE 0.0000001f

Particle *part_buf;
QuadTree *part_tree;
Particle **part_area_buf;

int border_count;
Border *border_buf;
QuadTree *border_tree;
Border **border_area_buf;

Area particle_area(Particle *p, float dt) {
    Vector pos = (Vector){p->pos.x - p->r + p->vel.x * dt, p->pos.y - p->r + p->vel.y * dt};
    Vector size = (Vector){2 * p->r + p->vel.x * dt, 2 * p->r + p->vel.y * dt};
    return (Area){pos, size};
}

void evaluateParticles(float delta_t) {
    // TODO: cache dt for every particle
    float sim_t = 0;
    float min_dt = delta_t;
    int sim_steps = 0;

    Particle *p1 = NULL;
    Particle *p2 = NULL;

    // simulate for the durration of delta_t  
    while (sim_t < delta_t) {
        sim_steps += 1;
        // TODO: just modify tree instead of rebuilding it every time
        quad_tree_clear(part_tree);
        for (int i = 0; i < PARTICLE_COUNT; i++)
            quad_tree_add_element(part_tree, &part_buf[i], particle_area(&part_buf[i], min_dt));

        for (int i = 0; i < PARTICLE_COUNT; i++) {
            int p_collision_count = 0; 
            quad_tree_get_elements(part_tree, particle_area(&part_buf[i], min_dt), (void **)part_area_buf, &p_collision_count);

            // skip if no other particles is in the same quadrant
            if (p_collision_count <= 1)
                continue;

            // find particles that collide before the next time step
            for (int j = 0; j < p_collision_count; j++) {
                if (i <= part_area_buf[j] - part_buf)
                    continue;

                // (x(vb) - x(va))² + (y(vb) - y(va))²
                float a = (part_area_buf[j]->vel.x - part_buf[i].vel.x) * (part_area_buf[j]->vel.x - part_buf[i].vel.x)
                        + (part_area_buf[j]->vel.y - part_buf[i].vel.y) * (part_area_buf[j]->vel.y - part_buf[i].vel.y);
                // ((x(B) - x(A)) (x(vb) - x(va)) + (y(B) - y(A)) (y(vb) - y(va))) * 2
                float b = 2 * ((part_area_buf[j]->pos.x - part_buf[i].pos.x) * (part_area_buf[j]->vel.x - part_buf[i].vel.x)
                             + (part_area_buf[j]->pos.y - part_buf[i].pos.y) * (part_area_buf[j]->vel.y - part_buf[i].vel.y));
                // (x(B) - x(A))² + (y(B) - y(A))² - (ra + rb)²
                float c = (part_area_buf[j]->pos.x - part_buf[i].pos.x) * (part_area_buf[j]->pos.x - part_buf[i].pos.x)
                        + (part_area_buf[j]->pos.y - part_buf[i].pos.y) * (part_area_buf[j]->pos.y - part_buf[i].pos.y)
                        - (part_buf[i].r + part_area_buf[j]->r) * (part_buf[i].r + part_area_buf[j]->r);
                
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

                // color(COLOR_DARK_CYAN);
                // draw_circle(p_buffer[i].pos.x + p_buffer[i].vel.x * tc1, p_buffer[i].pos.y + p_buffer[i].vel.y * tc1, p_buffer[i].r);
                // draw_circle(p_collision_b[j]->pos.x + p_collision_b[j]->vel.x * tc1, p_collision_b[j]->pos.y + p_collision_b[j]->vel.y * tc1, p_collision_b[j]->r);

                // color(COLOR_DARK_RED);
                // draw_circle(p_buffer[i].pos.x + p_buffer[i].vel.x * tc2, p_buffer[i].pos.y + p_buffer[i].vel.y * tc2, p_buffer[i].r);
                // draw_circle(p_collision_b[j]->pos.x + p_collision_b[j]->vel.x * tc2, p_collision_b[j]->pos.y + p_collision_b[j]->vel.y * tc2, p_collision_b[j]->r);
                // color(COLOR_WHITE);
                
                if (tc < min_dt) {
                    min_dt = tc;

                    p1 = &part_buf[i];
                    p2 = part_area_buf[j];
                }
            }

            // printf("%i\n", sim_steps);
        }

        // update position of all particles
        for (int i = 0; i < PARTICLE_COUNT; i++) {
            part_buf[i].pos.x += part_buf[i].vel.x * min_dt;
            part_buf[i].pos.y += part_buf[i].vel.y * min_dt;

            if (part_buf[i].pos.x > get_width())
                part_buf[i].pos.x = 0;
            if (part_buf[i].pos.x < 0)
                part_buf[i].pos.x = get_width();
            
            if (part_buf[i].pos.y > get_height())
                part_buf[i].pos.y = 0;
            if (part_buf[i].pos.y < 0)
                part_buf[i].pos.y = get_height();
        }

        // update particles with the new velocities
        if (p1 != NULL) {
            // TODO: do this without vectors but with trigonometry
            Vector n = vector_sub(p2->pos, p1->pos);
            Vector un = vector_div(n, vector_len(n));
            Vector ut = (Vector){-un.y , un.x};

            float n1 = vector_dot(un, p1->vel);
            float t1 = vector_dot(ut, p1->vel);
            float n2 = vector_dot(un, p2->vel);
            float t2 = vector_dot(ut, p2->vel);

            float n1_a = (n1 * (p1->mass - p2->mass) + 2.0f * p2->mass * n2) / (p1->mass + p2->mass);
            float n2_a = (n2 * (p2->mass - p1->mass) + 2.0f * p1->mass * n1) / (p1->mass + p2->mass);

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

void evaluate(float delta_t) {
    // TODO: cache dt for every particle
    float sim_t = 0;
    float min_dt = delta_t;
    int sim_steps = 0;

    Particle *p1 = NULL;
    Particle *p2 = NULL;

    // simulate for the durration of delta_t  
    while (sim_t < delta_t) {
        sim_steps += 1;
        // TODO: just modify tree instead of rebuilding it every time
        quad_tree_clear(part_tree);
        for (int i = 0; i < PARTICLE_COUNT; i++)
            quad_tree_add_element(part_tree, &part_buf[i], particle_area(&part_buf[i], min_dt));

        for (int i = 0; i < PARTICLE_COUNT; i++) {
            int part_area_count = 0; 
            Area part_area = particle_area(&part_buf[i], min_dt);
            quad_tree_get_elements(part_tree, part_area, (void **)part_area_buf, &part_area_count);

            // skip if no other particles is in the same quadrant
            if (part_area_count <= 1)
                continue;

            // find the next collision with another particle
            for (int j = 0; j < part_area_count; j++) {
                // skip particles that allready have been checked
                if (i <= part_area_buf[j] - part_buf)
                    continue;

                // (x(vb) - x(va))² + (y(vb) - y(va))²
                float a = (part_area_buf[j]->vel.x - part_buf[i].vel.x) * (part_area_buf[j]->vel.x - part_buf[i].vel.x)
                        + (part_area_buf[j]->vel.y - part_buf[i].vel.y) * (part_area_buf[j]->vel.y - part_buf[i].vel.y);
                // ((x(B) - x(A)) (x(vb) - x(va)) + (y(B) - y(A)) (y(vb) - y(va))) * 2
                float b = 2 * ((part_area_buf[j]->pos.x - part_buf[i].pos.x) * (part_area_buf[j]->vel.x - part_buf[i].vel.x)
                             + (part_area_buf[j]->pos.y - part_buf[i].pos.y) * (part_area_buf[j]->vel.y - part_buf[i].vel.y));
                // (x(B) - x(A))² + (y(B) - y(A))² - (ra + rb)²
                float c = (part_area_buf[j]->pos.x - part_buf[i].pos.x) * (part_area_buf[j]->pos.x - part_buf[i].pos.x)
                        + (part_area_buf[j]->pos.y - part_buf[i].pos.y) * (part_area_buf[j]->pos.y - part_buf[i].pos.y)
                        - (part_buf[i].r + part_area_buf[j]->r) * (part_buf[i].r + part_area_buf[j]->r);
                
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

                // color(COLOR_DARK_CYAN);
                // draw_circle(p_buffer[i].pos.x + p_buffer[i].vel.x * tc1, p_buffer[i].pos.y + p_buffer[i].vel.y * tc1, p_buffer[i].r);
                // draw_circle(p_collision_b[j]->pos.x + p_collision_b[j]->vel.x * tc1, p_collision_b[j]->pos.y + p_collision_b[j]->vel.y * tc1, p_collision_b[j]->r);

                // color(COLOR_DARK_RED);
                // draw_circle(p_buffer[i].pos.x + p_buffer[i].vel.x * tc2, p_buffer[i].pos.y + p_buffer[i].vel.y * tc2, p_buffer[i].r);
                // draw_circle(p_collision_b[j]->pos.x + p_collision_b[j]->vel.x * tc2, p_collision_b[j]->pos.y + p_collision_b[j]->vel.y * tc2, p_collision_b[j]->r);
                // color(COLOR_WHITE);
                
                if (tc < min_dt) {
                    min_dt = tc;

                    p1 = &part_buf[i];
                    p2 = part_area_buf[j];
                }
            }

            // for (int j = 0; j < border_count; j++) {
            //     // TODO: check if it would be faster without if if-statement with area_overlaps()
            //     if (area_contains(part_area, border_buf[i].area)) {
            //         // rotate the velocityvector
            //         float vy = sin(border_buf[i].alpha) * part_buf[i].pos.x - cos(border_buf[i].alpha) * part_buf[i].pos.y;

            //         // (r_A + r_B + x(A_0) - x(B_0)) / (x(vb) - x(va))
            //         float tc1 = (part_buf[i].r + part_buf[i].pos.y - border_buf[i].pos1.y) / (-part_buf[i].vel.y);
            //         // (-r_A - r_B + x(A_0) - x(B_0)) / (x(vb) - x(va))
            //         float tc2 = (part_buf[i].r + part_buf[i].pos.y - border_buf[i].pos1.y) / (-part_buf[i].vel.y);

            //         float tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;
            //     }
            // }

            int border_area_count = 0; 
            quad_tree_get_elements(border_tree, part_area, (void **)border_area_buf, &border_area_count);

            for (int j = 0; j < border_area_count; j++) {
                // ((r_c + r_l) / sin(π / 2 - a) + m - y(P) + x(P) s) / (y(v) - x(v) s)
                float tc1 = ((part_buf[i].r + border_area_buf[j]->r)/sin(M_PI / 2.0 - border_area_buf[j]->a) + border_area_buf[j]->m - part_buf[i].pos.x * border_area_buf[j]->s) / (part_buf[i].vel.y - part_buf[i].vel.x * border_area_buf[j]->s);
                float tc2 = ((-part_buf[i].r - border_area_buf[j]->r)/sin(M_PI / 2.0 - border_area_buf[j]->a) + border_area_buf[j]->m - part_buf[i].pos.x * border_area_buf[j]->s) / (part_buf[i].vel.y - part_buf[i].vel.x * border_area_buf[j]->s);

                // determine the nearest collision in the future
                float tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;

                // if (tc < TOLERANCE)
                //     continue;
                
                // printf("%f, %f, %f\n", tc1, tc2, tc);

                if (tc < min_dt) {
                    // min_dt = tc;

                    // p1 = &part_buf[i];
                    // p2 = NULL;

                    printf("collision :D");
                }
            }
        }

        // update position of all particles
        for (int i = 0; i < PARTICLE_COUNT; i++) {
            part_buf[i].pos.x += part_buf[i].vel.x * min_dt;
            part_buf[i].pos.y += part_buf[i].vel.y * min_dt;

            if (part_buf[i].pos.x > get_width())
                part_buf[i].pos.x = 0;
            if (part_buf[i].pos.x < 0)
                part_buf[i].pos.x = get_width();
            
            if (part_buf[i].pos.y > get_height())
                part_buf[i].pos.y = 0;
            if (part_buf[i].pos.y < 0)
                part_buf[i].pos.y = get_height();
        }

        // update particles with the new velocities
        if (p1 != NULL && p2 != NULL) {
            // TODO: do this without vectors but with trigonometry
            Vector n = vector_sub(p2->pos, p1->pos);
            Vector un = vector_div(n, vector_len(n));
            Vector ut = (Vector){-un.y , un.x};

            float n1 = vector_dot(un, p1->vel);
            float t1 = vector_dot(ut, p1->vel);
            float n2 = vector_dot(un, p2->vel);
            float t2 = vector_dot(ut, p2->vel);

            float n1_a = (n1 * (p1->mass - p2->mass) + 2.0f * p2->mass * n2) / (p1->mass + p2->mass);
            float n2_a = (n2 * (p2->mass - p1->mass) + 2.0f * p1->mass * n1) / (p1->mass + p2->mass);

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
        else if (p1 != NULL) {
            // TODO: collision with border
            // printf("collision :D\n");
        }

        // keep track of the simulated time
        sim_t += min_dt;
        min_dt = delta_t - sim_t;
    }
}

void on_create() {
    part_buf = malloc(sizeof(Particle) * PARTICLE_COUNT);

    part_tree = quad_tree_create((Area){(Vector){0, 0}, (Vector){get_width(), get_height()}}, 0);
    part_area_buf = malloc(sizeof(Particle *) * PARTICLE_COUNT);

    part_buf[0] = (Particle){3.0, 4.0, (Vector){50, 70}, (Vector){20.0, 0.0}};
    part_buf[1] = (Particle){1.0, 4.0, (Vector){80, 68}, (Vector){0.0, 0.0}};

    // static borders
    border_count = 1;

    border_buf = malloc(sizeof(Border) * border_count);
    border_buf[0] = border_create((Vector){25, 25}, (Vector){75, 75}, 2.0);

    border_tree = quad_tree_create((Area){(Vector){0.0, 0.0}, (Vector){get_width(), get_height()}}, 0);
    border_area_buf = malloc(sizeof(Border *) * border_count);

    quad_tree_clear(border_tree);
    for (int i = 0; i < border_count; i++)
        quad_tree_add_element(border_tree, &border_buf[i], border_buf[i].area);
}

void on_update() {
    clear();
    // evaluateParticles(delta_time);
    evaluate(delta_time);
    
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        draw_circle(part_buf[i].pos.x, part_buf[i].pos.y, part_buf[i].r);

        // Area part_area = particle_area(&part_buf[i], delta_time);
        // draw_rect(part_area.pos.x, part_area.pos.y, part_area.size.x, part_area.size.y);
    }

    for (int i = 0; i < border_count; i++) {
        float dx = border_buf[i].r * sin(M_PI / 2 - border_buf[i].a);
        float dy = border_buf[i].r * sin(border_buf[i].r);

        draw_line(border_buf[i].area.pos.x - dx, border_buf[i].area.pos.y + dy, border_buf[i].area.pos.x + border_buf[i].area.size.x - dx, border_buf[i].area.pos.y + border_buf[i].area.size.y + dy);
        draw_line(border_buf[i].area.pos.x + dx, border_buf[i].area.pos.y - dy, border_buf[i].area.pos.x + border_buf[i].area.size.x + dx, border_buf[i].area.pos.y + border_buf[i].area.size.y - dy);

        draw_circle(border_buf[i].area.pos.x, border_buf[i].area.pos.y, border_buf[i].r);
        draw_circle(border_buf[i].area.pos.x + border_buf[i].area.size.x, border_buf[i].area.pos.y + border_buf[i].area.size.y, border_buf[i].r);

        draw_rect(border_buf[i].area.pos.x, border_buf[i].area.pos.y, border_buf[i].area.size.x, border_buf[i].area.size.y);
    }
}

void key_cb(int key, int action, int flags) {
    if (key == KEY_SPACE && action == KEY_PRESS)
        evaluateParticles(1.0);
}

int main() {
    srand(time(NULL));

    engine_create(500, 500, 5, 5);
    // engine_set_user_input(key_cb, NULL);
    scene_create(on_create, on_update, NULL);
    clear_color(COLOR_VERY_DARK_GREY);
    engine_start();
}