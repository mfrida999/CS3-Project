#include "forces.h"
#include "list.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// window constants
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t WINDOW_CENTER = (vector_t){.x = 500, .y = 250};

// physics constants
const double K = 5;
const double DAMP = 1;

// color gradient / sigmoid constants
const double STEEPNESS = 2.0;
const double MIN_COL = 0.3; // >0.001
const double MAX_COL = 1.0; // <=1.0

// ball constants
const size_t NUM_POINTS = 20;
const double MASS = 5;
const double RADIUS = 8 + 1.0 / 3.0;
size_t NUM_BALLS = 60;
rgb_color_t ANCHOR_COLOR = {1, 1, 1};

const double TWOPI = 2.0 * M_PI;

typedef struct state {
  scene_t *scene;
} state_t;

body_t *circle_init(vector_t center, rgb_color_t color) {
  list_t *circle_points = list_init(NUM_POINTS, (free_func_t)free);
  double curr_angle = 0;
  double vert_angle = TWOPI / NUM_POINTS;
  for (size_t i = 0; i < NUM_POINTS; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);
    point->x = cos(curr_angle) * RADIUS + center.x;
    point->y = sin(curr_angle) * RADIUS + center.y;
    list_add(circle_points, point);
    curr_angle += vert_angle;
  }
  body_t *circle = body_init(circle_points, MASS, color);
  return circle;
}

body_t *anchor_init(vector_t center) {
  list_t *anchor_points = list_init(NUM_POINTS, (free_func_t)free);
  double curr_angle = 0;
  double vert_angle = TWOPI / NUM_POINTS;
  for (size_t i = 0; i < NUM_POINTS; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);
    point->x = cos(curr_angle) * RADIUS + center.x;
    point->y = sin(curr_angle) * RADIUS + center.y;
    list_add(anchor_points, point);
    curr_angle += vert_angle;
  }
  body_t *anchor = body_init(anchor_points, INFINITY, ANCHOR_COLOR);
  return anchor;
}

double compute_sigmoid_1(size_t i) {
  double k = (double)i / ((double)NUM_BALLS - 1.0);
  return (1.0 / (1.0 + pow((1.0 / (2 * k) - 1.0), STEEPNESS))) * WINDOW.y / 2.0;
}

double compute_sigmoid_2(size_t i) {
  double k = (double)i / ((double)NUM_BALLS - 1.0) - 0.5;
  return (1.0 / (1.0 + pow((1.0 / (2 * k) - 1.0), STEEPNESS))) * WINDOW.y /
             4.0 +
         WINDOW.y / 2.0;
}

void make_circles(scene_t *scene) {
  rgb_color_t color =
      (rgb_color_t){.r = MIN_COL - 0.001,
                    .g = MIN_COL + (MAX_COL - MIN_COL) / 2 - 0.001,
                    .b = MIN_COL + (MAX_COL - MIN_COL) / 2 - 0.001};
  for (size_t i = 0; i < NUM_BALLS / 6; i++) {
    color.g -= (MAX_COL - MIN_COL) / 2 / (NUM_BALLS / 6.0);
    color.b += (MAX_COL - MIN_COL) / 2 / (NUM_BALLS / 6.0);
    double x = RADIUS + (i * 2 * RADIUS);
    double y = compute_sigmoid_1(i);
    body_t *circle = circle_init((vector_t){.x = x, .y = y}, color);
    scene_add_body(scene, circle);
  }
  for (size_t i = NUM_BALLS / 6; i < 3 * NUM_BALLS / 6; i++) {
    color.r += (MAX_COL - MIN_COL) / 2 / (NUM_BALLS / 6.0);
    color.b -= (MAX_COL - MIN_COL) / 2 / (NUM_BALLS / 6.0);
    double x = RADIUS + (i * 2 * RADIUS);
    double y = compute_sigmoid_1(i);
    body_t *circle = circle_init((vector_t){.x = x, .y = y}, color);
    scene_add_body(scene, circle);
  }
  for (size_t i = NUM_BALLS / 2; i < 5 * NUM_BALLS / 6; i++) {
    color.r -= (MAX_COL - MIN_COL) / 2 / (NUM_BALLS / 6.0);
    color.g += (MAX_COL - MIN_COL) / 2 / (NUM_BALLS / 6.0);
    double x = RADIUS + (i * 2 * RADIUS);
    double y = compute_sigmoid_2(i);
    body_t *circle = circle_init((vector_t){.x = x, .y = y}, color);
    scene_add_body(scene, circle);
  }
  for (size_t i = 5 * NUM_BALLS / 6; i < NUM_BALLS; i++) {
    color.g -= (MAX_COL - MIN_COL) / 2 / (NUM_BALLS / 6.0);
    color.b += (MAX_COL - MIN_COL) / 2 / (NUM_BALLS / 6.0);
    double x = RADIUS + (i * 2 * RADIUS);
    double y = compute_sigmoid_2(i);
    body_t *circle = circle_init((vector_t){.x = x, .y = y}, color);
    scene_add_body(scene, circle);
  }
}

void make_anchors(scene_t *scene) {
  for (size_t i = 0; i < NUM_BALLS; i++) {
    body_t *anchor = anchor_init(
        (vector_t){.x = RADIUS + (i * 2 * RADIUS), .y = WINDOW_CENTER.y});
    scene_add_body(scene, anchor);
  }
}

void add_forces(scene_t *scene) {
  for (size_t j = NUM_BALLS; j < scene_bodies(scene); j++) {
    body_t *circle = scene_get_body(scene, j);
    body_t *anchor = scene_get_body(scene, j - NUM_BALLS);
    create_spring(scene, K, circle, anchor);
    create_drag(scene, DAMP, circle);
  }
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  state->scene = scene_init();

  make_anchors(state->scene);
  make_circles(state->scene);
  add_forces(state->scene);
  return state;
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}