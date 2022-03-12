#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Pixa/color.h"
#include "Pixa/graphics.h"

#include "debug.h"
#include "particle.h"
#include "quad_tree.h"
#include "utils.h"


QuadTree *quad_tree_create(Area area, int layer) {
    QuadTree *tree = malloc(sizeof(QuadTree));

    tree->layer = layer;
    tree->area = area;
    tree->elements = NULL;

    if (layer + 1 < QUAD_TREE_MAX_DEPTH) {
        Vector size = (Vector){area.size.x / 2.0f, area.size.y / 2.0f};

        tree->childs[0] = quad_tree_create((Area){(Vector){area.pos.x, area.pos.y}, size}, layer + 1);
        tree->childs[1] = quad_tree_create((Area){(Vector){area.pos.x + size.x, area.pos.y}, size}, layer + 1);
        tree->childs[2] = quad_tree_create((Area){(Vector){area.pos.x, area.pos.y + size.y}, size}, layer + 1);
        tree->childs[3] = quad_tree_create((Area){(Vector){area.pos.x + size.x, area.pos.y + size.y}, size}, layer + 1);
    }

    return tree;
}

void quad_tree_clear(QuadTree *tree) {
    if (tree->element_count != 0) {
        free(tree->elements);
        tree->elements = NULL;
        tree->element_count = 0;
    }

    if (tree->layer + 1 < QUAD_TREE_MAX_DEPTH)
        for (int i = 0; i < 4; i++)
            quad_tree_clear(tree->childs[i]);
}


bool quad_tree_add_element(QuadTree *tree, void *element, Area area) {
    if (!area_contains(tree->area, area))
        return false;

    if (tree->layer + 1 < QUAD_TREE_MAX_DEPTH)
        for (int i = 0; i < 4; i++)
            if (area_contains(tree->childs[i]->area, area)) {
                quad_tree_add_element(tree->childs[i], element, area);
                return true;
            }

    tree->elements = realloc(tree->elements, sizeof(void *) * (tree->element_count + 1));
    tree->elements[tree->element_count++] = element;

    return true;
}

void quad_tree_get_all_elements(QuadTree *tree, void **elements, int *element_count) {
    // draw_rect(tree->area.pos.x, tree->area.pos.y, tree->area.size.x, tree->area.size.y);
    if (tree->element_count != 0)
        memcpy(&elements[*element_count], tree->elements, tree->element_count * sizeof(void *));
    
    *element_count += tree->element_count;

    if (tree->layer + 1 < QUAD_TREE_MAX_DEPTH)
        for (int i = 0; i < 4; i++)
            quad_tree_get_all_elements(tree->childs[i], elements, element_count);
}


void quad_tree_get_elements(QuadTree *tree, Area area, void **elements, int *element_count) {
    // draw_rect(tree->area.pos.x, tree->area.pos.y, tree->area.size.x, tree->area.size.y);

    if (!area_overlaps(tree->area, area))
        return;

    // if (elements == NULL) {
    //     *element_count = 0;
    //     elements = malloc(sizeof(void *) * quad_tree_get_size_in_area(tree, area));
    // }

    if (tree->element_count != 0)
        memcpy(&elements[*element_count], tree->elements, tree->element_count * sizeof(void *));
    
    *element_count += tree->element_count;

    if (tree->layer + 1 < QUAD_TREE_MAX_DEPTH)
        for(int i = 0; i < 4; i++) {
            if (area_contains(area, tree->childs[i]->area))
                quad_tree_get_all_elements(tree->childs[i], elements, element_count);
            
            else if (area_overlaps(tree->childs[i]->area, area))
                quad_tree_get_elements(tree->childs[i], area, elements, element_count);
        }
}

int quad_tree_get_size(QuadTree *tree) {
    int count = tree->element_count;

    if (tree->layer + 1 < QUAD_TREE_MAX_DEPTH)
        for (int i = 0; i < 4; i++)
            count += quad_tree_get_size(tree->childs[i]);

    return count;
}

int quad_tree_get_size_in_area(QuadTree *tree, Area area) {
    if (!area_overlaps(tree->area, area))
        return 0;
    
    int count = tree->element_count;

    if (tree->layer + 1 < QUAD_TREE_MAX_DEPTH) {
        for (int i = 0; i < 4; i++) {
            if (area_contains(area, tree->childs[i]->area))
                count += quad_tree_get_size(tree->childs[i]);

            else if (area_overlaps(tree->childs[i]->area, area))
                count += quad_tree_get_size_in_area(tree->childs[i], area);
        }
    }

    return count;
}