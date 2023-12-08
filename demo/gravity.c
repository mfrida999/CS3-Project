#include "color.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// window constants
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};

// state initialization constant
const size_t NUM_STARS = 15;
const vector_t STAR_SPAWN = (vector_t){.x = 100, .y = 400};

// star initialization constants
const double ANGULAR_VELOCITY = -M_PI / 1.5;
const size_t OUTER_RADIUS = 75;
const size_t INNER_RADIUS = 25;
const double GRAVITY = -350;
const size_t MIN_POINTS = 3;
const size_t MAX_POINTS = 6;
const vector_t INITIAL_VELOCITY = {.x = 175, .y = 0};
const double MIN_ELASTICITY = 0.65;
const double MAX_ELASTICITY = 0.9;

typedef struct state {
  list_t *stars;
  size_t stars_left;
} state_t;

typedef struct star {
  list_t *points;
  vector_t linear_velocity;
  rgb_color_t color;
  double elasticity;
} star_t;

void free_star_inator(void *s) {
  star_t *star = (star_t *)s;
  list_free(star->points);
  free(star);
}

star_t *make_star() {
  star_t *poly_star = malloc(sizeof(star_t));
  size_t num_points = (rand() % (MAX_POINTS)) + MIN_POINTS;
  poly_star->color = (rgb_color_t){.r = (double)rand() / (double)RAND_MAX,
                                   .g = (double)rand() / (double)RAND_MAX,
                                   .b = (double)rand() / (double)RAND_MAX};
  poly_star->linear_velocity = INITIAL_VELOCITY;
  poly_star->elasticity =
      ((float)rand() / (float)RAND_MAX) * (MAX_ELASTICITY - MIN_ELASTICITY) +
      MIN_ELASTICITY;

  list_t *star = list_init(2 * num_points, (free_func_t)free);
  double angle_offset = M_PI / num_points;
  poly_star->points = star;

  for (size_t i = 0; i < num_points; i++) {
    double angle = 2 * M_PI * i / num_points;
    vector_t *outer_p = malloc(sizeof(vector_t));
    outer_p->x = cos(angle) * OUTER_RADIUS + STAR_SPAWN.x;
    outer_p->y = sin(angle) * OUTER_RADIUS + STAR_SPAWN.y;
    vector_t *inner_p = malloc(sizeof(vector_t));
    inner_p->x = cos(angle + angle_offset) * INNER_RADIUS + STAR_SPAWN.x;
    inner_p->y = sin(angle + angle_offset) * INNER_RADIUS + STAR_SPAWN.y;
    list_add(star, outer_p);
    list_add(star, inner_p);
  }
  return poly_star;
}

state_t *emscripten_init() {
  // random seed every refresh
  srand(time(NULL));

  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  list_t *poly_stars =
      list_init(WINDOW.x / (2 * OUTER_RADIUS), free_star_inator);
  list_add_front(poly_stars, make_star());

  state_t *state = malloc(sizeof(state_t));
  state->stars_left = NUM_STARS - 1;
  state->stars = poly_stars;

  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();

  if (list_size(state->stars) > 0) {
    if (state->stars_left > 0 &&
        polygon_centroid(((star_t *)list_get(state->stars, 0))->points).x >
            (4 * OUTER_RADIUS)) {
      list_add_front(state->stars, make_star());
      state->stars_left = state->stars_left - 1;
    }

    for (size_t i = 0; i < list_size(state->stars); i++) {
      star_t *star = list_get(state->stars, i);
      list_t *points = star->points;
      polygon_rotate(points, ANGULAR_VELOCITY * dt, polygon_centroid(points));
      star->linear_velocity.y += (GRAVITY * dt);
      polygon_translate(points, vec_multiply(dt, star->linear_velocity));

      for (size_t k = 0; k < list_size(points); k++) {
        vector_t *point = list_get(points, k);
        if (point->y <= 0) {
          polygon_translate(
              points, vec_negate(vec_multiply(dt, star->linear_velocity)));
          star->linear_velocity.y =
              fabs(star->linear_velocity.y) * (star->elasticity);
          polygon_translate(points, vec_multiply(dt, star->linear_velocity));
          break;
        }
      }
    }

    star_t *last_star = list_get(state->stars, list_size(state->stars) - 1);
    list_t *last_points = last_star->points;

    if (polygon_centroid(last_points).x > (OUTER_RADIUS + WINDOW.x)) {
      list_remove(state->stars, list_size(state->stars) - 1);
    }

    for (size_t i = 0; i < list_size(state->stars); i++) {
      star_t *curr_star = list_get(state->stars, i);
      sdl_draw_polygon(curr_star->points, curr_star->color);
    }
  }
  sdl_show();
}

void emscripten_free(state_t *state) {
  list_free(state->stars);
  free(state);
}