#include "assert.h"
#include "forces.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const size_t NUM_STARS = 30;
const size_t NUM_POINTS_STAR = 4;
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const size_t MIN_RADIUS = 4;
const size_t MAX_RADIUS = 20;
const double STAR_SPAWN_CHANCE = 0.025;
const double WEIGHT_MULTIPLIER = 0.1;
const double G = 1e3;

typedef struct state {
  scene_t *scene;
} state_t;

vector_t randomize_center() {
  double x = (rand() % (size_t)(WINDOW.x + 1));
  double y = (rand() % (size_t)(WINDOW.y + 1));
  vector_t center = {.x = x, .y = y};
  return center;
}

body_t *make_star() {
  double inner_radius = MIN_RADIUS + rand() % (MAX_RADIUS - MIN_RADIUS);
  double outer_radius = 2 * inner_radius;
  list_t *star_points = list_init(2 * NUM_POINTS_STAR, (free_func_t)free);
  vector_t center = randomize_center();

  double angle_offset = M_PI / NUM_POINTS_STAR;

  for (size_t i = 0; i < NUM_POINTS_STAR; i++) {
    double angle = 2 * M_PI * i / NUM_POINTS_STAR;
    vector_t *outer_p = malloc(sizeof(vector_t));
    assert(outer_p != NULL);
    outer_p->x = cos(angle) * outer_radius + center.x;
    outer_p->y = sin(angle) * outer_radius + center.y;
    vector_t *inner_p = malloc(sizeof(vector_t));
    assert(inner_p != NULL);
    inner_p->x = cos(angle + angle_offset) * inner_radius + center.x;
    inner_p->y = sin(angle + angle_offset) * inner_radius + center.y;
    list_add(star_points, outer_p);
    list_add(star_points, inner_p);
  }
  rgb_color_t rand_color =
      (rgb_color_t){.r = (double)rand() / (double)RAND_MAX,
                    .g = (double)rand() / (double)RAND_MAX,
                    .b = (double)rand() / (double)RAND_MAX};

  body_t *star_body = body_init(
      star_points, WEIGHT_MULTIPLIER * outer_radius * inner_radius, rand_color);
  return star_body;
}

void star_spawn(scene_t *scene) {
  if (rand() < (double)RAND_MAX * STAR_SPAWN_CHANCE) {
    body_t *new_star = make_star();
    scene_add_body(scene, new_star);
    for (size_t i = 0; i < scene_bodies(scene) - 1; i++) {
      body_t *other = scene_get_body(scene, i);
      create_newtonian_gravity(scene, G, new_star, other);
    }
  }
}

state_t *emscripten_init() {
  // random seed every refresh
  srand(time(NULL));

  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  scene_t *scene = scene_init();

  // spawn stars randomly
  for (size_t i = 0; i < NUM_STARS; i++) {
    body_t *star = make_star();
    scene_add_body(scene, star);
    for (size_t j = 0; j < scene_bodies(scene) - 1; j++) {
      body_t *other = scene_get_body(scene, j);
      create_newtonian_gravity(scene, G, star, other);
    }
  }

  state_t *state = malloc(sizeof(state_t));
  state->scene = scene;
  return state;
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();

  scene_t *scene = state->scene;
  star_spawn(scene);
  scene_tick(scene, dt);
  sdl_render_scene(scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
