#include "forces.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct state {
  scene_t *scene;
  int invaders_at_bottom_count;
} state_t;

const size_t WIDTH_X = 1000;
const size_t HEIGHT_Y = 500;
const vector_t MIN = {0, 0};
const vector_t MAX = {WIDTH_X, HEIGHT_Y};
const vector_t INVADER_LOC = {500, 250};
const vector_t SHOOTER_LOC = {500, 10};
const vector_t VEL = {500, 0};
const int I_RAD = 40;
const int ROWS = 3;
const int COLS = 7;
const int S_RAD = 10;
const double MASS = 1;
const rgb_color_t I_COLOR = {0, 1, 0};
const rgb_color_t BS_COLOR = {0, 1, 0};
const rgb_color_t BI_COLOR = {1, 0, 0};
const double ALPHA = 4 * M_PI / 3;
const double B_VEL = 500;
const double SHOOTING_RATE = 0.0001;
const double LENGTH = 1;
const double HEIGHT = 4;
const double SPACE = 10;
const double I_VEL = 100;
const size_t B_ID = 0;
const size_t S_ID = 1;
const size_t I_ID = 2;
const double DAMPING = -0.5;
const double POINTS = 30;

body_t *make_invaders() {
  list_t *figure = list_init(POINTS + 1, (free_func_t)free);
  for (size_t i = 0; i < POINTS; i++) {
    double angle = ALPHA + 2 * (M_PI - ALPHA) * i / POINTS;
    vector_t *v = malloc(sizeof(*v));
    *v = (vector_t){I_RAD * sin(angle), I_RAD * cos(angle)};
    *v = vec_add(*v, INVADER_LOC);
    list_add(figure, v);
  }
  vector_t *SPAWN = malloc(sizeof(vector_t));
  *SPAWN = INVADER_LOC;
  list_add(figure, SPAWN);
  body_t *body = body_init_with_info(figure, MASS, I_COLOR, (void *)I_ID, NULL);
  body_set_rotation(body, M_PI);
  return body;
}

list_t *make_rect() {
  list_t *rect = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){-LENGTH, -HEIGHT};
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+LENGTH, -HEIGHT};
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+LENGTH, +HEIGHT};
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-LENGTH, +HEIGHT};
  list_add(rect, v);
  return rect;
}

body_t *make_shooter() {
  vector_t spawn = SHOOTER_LOC;
  list_t *points = list_init(POINTS, (free_func_t)free);
  for (size_t i = 0; i < POINTS; i++) {
    double angle = 2 * M_PI * i / POINTS;
    vector_t *v = malloc(sizeof(*v));
    *v = (vector_t){3 * S_RAD * cos(angle), S_RAD * sin(angle)};
    *v = vec_add(*v, spawn);
    list_add(points, v);
  }
  rgb_color_t rand_color =
      (rgb_color_t){.r = (double)rand() / (double)RAND_MAX,
                    .g = (double)rand() / (double)RAND_MAX,
                    .b = (double)rand() / (double)RAND_MAX};
  return body_init_with_info(points, MASS, rand_color, (void *)S_ID, NULL);
}

body_t *shoot(body_t *shooter, bool direction, scene_t *scene) {
  vector_t centr = body_get_centroid(shooter);
  rgb_color_t color = (rgb_color_t){.r = (double)rand() / (double)RAND_MAX,
                                    .g = (double)rand() / (double)RAND_MAX,
                                    .b = (double)rand() / (double)RAND_MAX};
  vector_t vel = {0, B_VEL};
  if (!direction) {
    color = BI_COLOR;
    vel = vec_negate(vel);
  }
  body_t *bullet =
      body_init_with_info(make_rect(), MASS, color, (void *)B_ID, NULL);
  body_set_velocity(bullet, vel);
  body_set_centroid(bullet, centr);
  scene_add_body(scene, bullet);
  return bullet;
}

void b_off_screen(scene_t *scene) {
  for (int i = 0; i < scene_bodies(scene); i++) {
    body_t *body = scene_get_body(scene, i);
    if (body_get_info(body) == 0) {
      if (body_get_centroid(body).y > HEIGHT_Y ||
          body_get_centroid(body).y < 0) {
        body_remove(body);
      }
    }
  }
}

