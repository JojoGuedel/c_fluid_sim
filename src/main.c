#include <corecrt_math_defines.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define _USE_MATH_DEFINES
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

#define PARTICLE_COUNT 1
// this is needed because of floating point precision errors
#define TOLERANCE 0.000001f

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

/*
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
*/

enum {
    COLLISION_NONE = 0,
    COLLISION_PART_PART = 1,
    COLLISION_PART_BORD = 2,
    COLLISION_COMP_ELAS = 3,
};

enum {
    COLL_CASE_NOR = 0,
    COLL_CASE_LOW = 1,
    COLL_CASE_HIG = 2,
    COLL_CASE_FRO = 3,
};

float evaluate_next_part_col(Particle* p1, Particle* p2) {
    // (x(vb) - x(va))² + (y(vb) - y(va))²
    float a = (p2->vel.x - p1->vel.x) * (p2->vel.x - p1->vel.x)
            + (p2->vel.y - p1->vel.y) * (p2->vel.y - p1->vel.y);
    // ((x(B) - x(A)) (x(vb) - x(va)) + (y(B) - y(A)) (y(vb) - y(va))) * 2
    float b = 2 * ((p2->pos.x - p1->pos.x) * (p2->vel.x - p1->vel.x)
                    + (p2->pos.y - p1->pos.y) * (p2->vel.y - p1->vel.y));
    // (x(B) - x(A))² + (y(B) - y(A))² - (ra + rb)²
    float c = (p2->pos.x - p1->pos.x) * (p2->pos.x - p1->pos.x)
            + (p2->pos.y - p1->pos.y) * (p2->pos.y - p1->pos.y)
            - (p1->r + p2->r) * (p1->r + p2->r);
    
    // only real numbers are important
    float D = b * b - 4 * a * c;
    if (D < 0)
        return -1.0;

    // everybodies favorite, the midnight formula
    // (-b + sqrt(b² - 4a c)) / (2a)
    float tc1 = (-b + sqrt(D)) / (2 * a);
    // (-b - sqrt(b² - 4a c)) / (2a)
    float tc2 = (-b - sqrt(D)) / (2 * a);

    // determine the nearest collision in the future
    float tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;

    if (tc < TOLERANCE)
        return -1.0;
    
    return tc;
}

