#include "collision.h"
#include "forces.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const int POINTS = 20;
const size_t WIDTH_X = 800;
const size_t HEIGHT_Y = 800;
vector_t CENTER = {WIDTH_X / 2, HEIGHT_Y / 2};
const vector_t MIN = {0, 0};
const vector_t MAX = {WIDTH_X, HEIGHT_Y};
const vector_t BRICK_LOC = {500, 250};
const vector_t PLAYER_LOC = {500, 10};
const vector_t VEL = {750, 0};
const vector_t BALL_VEL = {-400, -200};
const int BRICK_HEIGHT = 40;
const int ROWS = 3;
const int COLS = 10;
const int BALL_RAD = 10;
const double BALL_MASS = 5.0;
const rgb_color_t RED = {1, 0, 0};
const rgb_color_t INVISIBLE_COLOR = {1, 1, 1};
const double LENGTH = 36;
const double HEIGHT = 15;
const double SPECIAL_BRICK_LEN = 15;
const double INVISIBLE_LEN = 1;
const double INVISIBLE_HEIGHT = 800;
const double SPACE = 3;
const double I_VEL = 100;
const double ELASTICITY = 1;
const vector_t SPECIAL_VELOCITY = {0, -100};
const double BRICK_MULTIPLIER = 0.6;
const rgb_color_t SPECIAL_COLOR = {0.9, 0.1, 0.05};
const rgb_color_t colors[] = {
    {.r = 1, .g = 0, .b = 0},   {.r = 1, .g = 0.5, .b = 0},
    {.r = 1, .g = 1, .b = 0},   {.r = 0.5, .g = 1, .b = 0},
    {.r = 0, .g = 1, .b = 0.5}, {.r = 0, .g = 1, .b = 1},
    {.r = 0, .g = 0.5, .b = 1}, {.r = 0.5, .g = 0, .b = 1},
    {.r = 1, .g = 0, .b = 1},   {.r = 1, .g = 0, .b = 0.5}};
const size_t num_colors = 10;

typedef struct state {
  scene_t *scene;
  bool special_brick_used;
} state_t;

list_t *make_rect(double length, double height) {
  list_t *rect = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){-length, -height};
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+length, -height};
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+length, +height};
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-length, +height};
  list_add(rect, v);
  return rect;
}

body_t *make_brick(rgb_color_t color) {
  body_t *body = body_init(make_rect(LENGTH, HEIGHT), INFINITY, color);
  return body;
}

body_t *make_player(scene_t *scene) {
  vector_t spawn = PLAYER_LOC;
  body_t *body = body_init(make_rect(LENGTH, HEIGHT), INFINITY, RED);
  body_set_centroid(body, spawn);
  scene_add_body(scene, body);
  return body;
}

body_t *make_ball(vector_t center) {
  list_t *ball_points = list_init(POINTS, (free_func_t)free);

  double curr_angle = 0;
  double vert_angle = 2 * M_PI / (POINTS);

  for (size_t i = 0; i < POINTS; i++) {
    vector_t *outer_p = malloc(sizeof(vector_t));
    outer_p->x = cos(curr_angle) * BALL_RAD + center.x;
    outer_p->y = sin(curr_angle) * BALL_RAD + center.y;
    list_add(ball_points, outer_p);
    curr_angle += vert_angle;
  }

  body_t *ball_body = body_init(ball_points, BALL_MASS, RED);
  body_set_velocity(ball_body, BALL_VEL);
  return ball_body;
}

void make_special_brick(scene_t *scene) {
  vector_t spawn = {WIDTH_X / 2, HEIGHT_Y - BRICK_HEIGHT};
  body_t *special_brick = body_init(
      make_rect(SPECIAL_BRICK_LEN, SPECIAL_BRICK_LEN), INFINITY, SPECIAL_COLOR);
  body_set_centroid(special_brick, spawn);
  body_set_velocity(special_brick, SPECIAL_VELOCITY);
  scene_add_body(scene, special_brick);
}

void init_make_bricks(scene_t *scene) {
  double y = HEIGHT_Y - BRICK_HEIGHT;
  double x = BRICK_HEIGHT;
  double down = BRICK_HEIGHT;
  for (size_t i = 0; i < ROWS; i++) {
    for (size_t j = 0; j < COLS; j++) {
      body_t *brick = make_brick(colors[j % num_colors]);
      body_set_centroid(brick, (vector_t){x + 2 * BRICK_HEIGHT * j, y});
      scene_add_body(scene, brick);
      create_one_way_destructive_collision(scene, brick,
                                           scene_get_body(scene, 1));
      create_physics_collision(scene, ELASTICITY, brick,
                               scene_get_body(scene, 1));
    }
    y -= down;
  }
}

