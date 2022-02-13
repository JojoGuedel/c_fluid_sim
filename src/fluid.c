#include "Pixa/core.h"

#include "fluid.h"

void fluid_add_source(FluidCell *fluid_target, int width, int x, int y, float source)
{
    fluid_target[x + y * width].density += source * delta_time;
}

void fluid_set_boundaries(FluidCell *fluid_target, int width, int height)
{
    for (int x = 0; x < width; x++)
    {
        fluid_target[x +                    0] = fluid_target[x +                width];
        fluid_target[x + (height - 1) * width] = fluid_target[x + (height - 2) * width];
        // TODO: isn't there a boundary missing?
    }

    for (int y = 0; y < height; y++)
    {
        fluid_target[0         + y * width] = fluid_target[1         + y * width];
        fluid_target[width - 1 + y * width] = fluid_target[width - 2 + y * width];
    }
}

void fluid_diffuse_bad(FluidCell *fluid_target, FluidCell *fluid_src, int width, int height, float diff_rate)
{
    float diff_rate_dt = diff_rate * delta_time /** width * height*/;

    for (int y = 1; y < height - 1; y++)
        for (int x = 1; x < width - 1; x++)
        {
            fluid_target[x + y * width].density = fluid_src[(x    ) + (y    ) * width].density + diff_rate_dt * (
                                                  fluid_src[(x + 1) + (y    ) * width].density +
                                                  fluid_src[(x - 1) + (y    ) * width].density +
                                                  fluid_src[(x    ) + (y + 1) * width].density +
                                                  fluid_src[(x    ) + (y - 1) * width].density - 4.0f * 
                                                  fluid_src[(x    ) + (y    ) * width].density);
        }
    
    fluid_set_boundaries(fluid_target, width, height);
}

void fluid_diffuse(FluidCell *fluid_target, FluidCell *fluid_src, int width, int height, float diff_rate, int percision)
{
    float diff_rate_dt = diff_rate * delta_time /** width * height*/;

    for (int p = 0; p < percision; p++)
        for (int y = 1; y < height - 1; y++)
            for (int x = 1; x < width - 1; x++)
            {
                fluid_target[x + y * width].density = (  fluid_src[(x    ) + (y    ) * width].density + diff_rate_dt * (
                                                      fluid_target[(x + 1) + (y    ) * width].density +
                                                      fluid_target[(x - 1) + (y    ) * width].density +
                                                      fluid_target[(x    ) + (y + 1) * width].density +
                                                      fluid_target[(x    ) + (y - 1) * width].density)) / (1 + 4.0f * diff_rate_dt);
            }
    
    fluid_set_boundaries(fluid_target, width, height);
}