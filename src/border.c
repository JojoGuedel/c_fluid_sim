#include <math.h>
#include <stdlib.h>

#include "border.h"
#include "Pixa/graphics.h"

Border border_create(Vector pos1, Vector pos2, float thickness) {
    if (pos2.x < pos1.x)
        SWAP(pos1, pos2)

    Area area = (Area) {
        (Vector){min(pos1.x, pos2.x) - thickness, min(pos1.y, pos2.y) - thickness},
        (Vector){fabs(pos2.x - pos1.x) + 2 * thickness, fabs(pos2.y - pos1.y) + 2 * thickness}
    };

    float a = atan2(pos2.y - pos1.y, pos2.x - pos1.x);
    float s = (pos2.y - pos1.y) / (pos2.x - pos1.x);
    float m = pos1.y - s * pos1.x;
    
    return (Border){pos1, pos2, area, thickness, a, s, m};
}