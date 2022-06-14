#include "border.h"
#include "Pixa/graphics.h"
#include <math.h>
#include <stdlib.h>

Border border_create(Vector pos1, Vector pos2, float thickness) {
    Area area = (Area) {
        (Vector){min(pos1.x, pos2.x), min(pos1.y, pos2.y)},
        (Vector){fabs(pos2.x - pos1.x), fabs(pos2.y - pos1.y)}
    };

    float a = atan2(area.size.y, area.size.x);
    float s = area.size.x != 0? area.size.y / area.size.x : 0;
    float m = area.pos.y - s * area.pos.x;
    
    return (Border){area, thickness, a, s, m};
}