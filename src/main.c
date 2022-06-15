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

enum {
    COLLISION_NONE = 0,
    COLLISION_PART_PART = 1,
    COLLISION_COMP_ELAS = 2,
};

typedef enum {
    COLL_CASE_NOR = 0,
    COLL_CASE_LOW = 1,
    COLL_CASE_HIG = 2,
    COLL_CASE_FRO = 3,
} CollisionType;

float check_collision_part_part(Particle* p1, Particle* p2) {
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

float check_collision_part_bord(Particle* p, Border* b) {
    // (+-(r_c + r_l) / sin(π / 2 - a) + m - y(P) + x(P) s) / (y(v) - x(v) s)
    float min_dist = (p->r + b->r);
    float tc1 = ( min_dist / sin(M_PI / 2.0f - b->s.alpha) + b->s.b - p->pos.y + p->pos.x * b->s.a) / (p->vel.y - p->vel.x * b->s.a);
    float tc2 = (-min_dist / sin(M_PI / 2.0f - b->s.alpha) + b->s.b - p->pos.y + p->pos.x * b->s.a) / (p->vel.y - p->vel.x * b->s.a);

    // determine the nearest collision in the future
    float tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;

    // find out if the particle is going to miss the section
    // dx = y(P) - a_s x(P) - m sin(a_w) sin(π / 2 - a_w)
    float np_x = (p->pos.y - p->pos.x * b->s.a - b->s.b) * sin(b->s.alpha) * sin(M_PI / 2.0f - b->s.alpha);
    // dy = (y(P) - a_s x(P) - m) (sin(a_w) sin(a_w) - 1)
    float np_y = (p->pos.y - p->pos.x * b->s.a) * (sin(b->s.alpha) * sin(b->s.alpha) - 1);

    Vector nadir_point = (Vector) {np_x, np_y};
    float lerp_val = straight_inv_lerp(b->s, nadir_point, b->pos1, b->pos2);
    printf("%f", lerp_val);
    lerp_val = min(max(lerp_val, 0.0f), 1.0f);
    printf("%f", lerp_val);

    Vector p_cp = (Vector){p->pos.x + p->vel.x * tc, p->pos.y + p->vel.y * tc};
    Vector s_cp = straight_lerp_v(b->s, b->pos1, b->pos2, lerp_val);
    
    tc = check_collision_part_part(p, &(Particle){0.0f, b->r, s_cp, (Vector){0.0f, 0.0f}});
    return tc;

    // TODO: Somehow return the rotation vector
    
    Vector direction = vector_rotate(vector_sub(s_cp, p_cp), M_PI / 2.0f);

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

void simulate_collision_comp_elas(Particle* p, Vector* direction) {
    p->vel = vector_mirror(p->vel, *direction);
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

            // for (int j = 0; j < border_area_count; j++) {
            //     Border* border = border_area_buf[j];
                
            //     float min_dist = (part_buf[i].r + border_area_buf[j]->r);
            //     float tc1, tc2, tc;

            //     int cc = COLL_CASE_NOR;

            //     if (border->s == INFINITY && part->vel.x) {
            //         // (x(A)- x(P) +- (r_c + r_l)) / x(v)
            //         tc1 = (border->pos1.x - part->pos.x + min_dist) / part->vel.x;
            //         tc2 = (border->pos1.x - part->pos.x - min_dist) / part->vel.x;

            //         // determine the nearest collision in the future
            //         tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;

            //         // find out if the particle is going to miss the line segment
            //         // this is why the positions of the borders have to be sorted
            //         float dy = part->vel.y * tc;
            //         cc = part->pos.y + dy < border->pos1.y? COLL_CASE_LOW : cc;
            //         cc = part->pos.y + dy > border->pos2.y? COLL_CASE_HIG : cc;
            //     } else if (border->s == INFINITY && !part->vel.x) {
            //         // skip this check if there can't be a collision
            //         if (part->pos.x + part->r < border->pos1.x - border->r || part->pos.x - part->r > border->pos1.x + border->r)
            //             continue;
            //         else {
            //             tc1 = check_collision_part_part(part, &(Particle){0, border->r, border->pos1, (Vector){0.0, 0.0}});
            //             tc2 = check_collision_part_part(part, &(Particle){0, border->r, border->pos2, (Vector){0.0, 0.0}});

            //             // find out if the particle colides frontal with the line segment
            //             tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;

            //             if (tc > min_dt || tc < TOLERANCE)
            //                 continue;

            //             cc = COLL_CASE_FRO;
            //         }
            //     } else {
            //         // (+-(r_c + r_l) / sin(π / 2 - a) + m - y(P) + x(P) s) / (y(v) - x(v) s)
            //         tc1 = ( min_dist / sin(M_PI / 2.0f - border->a) + border->b - part->pos.y + part->pos.x * border->s) / (part->vel.y - part->vel.x * border->s);
            //         tc2 = (-min_dist / sin(M_PI / 2.0f - border->a) + border->b - part->pos.y + part->pos.x * border->s) / (part->vel.y - part->vel.x * border->s);

            //         // determine the nearest collision in the future
            //         tc = (tc2 >= TOLERANCE && (tc2 < tc1 || tc1 <= TOLERANCE)) ? tc2 : tc1;

            //         // find out if the particle is going to miss the section
            //         // dx = y(P) - a_s x(P) - m sin(a_w) sin(π / 2 - a_w)
            //         float dx = (part->pos.y - part->pos.x * border->s - border->b) * sin(border->a) * sin(M_PI / 2.0f - border->a);
            //         cc = part->pos.x + dx < border->pos1.x? COLL_CASE_LOW : cc;
            //         cc = part->pos.x + dx > border->pos2.x? COLL_CASE_HIG : cc;
            //     }

            //     switch(cc) {
            //         // this is the default way of colliging with the line segment
            //         case COLL_CASE_NOR: {
            //             if (tc > min_dt || tc < TOLERANCE)
            //                 continue;

            //             Vector direction = vector_rotate(vector_sub((Vector){part->pos.x + part->vel.x * tc, part->pos.y + part->vel.y * tc}, border->pos1), M_PI / 2.0f);
            //             co2 = &direction;
            //             ct = COLLISION_COMP_ELAS;
            //         } break;

            //         // this is the way of colliging when the front segment is hit
            //         case COLL_CASE_LOW: {
            //             Particle bc = (Particle){0, border->r, border->pos1, (Vector){0.0f, 0.0f}};
            //             tc = check_collision_part_part(part, &bc);

            //             if (tc > min_dt || tc < TOLERANCE)
            //                 continue;
                        
            //             Vector direction = vector_rotate(vector_sub((Vector){part->pos.x + part->vel.x * tc, part->pos.y + part->vel.y * tc}, border->pos1), M_PI / 2.0f);
            //             co2 = &direction;
            //             ct = COLLISION_COMP_ELAS;
            //         } break;

            //         // this is the way of colliging when the back segment is hit
            //         case COLL_CASE_HIG: {
            //             Particle bc = (Particle){0, border->r, border->pos2, (Vector){0.0f, 0.0f}};
            //             tc = check_collision_part_part(part, &bc);

            //             if (tc > min_dt || tc < TOLERANCE)
            //                 continue;
                        
            //             Vector direction = vector_rotate(vector_sub((Vector){part->pos.x + part->vel.x * tc, part->pos.y + part->vel.y * tc}, border->pos2), M_PI / 2.0);
            //             co2 = &direction;
            //             ct = COLLISION_COMP_ELAS;
            //         } break;

            //         // this is a special case where the velodity is paralel to the line
            //         case COLL_CASE_FRO: {
            //             Vector direction = vector_rotate(vector_sub((Vector){part->pos.x + part->vel.x * tc, part->pos.y + part->vel.y * tc}, border->pos1), M_PI / 2.0f);
            //             co2 = &direction;
            //             ct = COLLISION_COMP_ELAS;
            //         } break;

            //         default: continue;
            //     }

            //     min_dt = tc;
            //     co1 = part;
            // }

            for (int j = 0; j < border_area_count; j++) {
                float tc = check_collision_part_bord(part, border_area_buf[j]);

                if (tc < TOLERANCE || tc > min_dt)
                    continue;
            }

            // check particle-particle collisions
            int part_area_count = 0; 
            quad_tree_get_elements(part_tree, part_area, (void **)part_area_buf, &part_area_count);

            // find the next collision with another particle
            if (part_area_count > 1) {
                for (int j = 0; j < part_area_count; j++) {
                    float tc = check_collision_part_part(part, part_area_buf[j]);

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
                simulate_collision_comp_elas((Particle*) co1, (Vector*) co2);
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