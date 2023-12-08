#include "forces.h"
#include "assert.h"
#include "collision.h"
#include "shape.h"
#include "stdlib.h"
#include <math.h>
#include "stdio.h"

const double MIN_DIST = 5.0;

typedef struct aux {
  list_t *bodies;
  list_t *constants;
} aux_t;

void aux_free(aux_t *aux) {
  if (aux->bodies != NULL) {
    list_free(((aux_t *)aux)->bodies);
  }
  if (aux->constants != NULL) {
    list_free(((aux_t *)aux)->constants);
  }
  free(aux);
}

void gravity_force_creator(aux_t *aux) {
  assert(list_size(aux->bodies) == 2);
  assert(list_size(aux->constants) == 1);
  vector_t r21 = vec_subtract(body_get_centroid(list_get(aux->bodies, 1)),
                              body_get_centroid(list_get(aux->bodies, 0)));
  double dist = sqrt(vec_dot(r21, r21));
  if (dist < MIN_DIST) {
    return;
  }
  double C = *(double *)list_get(aux->constants, 0);
  double scalar = (-1.0 * C * body_get_mass(list_get(aux->bodies, 0)) *
                   body_get_mass(list_get(aux->bodies, 1))) /
                  (vec_dot(r21, r21));
  scalar /= dist;
  vector_t f21 = vec_multiply(scalar, r21);
  body_add_force(list_get(aux->bodies, 1), f21);
  body_add_force(list_get(aux->bodies, 0), vec_negate(f21));
}

void one_way_gravity_force_creator(aux_t *aux){
  assert(aux != NULL);
  assert(list_size(aux->bodies) == 2);
  assert(list_size(aux->constants) == 2);
  
  double max_dist = *(double *) list_get(aux->constants, 1);

  assert (max_dist > 0);
  vector_t r21 = vec_subtract(body_get_centroid(list_get(aux->bodies, 1)),
                              body_get_centroid(list_get(aux->bodies, 0)));
  double dist = sqrt(vec_dot(r21, r21));

  if (dist > max_dist) {
    return;
  }

  double C = *(double *)list_get(aux->constants, 0);
  double scalar = (-1.0 * C * body_get_mass(list_get(aux->bodies, 0)) *
                   body_get_mass(list_get(aux->bodies, 1))) /
                  (vec_dot(r21, r21));
  scalar /= dist;
  vector_t f21 = vec_multiply(scalar, r21);
  body_add_force(list_get(aux->bodies, 1), f21);
}

void downward_gravity_force_creator(aux_t *aux) {
  assert(list_size(aux->bodies) == 1);
  assert(list_size(aux->constants) == 1);

  body_t *body = list_get(aux->bodies, 0);
  double gravity_constant = *(double *)list_get(aux->constants, 0);

  vector_t downward_force = {0, -gravity_constant * body_get_mass(body)};

  body_add_force(body, downward_force);
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->bodies = list_init(2, NULL);
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);
  aux->constants = list_init(1, free);
  double *c1 = malloc(sizeof(double));
  *c1 = G;
  list_add(aux->constants, c1);
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)gravity_force_creator,
                                 aux, bodies, (free_func_t)aux_free);
}

void create_one_way_gravity(double max_dist, scene_t *scene, double G, body_t *body1,
                            body_t *body2) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->bodies = list_init(2, NULL);
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);

  aux->constants = list_init(2, free);
  double *c1 = malloc(sizeof(double));
  *c1 = G;
  double *c2 = malloc(sizeof(double));
  *c2 = max_dist;
  list_add(aux->constants, c1);
  list_add(aux->constants, c2);

  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)one_way_gravity_force_creator,
                                    aux, bodies, (free_func_t)aux_free);
}

void create_downward_gravity(scene_t *scene, double G, body_t *body1) {
    aux_t *aux = malloc(sizeof(aux_t));
    aux->bodies = list_init(1, NULL);
    list_add(aux->bodies, body1);
    aux->constants = list_init(1, free);
    double *c1 = malloc(sizeof(double));
    *c1 = G;
    list_add(aux->constants, c1);
    list_t *bodies = list_init(1, NULL);
    list_add(bodies, body1);
    scene_add_bodies_force_creator(scene, (force_creator_t) downward_gravity_force_creator,
                                 aux, bodies, (free_func_t) aux_free);
}

void spring_force_creator(aux_t *aux) {
  assert(list_size(aux->bodies) == 2);
  assert(list_size(aux->constants) == 1);
  vector_t r21 = vec_subtract(body_get_centroid(list_get(aux->bodies, 1)),
                              body_get_centroid(list_get(aux->bodies, 0)));
  double C = *(double *)list_get(aux->constants, 0);
  double scalar = (-1.0 * C);
  vector_t f21 = vec_multiply(scalar, r21);
  body_add_force(list_get(aux->bodies, 1), f21);
  body_add_force(list_get(aux->bodies, 0), vec_negate(f21));
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->bodies = list_init(2, NULL);
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);
  aux->constants = list_init(1, free);
  double *c1 = malloc(sizeof(double));
  *c1 = k;
  list_add(aux->constants, c1);
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)spring_force_creator,
                                 aux, bodies, (free_func_t)aux_free);
}

void springy_rope_force_creator(aux_t *aux) {
  assert(list_size(aux->bodies) == 2);
  assert(list_size(aux->constants) == 2);
  double spring_len = *(double *)list_get(aux->constants, 1);
  vector_t r21 = vec_subtract(body_get_centroid(list_get(aux->bodies, 1)),
                              body_get_centroid(list_get(aux->bodies, 0)));
  if (vec_magnitude(r21) <= spring_len) {
    return;
  }
  r21 = vec_multiply((vec_magnitude(r21) - spring_len), vec_normalized(r21));
  double C = *(double *)list_get(aux->constants, 0);
  double scalar = (-1.0 * C);
  vector_t f21 = vec_multiply(scalar, r21);
  body_add_force(list_get(aux->bodies, 1), f21);
  body_add_force(list_get(aux->bodies, 0), vec_negate(f21));
}

