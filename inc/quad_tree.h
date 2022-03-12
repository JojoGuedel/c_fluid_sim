#ifndef H_QUAD_LAYER_TREE
#define H_QUAD_LAYER_TREE

#include "utils.h"

#ifndef QUAD_TREE_MAX_DEPTH
#define QUAD_TREE_MAX_DEPTH 6
#endif

typedef struct QuadTree {
    int layer;
    Area area;
    int element_count;
    void **elements;

    struct QuadTree *childs[4];
} QuadTree;

QuadTree *quad_tree_create(Area area, int layer);
void quad_tree_clear(QuadTree *tree);
bool quad_tree_add_element(QuadTree *tree, void *element, Area area);

void quad_tree_get_elements(QuadTree *tree, Area area, void **elements, int *element_count);

int quad_tree_get_size_in_area(QuadTree *tree, Area area);

#endif