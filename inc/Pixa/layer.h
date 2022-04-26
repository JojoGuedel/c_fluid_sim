#ifndef PIXA_LAYER_H
#define PIXA_LAYER_H

#include "Pixa/color.h"
#include "Pixa/sprite.h"

#define LAYER_DEFAULT NULL

typedef struct {
    int layer_level;

    Color *clear_color;
    Sprite *draw_target;
} Layer;

Layer *layer_create(int layer_level, Sprite *draw_target);
bool layer_destroy(Layer *layer);

void layer_bind(Layer *layer);

#endif