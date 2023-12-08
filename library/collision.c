#include "collision.h"
#include "list.h"
#include "stdlib.h"
#include "vector.h"
#include <assert.h>
#include <stdbool.h>
#include <math.h>

list_t *get_normals(list_t *shape) {
  list_t *normals = list_init(list_size(shape), (free_func_t)free);
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t *v1 = list_get(shape, i);
    size_t next_point = i + 1;
    if (i == list_size(shape) - 1) {
      next_point = 0;
    }
    vector_t *v2 = list_get(shape, next_point);
    vector_t side = vec_subtract(*v2, *v1);
    vector_t *normal = malloc(sizeof(vector_t));
    assert(normal != NULL);
    normal->x = side.y;
    normal->y = -side.x;
    *normal = vec_multiply(1.0 / sqrt(vec_dot(*normal, *normal)), *normal);
    list_add(normals, normal);
  }
  return normals;
}

double get_overlap(list_t *shape1, list_t *shape2, vector_t axis) {
  double min1 = vec_dot(axis, *((vector_t *)list_get(shape1, 0)));
  double max1 = min1;
  for (size_t i = 1; i < list_size(shape1); i++) {
    double val = vec_dot(axis, *((vector_t *)list_get(shape1, i)));
    if (val > max1) {
      max1 = val;
    }
    if (val < min1) {
      min1 = val;
    }
  }
  double min2 = vec_dot(axis, *((vector_t *)list_get(shape2, 0)));
  double max2 = min2;
  for (size_t i = 1; i < list_size(shape2); i++) {
    double val = vec_dot(axis, *((vector_t *)list_get(shape2, i)));
    if (val > max2) {
      max2 = val;
    }
    if (val < min2) {
      min2 = val;
    }
  }
  if (max1 < min2 || max2 < min1) {
    return 0.0;
  } else {
    return fabs(fmax(min1, min2) - fmin(max1, max2));
  }
}

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  collision_info_t collision_info;
  list_t *axes1 = get_normals(shape1);
  double min_overlap = INFINITY;
  vector_t min_axis = *(vector_t *)list_get(axes1, 0);
  for (size_t i = 0; i < list_size(axes1); i++) {
    vector_t axis = *((vector_t *)list_get(axes1, i));
    double overlap = get_overlap(shape1, shape2, axis);
    if (overlap == 0.0) {
      list_free(axes1);
      collision_info.collided = false;
      return collision_info;
    } else if (overlap < min_overlap) {
      min_overlap = overlap;
      min_axis = axis;
    }
  }
  list_free(axes1);
  list_t *axes2 = get_normals(shape2);
  for (size_t i = 0; i < list_size(axes2); i++) {
    vector_t axis = *((vector_t *)list_get(axes2, i));
    double overlap = get_overlap(shape1, shape2, axis);
    if (overlap == 0.0) {
      list_free(axes2);
      collision_info.collided = false;
      return collision_info;
    } else if (overlap < min_overlap) {
      min_overlap = overlap;
      min_axis = vec_negate(axis);
    }
  }
  list_free(axes2);
  collision_info.collided = true;
  collision_info.axis = min_axis;
  collision_info.overlap = min_overlap;
  return collision_info;
}

void collision_aux_free(collision_aux_t *collision_aux){
   if(collision_aux->freer != NULL){
    collision_aux->freer(collision_aux->aux);
   }
   free(collision_aux);
 }
