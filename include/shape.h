#ifndef __SHAPE_H__
#define __SHAPE_H__

#include "list.h"
#include "vector.h"

list_t *make_circle(vector_t center, double radius, size_t num_points);

list_t *make_rect(vector_t bottom_left, double len, double height);

#endif 
