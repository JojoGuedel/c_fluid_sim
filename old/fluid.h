#ifndef FLUID_H
#define FLUID_H

typedef struct
{
    int vel_x;
    int vel_y;

    float density;
} FluidCell;

// void fluid_add_source(FluidCell *fluid_target, int width, int x, int y, float source);

// void fluid_set_boundaries(FluidCell *fluid_target, int width, int height);

void fluid_diffuse_bad(FluidCell *fluid_target, FluidCell *fluid_src, int width, int height, float diff_rate);
void fluid_diffuse(FluidCell *fluid_target, FluidCell *fluid_src, int width, int height, float diff_rate, int percision);

#endif