void evaluate(float delta_t) {
    // TODO: cache dt for every particle
    float sim_t = 0;
    float min_dt = delta_t;
    int sim_steps = 0;

    void *co1;
    void *co2;

    int ct;

    // simulate for the durration of delta_t  
    while (sim_t < delta_t) {
        co1 = NULL;
        co2 = NULL;

        ct = COLLISION_NONE;

        // TODO: just modify tree instead of rebuilding it every time
        quad_tree_clear(part_tree);
        for (int i = 0; i < PARTICLE_COUNT; i++)
            quad_tree_add_element(part_tree, &part_buf[i], particle_area(&part_buf[i], min_dt));

        // check collisions for all particles
        for (int i = 0; i < PARTICLE_COUNT; i++) {
            Area part_area = particle_area(&part_buf[i], min_dt);
            Particle* part = &part_buf[i];
            
            // check particle-border collisions
            int border_area_count = 0; 
            quad_tree_get_elements(border_tree, part_area, (void **)border_area_buf, &border_area_count);

            for (int j = 0; j < border_area_count; j++) {
                Border* border = border_area_buf[j];
                
                float min_dist = (part_buf[i].r + border_area_buf[j]->r);
                float tc1, tc2, tc;

                int cc = COLL_CASE_NOR;

                if (border->s == INFINITY && part->vel.x) {
                    // (x(A)- x(P) +- (r_c + r_l)) / x(v)
                    tc1 = (border->pos1.x - part->pos.x + min_dist) / part->vel.x;
                    tc2 = (border->pos1.x - part->pos.x - min_dist) / part->vel.x;

                    // determine the nearest collision in the future
                    tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;

                    // find out if the particle is going to miss the line segment
                    // this is why the positions of the borders have to be sorted
                    float dy = part->vel.y * tc;
                    cc = part->pos.y + dy < border->pos1.y? COLL_CASE_LOW : cc;
                    cc = part->pos.y + dy > border->pos2.y? COLL_CASE_HIG : cc;
                } else if (border->s == INFINITY && !part->vel.x) {
                    // skip this check if there can't be a collision
                    if (part->pos.x + part->r < border->pos1.x - border->r || part->pos.x - part->r > border->pos1.x + border->r)
                        continue;
                    else {
                        tc1 = evaluate_next_part_col(part, &(Particle){0, border->r, border->pos1, (Vector){0.0, 0.0}});
                        tc2 = evaluate_next_part_col(part, &(Particle){0, border->r, border->pos2, (Vector){0.0, 0.0}});

                        // find out if the particle colides frontal with the line segment
                        tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;

                        if (tc > min_dt || tc < TOLERANCE)
                            continue;

                        cc = COLL_CASE_FRO;
                    }
                } else {
                    // (+-(r_c + r_l) / sin(π / 2 - a) + m - y(P) + x(P) s) / (y(v) - x(v) s)
                    tc1 = ( min_dist / sin(M_PI / 2.0f - border->a) + border->m - part->pos.y + part->pos.x * border->s) / (part->vel.y - part->vel.x * border->s);
                    tc2 = (-min_dist / sin(M_PI / 2.0f - border->a) + border->m - part->pos.y + part->pos.x * border->s) / (part->vel.y - part->vel.x * border->s);

                    // determine the nearest collision in the future
                    tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;

                    // find out if the particle is going to miss the section
                    // dx = y(P) - a_s x(P) - m sin(a_w) sin(π / 2 - a_w)
                    float dx = (part->pos.y - part->pos.x * border->s - border->m) * sin(border->a) * sin(M_PI / 2.0f - border->a);
                    cc = part->pos.x + dx < border->pos1.x? COLL_CASE_LOW : cc;
                    cc = part->pos.x + dx > border->pos2.x? COLL_CASE_HIG : cc;
                }

                switch(cc) {
                    // this is the default way of colliging with the line segment
                    case COLL_CASE_NOR: {
                        if (tc > min_dt || tc < TOLERANCE)
                            continue;

                        ct = COLLISION_PART_BORD;
                    co2 = border;
                    } break;

                    // this is the way of colliging when the front segment is hit
                    case COLL_CASE_LOW: {
                        Particle bc = (Particle){0, border->r, border->pos1, (Vector){0.0f, 0.0f}};
                        tc = evaluate_next_part_col(part, &bc);

                        if (tc > min_dt || tc < TOLERANCE)
                            continue;
                        
                        Vector direction = vector_rotate(vector_sub((Vector){part->pos.x + part->vel.x * tc, part->pos.y + part->vel.y * tc}, border->pos1), M_PI / 2.0f);
                        co2 = &direction;
                        ct = COLLISION_COMP_ELAS;
                    } break;

                    // this is the way of colliging when the back segment is hit
                    case COLL_CASE_HIG: {
                        Particle bc = (Particle){0, border->r, border->pos2, (Vector){0.0f, 0.0f}};
                        tc = evaluate_next_part_col(part, &bc);

                        if (tc > min_dt || tc < TOLERANCE)
                            continue;
                        
                        Vector direction = vector_rotate(vector_sub((Vector){part->pos.x + part->vel.x * tc, part->pos.y + part->vel.y * tc}, border->pos2), M_PI / 2.0);
                        co2 = &direction;
                        ct = COLLISION_COMP_ELAS;
                    } break;

                    // this is a special case where the velodity is paralel to the line
                    case COLL_CASE_FRO: {
                        Vector direction = vector_rotate(vector_sub((Vector){part->pos.x + part->vel.x * tc, part->pos.y + part->vel.y * tc}, border->pos1), M_PI / 2.0f);
                        co2 = &direction;
                        ct = COLLISION_COMP_ELAS;
                    } break;

                    default: continue;
                }

                min_dt = tc;
                co1 = part;
            }

            // check particle-particle collisions
            int part_area_count = 0; 
            quad_tree_get_elements(part_tree, part_area, (void **)part_area_buf, &part_area_count);

            // find the next collision with another particle
            if (part_area_count > 1) {
                for (int j = 0; j < part_area_count; j++) {
                    float tc = evaluate_next_part_col(part, part_area_buf[j]);

                    // color(COLOR_DARK_CYAN);
                    // draw_circle(part->pos.x + part->vel.x * tc1, part->pos.y + part->vel.y * tc1, part->r);
                    // draw_circle(part_area_bu[j]->pos.x + part_area_bu[j]->vel.x * tc1, part_area_bu[j]->pos.y + part_area_bu[j]->vel.y * tc1, part_area_bu[j]->r);

                    // color(COLOR_DARK_RED);
                    // draw_circle(part->pos.x + part->vel.x * tc2, part->pos.y + part->vel.y * tc2, part->r);
                    // draw_circle(part_area_bu[j]->pos.x + part_area_bu[j]->vel.x * tc2, part_area_bu[j]->pos.y + part_area_bu[j]->vel.y * tc2, part_area_bu[j]->r);
                    // color(COLOR_WHITE);
                    
                    if (tc < TOLERANCE || tc > min_dt)
                        continue;

                    min_dt = tc;

                    co1 = part;
                    co2 = part_area_buf[j];
                    ct = COLLISION_PART_PART;
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

        switch (ct) {
            case COLLISION_PART_PART: {
                Particle* p1 = (Particle*) co1;
                Particle* p2 = (Particle*) co2;

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
            } break;

            case COLLISION_PART_BORD: {
                Particle* p = (Particle*)co1;
                Border* b = (Border*)co2;

                // check special cases
                if (b->s == 0.0) {
                    p->vel = (Vector){p->vel.x, -p->vel.y};
                    break;
                }

                // (x(v) - y(v) / s) sin²(a)
                float dx = (p->vel.x - p->vel.y / b->s) * sin(b->a) * sin(b->a);
                // (x(v) - y(v) / s) sin(a) sin(π / 2 - a)
                float dy = (p->vel.x - p->vel.y / b->s) * sin(b->a) * sin(M_PI / 2.0 - b->a);

                p->vel = (Vector){p->vel.x - 2 * dx, p->vel.y + 2 * dy};
            } break;

            case COLLISION_COMP_ELAS: {
                Particle* p = (Particle*) co1;
                Vector* vec = (Vector*) co2;

                p->vel = vector_mirror(p->vel, *vec);
            } break;

            case COLLISION_NONE:
            default:
                break;
        }

        // keep track of the sim_step count
        sim_steps += 1;

        // keep track of the simulated time
        sim_t += min_dt;
        min_dt = delta_t - sim_t;
    }
}

void on_create() {
    part_buf = malloc(sizeof(Particle) * PARTICLE_COUNT);

    part_tree = quad_tree_create((Area){(Vector){0, 0}, (Vector){get_width(), get_height()}}, 0);
    part_area_buf = malloc(sizeof(Particle *) * PARTICLE_COUNT);

    part_buf[0] = (Particle){3.0, 4.0, (Vector){50, 20}, (Vector){0.0, 20.0}};
    // part_buf[1] = (Particle){1.0, 4.0, (Vector){80, 20}, (Vector){4.0, 10.0}};

    // static borders
    border_count = 2;

    border_buf = malloc(sizeof(Border) * border_count);
    border_buf[0] = border_create((Vector){50, 50}, (Vector){50, 75}, 10.0);
    border_buf[1] = border_create((Vector){20, 10}, (Vector){10, 80}, 5.0);

    border_tree = quad_tree_create((Area){(Vector){0.0, 0.0}, (Vector){get_width(), get_height()}}, 0);
    border_area_buf = malloc(sizeof(Border *) * border_count);

    quad_tree_clear(border_tree);
    for (int i = 0; i < border_count; i++)
        quad_tree_add_element(border_tree, &border_buf[i], border_buf[i].area);
}

void on_update() {
    clear();
    // evaluateParticles(delta_time);
    evaluate(0.001/*delta_time*/);
    
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        draw_circle(part_buf[i].pos.x, part_buf[i].pos.y, part_buf[i].r);

        // Area part_area = particle_area(&part_buf[i], delta_time);
        // draw_rect(part_area.pos.x, part_area.pos.y, part_area.size.x, part_area.size.y);
    }

    for (int i = 0; i < border_count; i++) {
        float dy = border_buf[i].r * sin(M_PI / 2.0 - border_buf[i].a);
        float dx = border_buf[i].r * sin(border_buf[i].a);

        draw_line(border_buf[i].pos1.x - dx, border_buf[i].pos1.y + dy, border_buf[i].pos2.x - dx, border_buf[i].pos2.y + dy);
        draw_line(border_buf[i].pos1.x + dx, border_buf[i].pos1.y - dy, border_buf[i].pos2.x + dx, border_buf[i].pos2.y - dy);

        draw_circle(border_buf[i].pos1.x, border_buf[i].pos1.y, border_buf[i].r);
        draw_circle(border_buf[i].pos2.x, border_buf[i].pos2.y, border_buf[i].r);
        // draw_rect(border_buf[i].area.pos.x, border_buf[i].area.pos.y, border_buf[i].area.size.x, border_buf[i].area.size.y);
    }
}

// void key_cb(int key, int action, int flags) {
//     if (key == KEY_SPACE && action == KEY_PRESS)
//         evaluateParticles(1.0);
// }

int main() {
    srand(time(NULL));

    engine_create(500, 500, 5, 5);
    // engine_set_user_input(key_cb, NULL);
    scene_create(on_create, on_update, NULL);
    clear_color(COLOR_VERY_DARK_GREY);
    engine_start();
}