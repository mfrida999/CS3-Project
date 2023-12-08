#include "scene.h"
#include "forces.h"
#include "stdlib.h"
#include <assert.h>
#include <unistd.h>

const size_t REASONABLE_GUESS = 12;

typedef struct force_creator_data {
  force_creator_t force_creator;
  void *aux;
  free_func_t freer;
  list_t *bodies;
} force_creator_data_t;

typedef struct scene {
  list_t *bodies;
  list_t *pre_force_creators;
  list_t *post_force_creators;
  bool clicked;
} scene_t;

void scene_set_clicked(scene_t *scene, bool clicked){
    scene->clicked = clicked;
}

void force_creator_data_freeer(force_creator_data_t *fcd) {
  if (fcd->aux != NULL && fcd->freer != NULL) {
    (fcd->freer)(fcd->aux);
  }
  if (fcd->bodies != NULL) {
    list_free(fcd->bodies);
  }
  free(fcd);
}

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  scene->bodies = list_init(REASONABLE_GUESS, (free_func_t)body_free);
  scene->pre_force_creators =
      list_init(REASONABLE_GUESS, (free_func_t)force_creator_data_freeer);
  scene->post_force_creators =
      list_init(REASONABLE_GUESS, (free_func_t)force_creator_data_freeer);
  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->pre_force_creators);
  list_free(scene->post_force_creators);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

void scene_add_body_front(scene_t *scene, body_t *body) {
  list_add_front(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_remove(scene_get_body(scene, index));
}

void scene_remove_body_override(scene_t *scene, size_t index) {
  list_remove(scene->bodies, index);
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  scene_add_bodies_force_creator(scene, forcer, aux, NULL, freer);
}

void scene_add_bodies_pre_force_creator(scene_t *scene,
                                    force_creator_t force_creator, void *aux,
                                    list_t *bodies, free_func_t freer) {
  force_creator_data_t *data = malloc(sizeof(force_creator_data_t));
  assert(data != NULL);
  data->force_creator = force_creator;
  data->aux = aux;
  data->freer = freer;
  data->bodies = bodies;

  list_add(scene->pre_force_creators, data);
}

void scene_add_bodies_force_creator(scene_t *scene,
                                    force_creator_t force_creator, void *aux,
                                    list_t *bodies, free_func_t freer) {
  scene_add_bodies_pre_force_creator(scene, force_creator, aux, bodies, freer);
}

void scene_add_bodies_post_force_creator(scene_t *scene,
                                    force_creator_t force_creator, void *aux,
                                    list_t *bodies, free_func_t freer) {
  force_creator_data_t *data = malloc(sizeof(force_creator_data_t));
  assert(data != NULL);
  data->force_creator = force_creator;
  data->aux = aux;
  data->freer = freer;
  data->bodies = bodies;

  list_add(scene->post_force_creators, data);
}

void scene_tick(scene_t *scene, double dt) {
  for (ssize_t i = list_size(scene->pre_force_creators) - 1; i >= 0; i--) {
    force_creator_data_t *fcd = list_get(scene->pre_force_creators, i);
    if (fcd->bodies != NULL) {
      for (size_t j = 0; j < list_size(fcd->bodies); j++) {
        if (body_is_removed(list_get(fcd->bodies, j))) {
          list_remove(scene->pre_force_creators, i);
          force_creator_data_freeer(fcd);
          break;
        }
      }
    }
  }

  for (size_t i = 0; i < list_size(scene->pre_force_creators); i++) {
    force_creator_data_t *fcd = list_get(scene->pre_force_creators, i);
    fcd->force_creator(fcd->aux);
  }

  for (ssize_t i = scene_bodies(scene) - 1; i >= 0; i--) {
    body_t *body = scene_get_body(scene, i);
    if (!body_is_removed(body)) {
      body_tick(body, dt);
    }
  }

  for (ssize_t i = list_size(scene->post_force_creators) - 1; i >= 0; i--) {
    force_creator_data_t *fcd = list_get(scene->post_force_creators, i);
    if (fcd->bodies != NULL) {
      for (size_t j = 0; j < list_size(fcd->bodies); j++) {
        if (body_is_removed(list_get(fcd->bodies, j))) {
          list_remove(scene->post_force_creators, i);
          force_creator_data_freeer(fcd);
          break;
        }
      }
    }
  }

  for (size_t i = 0; i < list_size(scene->post_force_creators); i++) {
    force_creator_data_t *fcd = list_get(scene->post_force_creators, i);
    fcd->force_creator(fcd->aux);
  }

  for (ssize_t i = list_size(scene->pre_force_creators) - 1; i >= 0; i--) {
    force_creator_data_t *fcd = list_get(scene->pre_force_creators, i);
    if (fcd->bodies != NULL) {
      for (size_t j = 0; j < list_size(fcd->bodies); j++) {
        if (body_is_removed(list_get(fcd->bodies, j))) {
          list_remove(scene->pre_force_creators, i);
          force_creator_data_freeer(fcd);
          break;
        }
      }
    }
  }
  for (ssize_t i = list_size(scene->post_force_creators) - 1; i >= 0; i--) {
    force_creator_data_t *fcd = list_get(scene->post_force_creators, i);
    if (fcd->bodies != NULL) {
      for (size_t j = 0; j < list_size(fcd->bodies); j++) {
        if (body_is_removed(list_get(fcd->bodies, j))) {
          list_remove(scene->post_force_creators, i);
          force_creator_data_freeer(fcd);
          break;
        }
      }
    }
  }

  for (ssize_t i = scene_bodies(scene) - 1; i >= 0; i--) {
    body_t *body = scene_get_body(scene, i);
    if (body_is_removed(body)) {
      list_remove(scene->bodies, i);
      body_free(body);
    } 
  }
}
