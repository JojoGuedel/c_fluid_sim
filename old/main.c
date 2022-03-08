#include <stddef.h>

#include "Pixa/core.h"
#include "Pixa/graphics.h"
#include "Pixa/input.h"
#include "Pixa/scene.h"
#include "Pixa/log.h"

#include "fluid.h"


#define min(a, b) a < b? a : b;

#define WIDTH 250
#define HEIGHT 250

bool m1_p = false;

FluidCell fluid_field_b1[(WIDTH + 2) * (HEIGHT + 2)];
FluidCell fluid_field_b2[(WIDTH + 2) * (HEIGHT + 2)];

FluidCell *front = fluid_field_b1;
FluidCell *back    = fluid_field_b2;

void on_create()
{
    for (int i = 0; i < (WIDTH + 2) * (HEIGHT + 2); i++)
        front[i] = (FluidCell) {0, 0, 0};
}

void on_update()
{
    if (m1_p)
    {
        int x = (int) mouse_x / (500 / WIDTH) + 1;
        int y = ((int) mouse_y / (500 / HEIGHT) + 1);

        if (x > 0 && x < WIDTH && y > 0 && y < WIDTH)
            front[x + y * (WIDTH + 2)].density += 50 * delta_time * WIDTH * HEIGHT;
    }

    // fluid_diffuse_bad(back, current, (WIDTH + 2), (HEIGHT + 2), 10.0f);
    fluid_diffuse(back, front, (WIDTH + 2), (HEIGHT + 2), 0.01f, 10);

    // "swap the buffers"
    FluidCell *temp = front;
    front = back;
    back = temp;

    for (int y = 0; y < HEIGHT + 1; y++)
        for (int x = 0; x < WIDTH + 1; x++)
        {
            float d = min(front[(x + 1) + (y + 1) * (WIDTH + 2)].density, 300);
            color(color_hsv(d, 100, 100));
            draw_pixel(x, y);
        }
    
    // log_info("%f", 1.0f / delta_time);
}

void mouse_cb(int key, int action, int flags)
{
    if (key == MOUSE_BUTTON_1 && action == BUTTON_PRESS)
        m1_p = true;
    else if (key == MOUSE_BUTTON_1 && action == BUTTON_RELEASE)
        m1_p = false;
}

int main()
{
    engine_create(500, 500, 500 / WIDTH, 500 / HEIGHT);
    engine_set_user_input(NULL, mouse_cb);
    scene_create(on_create, on_update, NULL);
    clear_color(COLOR_VERY_DARK_GREY);
    engine_start();
}