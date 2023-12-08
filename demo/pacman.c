#include "assert.h"
#include "body.h"
#include "list.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// star initialization constants
const size_t PACMAN_RADIUS = 64;
const rgb_color_t PACMAN_COLOR = (rgb_color_t){.r = 1.0, .g = 1.0, .b = 0.0};
const size_t PELLET_RADIUS = 6;
const rgb_color_t PELLET_COLOR = (rgb_color_t){.r = 1.0, .g = 1.0, .b = 0.0};
const double DEFAULT_VELOCITY = 175;
const double START_ANGLE = M_PI / 6;
const size_t NUM_POINTS_PACMAN = 100;
const size_t NUM_POINTS_PELLET = 20;
const vector_t CENTER = (vector_t){.x = 500, .y = 250};
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const double ACCELERATION = 750;
const double PELLET_CHANCE = 0.01;

typedef struct state {
  scene_t *scene;
  bool is_held;
  int last_dir_held;
} state_t;

body_t *make_pacman(vector_t center) {
  list_t *pacman_points = list_init(NUM_POINTS_PACMAN, (free_func_t)free);

  double curr_angle = START_ANGLE;
  double vert_angle = (5.0 / 6.0) * 2 * M_PI / (NUM_POINTS_PACMAN);

  for (size_t i = 0; i < NUM_POINTS_PACMAN - 1; i++) {
    vector_t *outer_p = malloc(sizeof(vector_t));
    outer_p->x = cos(curr_angle) * PACMAN_RADIUS + center.x;
    outer_p->y = sin(curr_angle) * PACMAN_RADIUS + center.y;
    list_add(pacman_points, outer_p);
    curr_angle += vert_angle;
  }

  vector_t *outer_p = malloc(sizeof(vector_t));
  outer_p->x = CENTER.x;
  outer_p->y = CENTER.y;
  list_add(pacman_points, outer_p);

  body_t *pacman_body = body_init(pacman_points, 5.0, PACMAN_COLOR);
  return pacman_body;
}

body_t *make_pellet(vector_t center) {
  list_t *pellet_points = list_init(NUM_POINTS_PELLET, (free_func_t)free);

  double curr_angle = 0;
  double vert_angle = 2 * M_PI / (NUM_POINTS_PELLET);

  for (size_t i = 0; i < NUM_POINTS_PELLET; i++) {
    vector_t *outer_p = malloc(sizeof(vector_t));
    outer_p->x = cos(curr_angle) * PELLET_RADIUS + center.x;
    outer_p->y = sin(curr_angle) * PELLET_RADIUS + center.y;
    list_add(pellet_points, outer_p);
    curr_angle += vert_angle;
  }

  body_t *pellet_body = body_init(pellet_points, 5.0, PELLET_COLOR);
  return pellet_body;
}

vector_t randomize_center() {
  double x = (rand() % (size_t)(WINDOW.x + 1));
  double y = (rand() % (size_t)(WINDOW.y + 1));
  vector_t center = {.x = x, .y = y};
  return center;
}

void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW:
      state->is_held = 1;
      state->last_dir_held = 1;
      break;
    case UP_ARROW:
      state->is_held = 1;
      state->last_dir_held = 2;
      break;
    case DOWN_ARROW:
      state->is_held = 1;
      state->last_dir_held = 4;
      break;
    case RIGHT_ARROW:
      state->is_held = 1;
      state->last_dir_held = 3;
      break;
    }
  }
  if (type == KEY_RELEASED) {
    state->is_held = 0;
  }
}

void pellet_spawn(scene_t *scene) {
  if (rand() < (double)RAND_MAX * PELLET_CHANCE) {
    vector_t center = randomize_center();
    body_t *new_pellet = make_pellet(center);
    scene_add_body(scene, new_pellet);
  }
}