bool hits_edge_shooter(body_t *body, double dt) {
  return (
      body_get_centroid(body).x + 4 * S_RAD + body_get_velocity(body).x * dt >=
          WIDTH_X ||
      body_get_centroid(body).x - 4 * S_RAD + body_get_velocity(body).x * dt <=
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
    case ' ': {
      body_t *bullet = shoot(ellipse, true, scene);
      for (size_t i = 0; i < scene_bodies(scene); i++) {
        if (body_get_info(scene_get_body(scene, i)) == (void *)I_ID) {
          create_destructive_collision(scene, scene_get_body(scene, i), bullet);
        }
      }
    }
    }
  }
}

bool hits_edge_invader(body_t *body, double dt) {
  return (body_get_centroid(body).x + I_RAD + body_get_velocity(body).x * dt >=
          WIDTH_X) ||
         (body_get_centroid(body).x - I_RAD + body_get_velocity(body).x * dt <=
          0);
}

int move_invaders(scene_t *scene, double dt) {
  int count = 0;
  double shift = (I_RAD + SPACE) * 3;
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    if (body_get_info((body_t *)scene_get_body(scene, i)) == (void *)I_ID &&
        hits_edge_invader(scene_get_body(scene, i), dt)) {
      vector_t centroid = body_get_centroid(scene_get_body(scene, i));
      vector_t new_centroid = vec_add(centroid, (vector_t){0, -shift});
      body_set_centroid(scene_get_body(scene, i), new_centroid);
      vector_t update_velocity =
          vec_negate(body_get_velocity(scene_get_body(scene, i)));
      body_set_velocity(scene_get_body(scene, i), update_velocity);
      if (new_centroid.y - I_RAD <= 0) {
        count++;
      }
    }
  }
  return count;
}

void init_make_invaders(scene_t *scene) {
  double y = HEIGHT_Y - I_RAD;
  double x = I_RAD + 10;
  double down = I_RAD + SPACE;
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      body_t *invaders = make_invaders();
      body_set_centroid(invaders, (vector_t){x + 2 * I_RAD * j, y});
      body_set_velocity(invaders, (vector_t){I_VEL, 0});
      scene_add_body(scene, invaders);
    }
    y -= down;
  }
}

state_t *emscripten_init() {
  srand(time(NULL));
  state_t *state = malloc(sizeof(state_t));
  state->invaders_at_bottom_count = 0;
  sdl_init(MIN, MAX);
  scene_t *scene = scene_init();
  body_t *fighter = make_shooter();
  scene_add_body(scene, fighter);
  init_make_invaders(scene);
  sdl_on_key((key_handler_t)on_key);
  state->scene = scene;
  return state;
}

void rand_shoot(scene_t *scene) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *shooter = scene_get_body(scene, i);
    if (body_get_info(shooter) == (void *)I_ID) {
      if (rand() < (double)RAND_MAX * SHOOTING_RATE) {
        body_t *bullet = shoot(shooter, false, scene);
        create_destructive_collision(scene, scene_get_body(scene, 0), bullet);
      }
    }
  }
}

void emscripten_main(state_t *state) {
  if (scene_bodies(state->scene) <= 1 ||
      (size_t)body_get_info((body_t *)scene_get_body(state->scene, 0)) !=
          S_ID ||
      (size_t)body_get_info((body_t *)scene_get_body(state->scene, 1)) !=
          I_ID) {
    exit(0);
  }

  double dt = time_since_last_tick();

  if (hits_edge_shooter(scene_get_body(state->scene, 0), dt)) {
    body_set_velocity(scene_get_body(state->scene, 0),
                      vec_multiply(DAMPING, body_get_velocity(scene_get_body(
                                                state->scene, 0))));
  }
  rand_shoot(state->scene);
  state->invaders_at_bottom_count += move_invaders(state->scene, dt);
  if (state->invaders_at_bottom_count > 0) {
    sdl_render_scene(state->scene);
    exit(0);
  }

  b_off_screen(state->scene);
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
