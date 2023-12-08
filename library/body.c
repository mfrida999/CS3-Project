#include "body.h"
#include "assert.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include <stdlib.h>

typedef struct body {
  list_t *shape;
  void *info;
  double mass;
  void *image;
  rgb_color_t color;
  vector_t centroid;
  vector_t velocity;
  vector_t net_force;
  vector_t net_impulse;
  double angle;
  free_func_t info_freer;
  bool is_removed;
  bool is_rect;
  double len;
  double height;
  bool is_circle;
  double radius;
} body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, NULL);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  assert(mass > 0);
  body_t *out = malloc(sizeof(body_t));
  assert(out != NULL);
  out->shape = shape;
  out->mass = mass;
  out->color = color;
  out->info = info;
  out->velocity = VEC_ZERO;
  out->net_force = VEC_ZERO;
  out->net_impulse = VEC_ZERO;
  out->angle = 0;
  out->centroid = polygon_centroid(shape);
  out->info_freer = info_freer;
  out->is_removed = false;
  out->image = NULL;
  out->is_rect = false;
  out->is_circle = false;
  return out;
}

void body_set_img(body_t *body, void *image) {
  body->image = image;
}

void *body_get_img(body_t *body) {
  return body->image;
}

bool is_rect(body_t *body) {
  return body->is_rect;
}

bool is_circle(body_t *body) {
  return body->is_circle;
}

void set_circle(body_t *body, double radius) {
  body->is_circle = true;
  body->radius = radius;
}

double body_get_radius(body_t *body) {
  return body->radius;
}

void set_rect(body_t *body, double len, double height) {
  body->is_rect = true;
  body->len = len;
  body->height = height;
}

double get_len(body_t *body) {
  return body->len;
}

double get_height(body_t *body) {
  return body->height;
}

void body_free(body_t *body) {
  if (body->info != NULL && body->info_freer != NULL) {
    body->info_freer(body->info);
  }
  // remove_img_from_body(body);
  list_free(body->shape);
  free(body);
}

list_t *body_get_shape(body_t *body) {
  list_t *out = list_init(list_size(body->shape), (free_func_t)free);
  for (size_t i = 0; i < list_size(body->shape); i++) {
    vector_t *original = list_get(body->shape, i);
    vector_t *copy = malloc(sizeof(vector_t));
    copy->x = original->x;
    copy->y = original->y;
    list_add(out, copy);
  }
  return out;
}

list_t *body_get_shape_shallow(body_t *body) { return body->shape; }

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_velocity(body_t *body) { return body->velocity; }

double body_get_mass(body_t *body) { return body->mass; }

rgb_color_t body_get_color(body_t *body) { return body->color; }

void *body_get_info(body_t *body) { return body->info; }

void body_set_info(body_t *body, void *info) {
  body->info = info;
}

void body_set_centroid(body_t *body, vector_t x) {
  for (size_t i = 0; i < list_size(body->shape); i++) {
    vector_t *point = list_get(body->shape, i);
    *point = vec_add(x, vec_subtract(*point, body->centroid));
  }
  body->centroid = x;
}

void body_set_polygon(body_t *body, list_t *shape) {
  list_free(body->shape);
  body->shape = shape;
}

void body_set_velocity(body_t *body, vector_t v) { body->velocity = v; }

void body_set_rotation(body_t *body, double angle) {
  polygon_rotate(body->shape, angle - body->angle, body->centroid);
  body->angle = angle;
}

void body_set_rotation_about_point(body_t *body, double angle, vector_t point) {
  polygon_rotate(body->shape, angle - body->angle, point);
  body->angle = angle;
  body->centroid = polygon_centroid(body->shape);
}

double body_get_rotation(body_t *body) {
  return body->angle;
}

void body_set_mass(body_t *body, double m) {
  body->mass = m;
}

void body_add_force(body_t *body, vector_t force) {
  body->net_force = vec_add(body->net_force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->net_impulse = vec_add(body->net_impulse, impulse);
}

void body_tick(body_t *body, double dt) {
  vector_t new_velocity = body->velocity;
  body->net_impulse = vec_multiply((1.0 / body->mass), body->net_impulse);
  body->net_force = vec_multiply((1.0 / body->mass), body->net_force);
  new_velocity = vec_add(new_velocity, body->net_impulse);
  new_velocity = vec_add(new_velocity, vec_multiply(dt, body->net_force));
  body_set_centroid(
      body,
      vec_add(body->centroid,
              vec_multiply(dt / 2.0, vec_add(body->velocity, new_velocity))));
  body->velocity = new_velocity;
  body->net_force = VEC_ZERO;
  body->net_impulse = VEC_ZERO;
}

void body_remove(body_t *body) { body->is_removed = true; }

bool body_is_removed(body_t *body) { return body->is_removed; }