void move_pacman(state_t *state, double dt) {
  scene_t *scene = state->scene;
  body_t *pacman = scene_get_body(scene, 0);
  vector_t curr_vel = body_get_velocity(pacman);
  if (curr_vel.x == 0) {
    curr_vel.x = DEFAULT_VELOCITY;
  }
  if (curr_vel.y == 0) {
    curr_vel.y = DEFAULT_VELOCITY;
  }
  if (state->is_held == 1 && state->last_dir_held == 1) {
    // left
    double y = 0;
    double x = (-1 * fabs(curr_vel.x)) - (ACCELERATION * dt);
    vector_t new_v = {.x = x, .y = y};
    body_set_velocity(pacman, new_v);
    body_set_rotation(pacman, M_PI);
  }

  else if (state->is_held == 1 && state->last_dir_held == 2) {
    // up
    double x = 0;
    double y = fabs(curr_vel.y) + (ACCELERATION * dt);
    vector_t new_v = {.x = x, .y = y};
    body_set_velocity(pacman, new_v);
    body_set_rotation(pacman, M_PI / 2.0);
  }

  else if (state->is_held == 1 && state->last_dir_held == 3) {
    // right
    double y = 0;
    double x = fabs(curr_vel.x) + (ACCELERATION * dt);
    vector_t new_v = {.x = x, .y = y};
    body_set_velocity(pacman, new_v);
    body_set_rotation(pacman, 0);
  }

  else if (state->is_held == 1 && state->last_dir_held == 4) {
    // down
    double x = 0;
    double y = (-1 * fabs(curr_vel.y)) - (ACCELERATION * dt);
    vector_t new_v = {.x = x, .y = y};
    body_set_velocity(pacman, new_v);
    body_set_rotation(pacman, (3.0 * M_PI) / 2.0);
  }

  if (state->is_held == 0) {
    vector_t curr_vel = body_get_velocity(pacman);
    if (curr_vel.x > 0)
      curr_vel.x = DEFAULT_VELOCITY;
    if (curr_vel.x < 0)
      curr_vel.x = -DEFAULT_VELOCITY;
    if (curr_vel.y > 0)
      curr_vel.y = DEFAULT_VELOCITY;
    if (curr_vel.y < 0)
      curr_vel.y = -DEFAULT_VELOCITY;
    body_set_velocity(pacman, curr_vel);
  }
}

void pellets_disappear(scene_t *scene) {
  body_t *pacman = scene_get_body(scene, 0);
  for (size_t h = 1; h < scene_bodies(scene); h++) {
    body_t *curr_pellet = scene_get_body(scene, h);
    vector_t diff =
        vec_subtract(body_get_centroid(pacman), body_get_centroid(curr_pellet));
    if ((sqrt(vec_dot(diff, diff))) < (PACMAN_RADIUS + PELLET_RADIUS)) {
      scene_remove_body(scene, h);
    }
  }
}

void wrap_around(scene_t *scene, body_t *pacman) {
  vector_t center = body_get_centroid(pacman);
  if ((center.x - PACMAN_RADIUS) > WINDOW.x) {
    center.x -= WINDOW.x + 2 * PACMAN_RADIUS;
  } else if ((center.x + PACMAN_RADIUS) < 0) {
    center.x += WINDOW.x + 2 * PACMAN_RADIUS;
  }
  if ((center.y - PACMAN_RADIUS) > WINDOW.y) {
    center.y -= WINDOW.y + 2 * PACMAN_RADIUS;
  } else if ((center.y + PACMAN_RADIUS) < 0) {
    center.y += WINDOW.y + 2 * PACMAN_RADIUS;
  }
  body_set_centroid(pacman, center);
}

state_t *emscripten_init() {
  // random seed every refresh
  srand(time(NULL));
  state_t *state = malloc(sizeof(state_t));
  state->is_held = 0;
  state->last_dir_held = 3;

  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  // scene creation
  scene_t *scene = scene_init();

  // number of pellets + objects
  int num_pellets = (rand() % (20 - 10)) + 10;

  // pacman - initialize place in list
  body_t *pacman = make_pacman(CENTER);
  scene_add_body(scene, pacman);

  // randomize pellets
  for (size_t i = 0; i < num_pellets; i++) {
    vector_t center = randomize_center();
    body_t *pellet = make_pellet(center);
    scene_add_body(scene, pellet);
  }

  sdl_on_key(on_key);
  state->scene = scene;

  return state;
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  scene_t *scene = state->scene;
  move_pacman(state, dt);
  pellets_disappear(scene);
  pellet_spawn(scene);
  scene_tick(scene, dt);
  wrap_around(scene, scene_get_body(scene, 0));
  sdl_render_scene(scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}