#ifndef PIXA_INTERNALS_H
#define PIXA_INTERNALS_H

#include <stddef.h>

#include "GLFW/glfw3.h"
#include "Pixa/color.h"
#include "Pixa/layer.h"
#include "Pixa/scene.h"

extern Scene *scenes;
extern size_t scene_c;

extern Color color_target;

extern Layer *layer_default;
extern Layer *layer_target;
extern Layer *layer_draw_stack;
extern size_t layer_draw_stack_count;

extern void (*on_key_callback)(int key, int action, int flags);
extern void (*on_mouse_callback)(int button, int action, int flags);

extern GLFWwindow *window;

#endif