void create_springy_rope(scene_t *scene, double k, body_t *body1, body_t *body2, double portion) {
  assert(portion >= 0);
  assert(portion <= 1);
  aux_t *aux = malloc(sizeof(aux_t));
  aux->bodies = list_init(2, NULL);
  list_add(aux->bodies, body1);
  list_add(aux->bodies, body2);
  aux->constants = list_init(2, free);
  double *c1 = malloc(sizeof(double));
  *c1 = k;
  list_add(aux->constants, c1);
  double *c2 = malloc(sizeof(double));
  *c2 = portion * vec_magnitude(vec_subtract(body_get_centroid(body1), body_get_centroid(body2)));
  list_add(aux->constants, c2);
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t) springy_rope_force_creator,
                                 aux, bodies, (free_func_t) aux_free);
}

void drag_force_creator(aux_t *aux) {
  assert(list_size(aux->bodies) == 1);
  assert(list_size(aux->constants) == 1);
  vector_t v = body_get_velocity(list_get(aux->bodies, 0));
  double C = *(double *)list_get(aux->constants, 0);
  double scalar = (-1.0 * C);
  vector_t f = vec_multiply(scalar, v);
  body_add_force(list_get(aux->bodies, 0), f);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->bodies = list_init(1, NULL);
  list_add(aux->bodies, body);
  aux->constants = list_init(1, free);
  double *c1 = malloc(sizeof(double));
  *c1 = gamma;
  list_add(aux->constants, c1);
  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, (force_creator_t)drag_force_creator,
                                 aux, bodies, (free_func_t)aux_free);
}

void collision_force_creator(void *aux) {
  collision_aux_t *temp = (collision_aux_t *)aux;
  collision_info_t info = find_collision(body_get_shape_shallow(temp->body1),
                                         body_get_shape_shallow(temp->body2));
  if (!temp->collided_previously && info.collided) {
    temp->handler(temp->body1, temp->body2, info.axis, temp->aux, info.overlap);
  }
  temp->collided_previously = info.collided;
}

void stronger_collision_force_creator(void *aux) {
  collision_aux_t *temp = (collision_aux_t *)aux;
  collision_info_t info = find_collision(body_get_shape_shallow(temp->body1),
                                         body_get_shape_shallow(temp->body2));
  if (info.collided) {
    temp->handler(temp->body1, temp->body2, info.axis, temp->aux, info.overlap);
  }
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  create_pre_collision(scene, body1, body2, handler, aux, freer);
}

void create_pre_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  collision_aux_t *collisiondata = malloc(sizeof(collision_aux_t));
  *collisiondata = (collision_aux_t){body1, body2, handler, aux, freer, false};
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_pre_force_creator(scene, collision_force_creator,
                                 (void *)collisiondata, bodies,
                                 (free_func_t)collision_aux_free);
}

void create_post_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  collision_aux_t *collisiondata = malloc(sizeof(collision_aux_t));
  *collisiondata = (collision_aux_t){body1, body2, handler, aux, freer, false};
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_post_force_creator(scene, collision_force_creator,
                                 (void *)collisiondata, bodies,
                                 (free_func_t)collision_aux_free);
}

void create_stronger_post_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  collision_aux_t *collisiondata = malloc(sizeof(collision_aux_t));
  *collisiondata = (collision_aux_t){body1, body2, handler, aux, freer, false};
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_post_force_creator(scene, stronger_collision_force_creator,
                                 (void *)collisiondata, bodies,
                                 (free_func_t) collision_aux_free);
}

void destructive_collision_force_creator(body_t *body1, body_t *body2,
                                         vector_t axis, void *aux, double overlap) {
  body_remove(body1);
  body_remove(body2);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  create_collision(scene, body1, body2, destructive_collision_force_creator,
                   NULL, NULL);
}

void one_way_destructive_collision_force_creator(body_t *body1, body_t *body2,
                                                 vector_t axis, void *aux, double overlap) {
  body_remove(body1);
}

void create_one_way_destructive_collision(scene_t *scene, body_t *body1,
                                          body_t *body2) {
  create_collision(scene, body1, body2,
                   one_way_destructive_collision_force_creator, NULL, NULL);
}

void physics_collision_force_creator(body_t *body1, body_t *body2,
                                     vector_t axis, void *aux, double overlap) {
  double elasticity = *(double *)aux;
  double reduced_mass = body_get_mass(body1) * body_get_mass(body2) /
                        (body_get_mass(body1) + body_get_mass(body2));
  if (body_get_mass(body1) == INFINITY) {
    reduced_mass = body_get_mass(body2);
  } else if (body_get_mass(body2) == INFINITY) {
    reduced_mass = body_get_mass(body1);
  }
  double vel_diff = (vec_dot(axis, body_get_velocity(body2)) -
                     vec_dot(axis, body_get_velocity(body1)));
  vector_t impulse =
      vec_multiply(reduced_mass * (1.0 + elasticity) * vel_diff, axis);
  body_add_impulse(body1, impulse);
  body_add_impulse(body2, vec_negate(impulse));
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  double *elast = malloc(sizeof(double));
  *elast = elasticity;
  create_collision(scene, body1, body2, physics_collision_force_creator,
                   (void *)elast, free);
}