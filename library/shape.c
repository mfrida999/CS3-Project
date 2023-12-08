#include "list.h"
#include "vector.h"
#include "forces.h"
#include "collision.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

list_t *make_circle(vector_t center, double radius, size_t num_points) { //move into new file for making standard shapes? (circle, rectangle)
  list_t *circle = list_init(num_points, (free_func_t) free);
  double curr_angle = 0;
  double vert_angle = 2.0 * M_PI / num_points;
  for (size_t i = 0; i < num_points; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);
    point->x = cos(curr_angle) * radius + center.x;
    point->y = sin(curr_angle) * radius + center.y;
    list_add(circle, point);
    curr_angle += vert_angle;
  }
  return circle;
}

list_t *make_rect(vector_t bottom_left, double len, double height) { //move into new file for making standard shapes? (circle, rectangle)
  list_t *rect = list_init(4, (free_func_t) free);

  vector_t *point = malloc(sizeof(vector_t));
  assert(point != NULL);
  *point = bottom_left;
  list_add(rect, point);

  point = malloc(sizeof(vector_t));
  assert(point != NULL);
  bottom_left.x += len;
  *point = bottom_left;
  list_add(rect, point);

  point = malloc(sizeof(vector_t));
  assert(point != NULL);
  bottom_left.y += height;
  *point = bottom_left;
  list_add(rect, point);

  point = malloc(sizeof(vector_t));
  assert(point != NULL);
  bottom_left.x -= len;
  *point = bottom_left;
  list_add(rect, point);

  return rect;
}
