#include "../include/body.h"
#include "../include/color.h"
#include "../include/forces.h"
#include "../include/test_util.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const int ITER = 1000000;
const double TIME_STEP = 1e-6;
const double K = 3;
const double MASS1 = 7;
const double MASS2 = 5;
const double ERROR = 1e-4;
const double G = 1e3;
const double GAMMA = .2;
const vector_t VELOCITY = {10, 0};

list_t *make_figure() {
  list_t *figure = list_init(4, free);
  vector_t *vector = malloc(sizeof(*vector));
  *vector = (vector_t){-1, -1};
  list_add(figure, vector);
  vector = malloc(sizeof(*vector));
  *vector = (vector_t){1, -1};
  list_add(figure, vector);
  vector = malloc(sizeof(*vector));
  *vector = (vector_t){1, 1};
  list_add(figure, vector);
  vector = malloc(sizeof(*vector));
  *vector = (vector_t){-1, 1};
  list_add(figure, vector);
  return figure;
}

double spring_p_energy(double k, body_t *first, body_t *second) {
  vector_t d =
      vec_subtract(body_get_centroid(first), body_get_centroid(second));
  return 0.5 * k * vec_dot(d, d);
}

double k_energy(body_t *body) {
  vector_t v = body_get_velocity(body);
  return body_get_mass(body) * vec_dot(v, v) / 2;
}

void test_spring_behavior() {
  scene_t *scene = scene_init();
  body_t *first = body_init(make_figure(), MASS1, (rgb_color_t){0, 0, 0});
  body_set_centroid(first, (vector_t){50, 50});
  scene_add_body(scene, first);
  body_t *second = body_init(make_figure(), MASS1, (rgb_color_t){0, 0, 0});
  body_set_centroid(second, (vector_t){10, 20});
  scene_add_body(scene, second);
  create_spring(scene, K, first, second);
  double initial_energy = spring_p_energy(K, first, second);
  for (int i = 0; i < ITER; i++) {
    double energy =
        spring_p_energy(K, first, second) + k_energy(first) + k_energy(second);
    assert(within(ERROR, energy / initial_energy, 1));
    scene_tick(scene, TIME_STEP);
  }
  scene_free(scene);
}

double gravitational_p_energy(double G, body_t *first, body_t *second) {
  vector_t d =
      vec_subtract(body_get_centroid(first), body_get_centroid(second));
  return -G * body_get_mass(first) * body_get_mass(second) /
         sqrt(vec_dot(d, d));
}

void test_gravity_behavior() {
  scene_t *scene = scene_init();
  body_t *first = body_init(make_figure(), MASS1, (rgb_color_t){0, 0, 0});
  body_set_centroid(first, (vector_t){0, 0});
  scene_add_body(scene, first);
  body_t *second = body_init(make_figure(), MASS2, (rgb_color_t){0, 0, 0});
  body_set_centroid(second, (vector_t){20, 10});
  scene_add_body(scene, second);
  create_newtonian_gravity(scene, G, first, second);
  double initial_energy = gravitational_p_energy(G, first, second);
  for (int i = 0; i < ITER; i++) {
    double energy = gravitational_p_energy(G, first, second) + k_energy(first) +
                    k_energy(second);
    assert(within(ERROR, energy / initial_energy, 1));
    scene_tick(scene, TIME_STEP);
  }
  scene_free(scene);
}

void test_drag_behavior() {
  scene_t *scene = scene_init();
  body_t *first = body_init(make_figure(), MASS1, (rgb_color_t){0, 0, 0});
  body_set_velocity(first, VELOCITY);
  scene_add_body(scene, first);
  create_drag(scene, GAMMA, first);
  for (int i = 0; i < ITER; i++) {
    assert(within(ERROR, body_get_velocity(first).x,
                  10 * exp(-GAMMA * i * TIME_STEP / MASS1)));
    scene_tick(scene, TIME_STEP);
  }
  scene_free(scene);
}

int main(int argc, char *argv[]) {
  bool all_tests = argc == 1;
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_spring_behavior)
  DO_TEST(test_gravity_behavior)
  DO_TEST(test_drag_behavior)

  puts("student_test PASS");
}
