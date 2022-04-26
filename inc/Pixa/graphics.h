#ifndef PIXA_GRAPHICS_H
#define PIXA_GRAPHICS_H

#include "Pixa/color.h"

void clear_color(Color color);
void clear();

void color(Color color);

void draw_pixel(int x, int y);
void draw_line(int x1, int y1, int x2, int y2);

void draw_rect(int x, int y, int width, int height);
void fill_rect(int x, int y, int width, int height);

void draw_circle(int x, int y, int r);

#endif