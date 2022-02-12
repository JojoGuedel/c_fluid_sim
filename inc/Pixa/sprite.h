#ifndef PIXA_SPRITE_H
#define PIXA_SPRITE_H

#include <stdbool.h>

#include "Pixa/color.h"

typedef struct
{
    unsigned int id;

    int width;
    int height;

    int x;
    int y;

    float scale_x;
    float scale_y;

    Color tint;
    Color *data;
} Sprite;

Sprite *sprite_create(int width, int height, bool filtered, bool clamp);
void sprite_destroy(Sprite *sprite);

Sprite *sprite_copy(Sprite *src);
void sprite_copy_data(Sprite *dst, Sprite *src);

void sprite_update(Sprite *sprite);
void sprite_draw(Sprite *sprite);
void sprite_clear(Sprite *sprite, Color color);

void sprite_draw_pixel(Sprite *sprite, int x, int y, Color color);
void sprite_draw_line(Sprite *sprite, int x1, int y1, int x2, int y2, Color color);

void sprite_draw_rect(Sprite *sprite, int x, int y, int width, int height, Color color);
void sprite_fill_rect(Sprite *sprite, int x, int y, int width, int height, Color color);

void sprite_set_pos(Sprite *sprite, int x, int y);
void sprite_set_scale(Sprite *sprite, float scale_x, float scale_y);

#endif