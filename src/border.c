#include <math.h>
#include <stdlib.h>

#include "border.h"
#include "utils.h"
#include "Pixa/graphics.h"

Border border_create(Vector p1, Vector p2, float thickness) {
    Straight s = straight_create_p(p1, p2);

    // sort the positions so that the physics-engine doen't have to do unnecessary checks
    if (s.a == INFINITY) {
        if (p2.y < p1.y)
            SWAP(p1, p2)
    } else {
        if (p2.x < p1.x)
            SWAP(p1, p2)
    }

    // pre-compute the area for the quad-tree
    Area area = (Area) {
        (Vector){min(p1.x, p2.x) - thickness, min(p1.y, p2.y) - thickness},
        (Vector){fabs(p2.x - p1.x) + 2 * thickness, fabs(p2.y - p1.y) + 2 * thickness}
    };

    return (Border){p1, p2, thickness, area, s};
}