list_t *make_invisible_bodies(scene_t *scene) {
  vector_t spawn1 = {0, HEIGHT_Y / 2};
  vector_t spawn2 = {WIDTH_X, HEIGHT_Y / 2};
  vector_t spawn3 = {WIDTH_X / 2, HEIGHT_Y};
  body_t *body1 = body_init(make_rect(INVISIBLE_LEN, INVISIBLE_HEIGHT),
                            INFINITY, INVISIBLE_COLOR);
  body_t *body2 = body_init(body_get_shape(body1), INFINITY, INVISIBLE_COLOR);
  body_t *body3 = body_init(make_rect(INVISIBLE_HEIGHT, INVISIBLE_LEN),
                            INFINITY, INVISIBLE_COLOR);
  body_set_centroid(body1, spawn1);
  body_set_centroid(body2, spawn2);
  body_set_centroid(body3, spawn3);
  scene_add_body(scene, body1);
  scene_add_body(scene, body2);
  scene_add_body(scene, body3);
  list_t *invisible_bodies = list_init(3, free);
  list_add(invisible_bodies, body1);
  list_add(invisible_bodies, body2);
  list_add(invisible_bodies, body3);
  return invisible_bodies;
}

void collide_with_walls(scene_t *scene) {
  list_t *invisible_bodies = make_invisible_bodies(scene);
  for (size_t i = 0; i < list_size(invisible_bodies); i++) {
    create_physics_collision(scene, ELASTICITY, scene_get_body(scene, 1),
                             list_get(invisible_bodies, i));
  }
}

void collide_with_player(scene_t *scene) {
  create_physics_collision(scene, ELASTICITY, scene_get_body(scene, 1),
                           scene_get_body(scene, 0));
}

void collide_with_special_brick(scene_t *scene) {
  size_t special_index = scene_bodies(scene) - 1;
  create_one_way_destructive_collision(
      scene, scene_get_body(scene, special_index), scene_get_body(scene, 0));
}

bool is_special_brick(state_t *state) {
  if (body_get_color(
          scene_get_body(state->scene, scene_bodies(state->scene) - 1))
              .r == SPECIAL_COLOR.r &&
      body_get_color(
          scene_get_body(state->scene, scene_bodies(state->scene) - 1))
              .g == SPECIAL_COLOR.g &&
      body_get_color(
          scene_get_body(state->scene, scene_bodies(state->scene) - 1))
              .b == SPECIAL_COLOR.b) {
    return true;
  } else {
    state->special_brick_used = true;
    printf("Special Feature: brick is gone yay\n");
    return false;
  }
}

bool player_bounds_left(body_t *body, double dt) {
  return (body_get_centroid(body).x - 4 * BALL_RAD +
              body_get_velocity(body).x * dt <=
          0);
}

bool player_bounds_right(body_t *body, double dt) {
  return (body_get_centroid(body).x + 4 * BALL_RAD +
              body_get_velocity(body).x * dt >=
          WIDTH_X);
}

bool ball_bounds_bottom(scene_t *scene, double dt) {
  return (body_get_centroid(scene_get_body(scene, 1)).y + 2 * BALL_RAD +
              body_get_velocity(scene_get_body(scene, 1)).y * dt <=
          0);
}

void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  scene_t *scene = state->scene;
  body_t *ellipse = scene_get_body(scene, 0);
  if (type == KEY_RELEASED) {
    switch (key) {
    case RIGHT_ARROW:
      if (body_get_velocity(ellipse).x > 0) {
        body_set_velocity(ellipse, VEC_ZERO);
      }
      break;
    case LEFT_ARROW:
      if (body_get_velocity(ellipse).x < 0) {
        body_set_velocity(ellipse, VEC_ZERO);
      }
      break;
    }
  } else {
    switch (key) {
    case RIGHT_ARROW: {
      body_set_velocity(ellipse, VEL);
      break;
    }
    case LEFT_ARROW: {
      body_set_velocity(ellipse, vec_negate(VEL));
      break;
    }
    }
  }
}

scene_t *initialize_scene(state_t *state) {
  scene_t *scene = scene_init();
  make_player(scene);
  scene_add_body(scene, make_ball(CENTER));
  init_make_bricks(scene);
  collide_with_walls(scene);
  collide_with_player(scene);
  make_special_brick(scene);
  collide_with_special_brick(scene);
  state->special_brick_used = false;
  return scene;
}

state_t *emscripten_init() {
  sdl_init(MIN, MAX);
  state_t *state = malloc(sizeof(state_t));
  state->scene = initialize_scene(state);
  sdl_on_key((key_handler_t)on_key);
  return state;
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  if (player_bounds_left(scene_get_body(state->scene, 0), dt)) {
    body_set_velocity(scene_get_body(state->scene, 0), VEC_ZERO);
    body_set_centroid(scene_get_body(state->scene, 0),
                      (vector_t){.x = 4 * BALL_RAD, .y = PLAYER_LOC.y});
  } else if (player_bounds_right(scene_get_body(state->scene, 0), dt)) {
    body_set_velocity(scene_get_body(state->scene, 0), VEC_ZERO);
    body_set_centroid(
        scene_get_body(state->scene, 0),
        (vector_t){.x = WIDTH_X - 4 * BALL_RAD, .y = PLAYER_LOC.y});
  }
  if (ball_bounds_bottom(state->scene, dt)) {
    scene_free(state->scene);
    state->scene = initialize_scene(state);
  }

  if (!state->special_brick_used && !is_special_brick(state)) {
    vector_t velocity = body_get_velocity(scene_get_body(state->scene, 1));
    velocity = vec_multiply(BRICK_MULTIPLIER, velocity);
    body_set_velocity(scene_get_body(state->scene, 1), velocity);
  }
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
}
void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}