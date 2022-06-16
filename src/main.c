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

enum {
    COLLISION_NONE = 0,
    COLLISION_PART_PART = 1,
    COLLISION_COMP_ELAS = 2,
};

float check_collision_part_part(Vector p1, Vector v1, float r1, Vector p2, Vector v2, float r2) {
    // (x(vb) - x(va))² + (y(vb) - y(va))²
    float a = (v2.x - v1.x) * (v2.x - v1.x)
            + (v2.y - v1.y) * (v2.y - v1.y);
    // ((x(B) - x(A)) (x(vb) - x(va)) + (y(B) - y(A)) (y(vb) - y(va))) * 2
    float b = 2 * ((p2.x - p1.x) * (v2.x - v1.x)
                 + (p2.y - p1.y) * (v2.y - v1.y));
    // (x(B) - x(A))² + (y(B) - y(A))² - (ra + rb)²
    float c = (p2.x - p1.x) * (p2.x - p1.x)
            + (p2.y - p1.y) * (p2.y - p1.y)
            - (r1 + r2) * (r1 + r2);
    
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

float check_collision_part_bord(Particle* p, Border* b, float* ret_ang) {
    // (+-(r_c + r_l) / sin(π / 2 - a) + m - y(P) + x(P) s) / (y(v) - x(v) s)
    float min_dist = (p->r + b->r);
    float tc1 = ( min_dist / sin(M_PI / 2.0f - b->s.alpha) + b->s.b - p->pos.y + p->pos.x * b->s.a) / (p->vel.y - p->vel.x * b->s.a);
    float tc2 = (-min_dist / sin(M_PI / 2.0f - b->s.alpha) + b->s.b - p->pos.y + p->pos.x * b->s.a) / (p->vel.y - p->vel.x * b->s.a);

    // determine the nearest collision in the future
    float tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;

    // find out if the particle is going to miss the section
    Vector p_cp = (Vector){p->pos.x + p->vel.x * tc, p->pos.y + p->vel.y * tc};
    Vector np_d = (Vector) {
        // dx = (y(P') - s x(P') - m) sin(a) sin(π / 2 - a)
        (p_cp.y - b->s.a * p_cp.x - b->s.b) * sin(b->s.alpha) * sin(M_PI / 2 - b->s.alpha),
        // dy = (y(P') - s x(P') - m) sin(a) sin(a) - (y(P') - s x(P') - m)
        (p_cp.y - b->s.a * p_cp.x - b->s.b) * sin(b->s.alpha) * sin(b->s.alpha) - (p_cp.y - b->s.a * p_cp.x - b->s.b)};

    Vector b_cp = vector_add(p_cp, np_d);
    // inv_lerp to know if the circle hits the border outside of pos1 and pos2 or inside
    float lerp = straight_inv_lerp(b->s, b->pos1, b->pos2, b_cp);

    color(COLOR_DARK_CYAN);
    draw_circle(p_cp.x, p_cp.y, p->r);
    draw_circle(b_cp.x, b_cp.y, b->r);
    color(COLOR_WHITE);

    // return tc and set the angle if it hits the border inside pos1 and pos2
    if (lerp <= 1.0f && lerp >= 0.0f) {
        *ret_ang = b->s.alpha;
        return tc;
    }

    // clamping the values between 0 and 1
    lerp = min(max(lerp, 0.0f), 1.0f);

    // set the new clamped collision point
    b_cp = straight_lerp(b->s, b->pos1, b->pos2, lerp);

    // calculate the new time
    tc = check_collision_part_part(p->pos, p->vel, p->r, b_cp, VECTOR_ZERO, b->r);
    // no chekcs needet, because it's guaranteed at this point, that the particle will hit the line
    p_cp = (Vector){p->pos.x + p->vel.x * tc, p->pos.y + p->vel.y * tc};

    color(COLOR_DARK_RED);
    draw_circle(p_cp.x, p_cp.y, p->r);
    draw_circle(b_cp.x, b_cp.y, b->r);
    color(COLOR_WHITE);
    
    // draw_line(s_cp.x, s_cp.y, s_cp.x + direction.x, s_cp.y + direction.y);
    *ret_ang = atan2(p_cp.y - b_cp.y, p_cp.x - b_cp.x) + M_PI / 2.0f;
    return tc;
}

void simulate_velocity(Particle* p, float dt) {
    p->pos.x += p->vel.x * dt;
    p->pos.y += p->vel.y * dt;

    if (p->pos.x > get_width())
        p->pos.x = 0;
    if (p->pos.x < 0)
        p->pos.x = get_width();
    
    if (p->pos.y > get_height())
        p->pos.y = 0;
    if (p->pos.y < 0)
        p->pos.y = get_height();
}

void simulate_collision_part_part(Particle* p1, Particle* p2) {
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
}

void simulate_collision_comp_elas(Particle* p, Straight s) {
    p->vel = vector_mirror(p->vel, s);
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

            float  direction = 0.0f;
            
            // check particle-border collisions
            int border_area_count = 0; 
            quad_tree_get_elements(border_tree, part_area, (void **)border_area_buf, &border_area_count);


            for (int j = 0; j < border_area_count; j++) {
                float temp_d;
                float tc = check_collision_part_bord(part, border_area_buf[j], &temp_d);

                if (tc < TOLERANCE || tc > min_dt)
                    continue;
                
                direction = temp_d;
                min_dt = tc;
                co1 = part;
                co2 = &direction;
                ct = COLLISION_COMP_ELAS;
            }

            // check particle-particle collisions
            int part_area_count = 0; 
            quad_tree_get_elements(part_tree, part_area, (void **)part_area_buf, &part_area_count);

            // find the next collision with another particle
            if (part_area_count > 1) {
                for (int j = 0; j < part_area_count; j++) {
                    float tc = check_collision_part_part(part->pos, part->vel, part->r, part_area_buf[j]->pos, part_area_buf[j]->vel, part_area_buf[j]->r);

                    // DEBUG STUFF
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
        for (int i = 0; i < PARTICLE_COUNT; i++)
            simulate_velocity(&part_buf[i], min_dt);

        // calculate the velocity after the next collision
        switch (ct) {
            case COLLISION_PART_PART:
                simulate_collision_part_part((Particle*)co1, (Particle*)co2);
                break;

            case COLLISION_COMP_ELAS:
                simulate_collision_comp_elas((Particle*) co1, straight_create_alphab(*(float*)co2, 0.0f));
                break;

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

    part_tree = quad_tree_create((Area){VECTOR_ZERO, (Vector){get_width(), get_height()}}, 0);
    part_area_buf = malloc(sizeof(Particle *) * PARTICLE_COUNT);

    part_buf[0] = (Particle){3.0, 4.0, (Vector){50, 20}, (Vector){0.0, 20.0}};
    // part_buf[1] = (Particle){1.0, 4.0, (Vector){80, 20}, (Vector){4.0, 10.0}};

    // static borders
    border_count = 2;

    border_buf = malloc(sizeof(Border) * border_count);
    border_buf[0] = border_create((Vector){20, 70}, (Vector){45, 75}, 5.0);
    border_buf[1] = border_create((Vector){20, 10}, (Vector){10, 70}, 5.0);

    border_tree = quad_tree_create((Area){VECTOR_ZERO, (Vector){get_width(), get_height()}}, 0);
    border_area_buf = malloc(sizeof(Border *) * border_count);

    quad_tree_clear(border_tree);
    for (int i = 0; i < border_count; i++)
        quad_tree_add_element(border_tree, &border_buf[i], border_buf[i].area);
}

void on_update() {
    clear();
    // evaluateParticles(delta_time);
    evaluate(delta_time);

    // printf("%f\n", straight_inv_lerp(border_buf[0].s, (Vector){mouse_x / 5.0, mouse_y / 5.0}, border_buf[0].pos1, border_buf[0].pos2));
    
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        draw_circle(part_buf[i].pos.x, part_buf[i].pos.y, part_buf[i].r);

        // Area part_area = particle_area(&part_buf[i], delta_time);
        // draw_rect(part_area.pos.x, part_area.pos.y, part_area.size.x, part_area.size.y);
    }

    for (int i = 0; i < border_count; i++) {
        float dy = border_buf[i].r * sin(M_PI / 2.0 - border_buf[i].s.alpha);
        float dx = border_buf[i].r * sin(border_buf[i].s.alpha);

        draw_line(border_buf[i].pos1.x - dx, border_buf[i].pos1.y + dy, border_buf[i].pos2.x - dx, border_buf[i].pos2.y + dy);
        draw_line(border_buf[i].pos1.x + dx, border_buf[i].pos1.y - dy, border_buf[i].pos2.x + dx, border_buf[i].pos2.y - dy);

        // draw_circle(jborder_buf[i].pos1.x, border_buf[i].pos1.y, border_buf[i].r);
        // draw_circle(border_buf[i].pos2.x, border_buf[i].pos2.y, border_buf[i].r);
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