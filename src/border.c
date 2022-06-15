#include <math.h>
#include <stdlib.h>

#include "border.h"
#include "Pixa/graphics.h"

Border border_create(Vector pos1, Vector pos2, float thickness) {
    // slope of the border
    float s = (pos2.y - pos1.y) / (pos2.x - pos1.x);
    // angle to the x-axis
    float a = atan2(pos2.y - pos1.y, pos2.x - pos1.x);
    // y-axis offset
    float m = pos1.y - s * pos1.x;
    
    // sort the positions so that the physics-engine doen't have to do unnecessary checks
    if (s == INFINITY) {
        if (pos2.y < pos1.y)
            SWAP(pos1, pos2)
    } else {
        if (pos2.x < pos1.x)
            SWAP(pos1, pos2)
    }

    // pre-compute the area for the quad-tree
    Area area = (Area) {
        (Vector){min(pos1.x, pos2.x) - thickness, min(pos1.y, pos2.y) - thickness},
        (Vector){fabs(pos2.x - pos1.x) + 2 * thickness, fabs(pos2.y - pos1.y) + 2 * thickness}
    };

    return (Border){pos1, pos2, area, thickness, a, s, m};
}