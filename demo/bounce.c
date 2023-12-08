#include "color.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// window constants
const vector_t WINDOW = (vector_t){.x = 1000, .y = 500};
const vector_t CENTER = (vector_t){.x = 500, .y = 250};

// star initialization constant
const vector_t LINEAR_VELOCITY = (vector_t){.x = 300, .y = 300};
const double ANGULAR_VELOCITY = -M_PI / 1.5;
const size_t OUTER_RADIUS = 100;
const size_t INNER_RADIUS = 35;
const size_t NUM_POINTS = 5;

// color constants
const rgb_color_t color = {.r = 1.0, .g = 1.0, .b = 0.0};

void free_star_inator(void *s) {
  list_t *star = (list_t *)s;
  list_free(star);
  free(star);
}

list_t *make_star(size_t outer_r, size_t inner_r, size_t num_points,
                  size_t center_x, size_t center_y) {
  list_t *star = list_init(2 * NUM_POINTS, free_star_inator);
  assert(num_points > 0);
  double angle_offset = M_PI / num_points;

  for (size_t i = 0; i < num_points; i++) {
    double angle = 2 * M_PI * i / num_points;
    vector_t *outer_p = malloc(sizeof(vector_t));
    outer_p->x = cos(angle) * outer_r + center_x;
    outer_p->y = sin(angle) * outer_r + center_y;
    vector_t *inner_p = malloc(sizeof(vector_t));
    inner_p->x = cos(angle + angle_offset) * inner_r + center_x;
    inner_p->y = sin(angle + angle_offset) * inner_r + center_y;
    list_add_back(star, outer_p);
    list_add_back(star, inner_p);
  }
  return star;
}

typedef struct state {
  list_t *star;
  vector_t linear_velocity;
  double angular_velocity;
} state_t;

state_t *emscripten_init() {
  vector_t min = (vector_t){.x = 0, .y = 0};
  vector_t max = WINDOW;
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  state->linear_velocity = LINEAR_VELOCITY;
  state->angular_velocity = ANGULAR_VELOCITY;
  state->star =
      make_star(OUTER_RADIUS, INNER_RADIUS, NUM_POINTS, CENTER.x, CENTER.y);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  list_t *star = state->star;
  polygon_rotate(star, state->angular_velocity * dt, polygon_centroid(star));
  polygon_translate(star, vec_multiply(dt, state->linear_velocity));
  for (size_t i = 0; i < list_size(star); i++) {
    vector_t *point = list_get(star, i);
    if (point->x >= WINDOW.x) {
      polygon_translate(star,
                        vec_negate(vec_multiply(dt, state->linear_velocity)));
      state->linear_velocity.x = -fabs(state->linear_velocity.x);
      polygon_translate(star, vec_multiply(dt, state->linear_velocity));
      break;
    }
    if (point->x <= 0) {
      polygon_translate(star,
                        vec_negate(vec_multiply(dt, state->linear_velocity)));
      state->linear_velocity.x = fabs(state->linear_velocity.x);
      polygon_translate(star, vec_multiply(dt, state->linear_velocity));
      break;
    }
    if (point->y >= WINDOW.y) {
      polygon_translate(star,
                        vec_negate(vec_multiply(dt, state->linear_velocity)));
      state->linear_velocity.y = -fabs(state->linear_velocity.y);
      polygon_translate(star, vec_multiply(dt, state->linear_velocity));
      break;
    }
    if (point->y <= 0) {
      polygon_translate(star,
                        vec_negate(vec_multiply(dt, state->linear_velocity)));
      state->linear_velocity.y = fabs(state->linear_velocity.y);
      polygon_translate(star, vec_multiply(dt, state->linear_velocity));
      break;
    }
  }
  sdl_draw_polygon(star, color);
  sdl_show();
}

void emscripten_free(state_t *state) {
  list_t *star = state->star;
  list_free(star);
  free(state);
}
