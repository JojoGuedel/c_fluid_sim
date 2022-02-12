
#include "Pixa/core.h"
#include "Pixa/graphics.h"
#include "Pixa/scene.h"
#include "Pixa/log.h"


typedef struct
{
    int vel_x;
    int vel_y;

    float density;
} FluidCell;

void fluid_add_source(FluidCell *fluid_target, int width, int x, int y, float source)
{
    fluid_target[x + y * width].density += source * delta_time;
}

void fluid_set_boundaries(FluidCell *fluid_target, int width, int height)
{
    for(int x = 0; x < width; x++)
    {
        fluid_target[x + 0] = fluid_target[x + width];
        // TODO: isn't there a boundary missing?
    }

    for(int y = 0; y < height; y++)
    {
        fluid_target[0         + y * width] = fluid_target[1         + y * width];
        fluid_target[width - 1 + y * width] = fluid_target[width - 2 + y * width];
    }
}

void fluid_diffuse_bad(FluidCell *fluid_target, FluidCell *fluid_src, int width, int height, float diff_rate)
{
    float diff_rate_dt = diff_rate * delta_time /** width * height*/;

    for(int y = 1; y < height - 1; y++)
        for(int x = 1; x < width - 1; x++)
        {
            fluid_target[x + y * width].density = fluid_src[(x    ) + (y    ) * width].density + diff_rate_dt * (
                                                  fluid_src[(x + 1) + (y    ) * width].density +
                                                  fluid_src[(x - 1) + (y    ) * width].density +
                                                  fluid_src[(x    ) + (y + 1) * width].density +
                                                  fluid_src[(x    ) + (y - 1) * width].density - 4.0f * 
                                                  fluid_src[(x    ) + (y    ) * width].density);
            // log_info("%f, %f", diff_rate_dt, fluid_target[x + y * width].density);
        }
    
    // fluid_set_boundaries(fluid_target, width, height);
}

#define WIDTH 125
#define HEIGHT 125

FluidCell fluid_field_b1[(WIDTH + 2) * (HEIGHT + 2)];
FluidCell fluid_field_b2[(WIDTH + 2) * (HEIGHT + 2)];

FluidCell *current = fluid_field_b1;
FluidCell *back    = fluid_field_b2;

void on_create()
{
    for(int i = 0; i < (WIDTH + 2) * (HEIGHT + 2); i++)
        current[i] = (FluidCell) {0, 0, 255};
}

void on_update()
{
    fluid_diffuse_bad(back, current, (WIDTH + 2), (HEIGHT + 2), 10.0f);

    // "swap the buffers"
    FluidCell *temp = current;
    current = back;
    back = temp;

    for(int y = 0; y < HEIGHT; y++)
        for(int x = 0; x < WIDTH; x++)
        {
            color((Color){x, y, current[(x + 1) + (y + 1) * (WIDTH + 2)].density, 255});
            draw_pixel(x, y);
            // log_info("%i, %i: %f", x, y, current[(x + 1) + (y + 1) * (WIDTH + 2)].density);
        }
}

int main()
{
    engine_create(500, 500, 500 / WIDTH, 500 / HEIGHT);
    scene_create(on_create, on_update, NULL);
    clear_color(COLOR_VERY_DARK_GREY);
    engine_start();
}