#include "forces.h"
#include "collision.h"
#include "sdl_wrapper.h"
#include "sdl_mixer.h"
#include "state.h"
#include "shape.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <emscripten.h>

//top wall constants

const char *instructions = "GAME: SLIME\nGOAL: Get to the end without dying. You can die by hitting a thorny wall or losing too much mass. You can grab pickups that replenish your mass along the way. There are checkpoints to help you through.\nABILITY 1: jump\nPoint and click to jump. You can only jump once every 5 seconds or every time you are stuck to a wall. \nPro tip: You can click and hold to slow down time while aiming your jump.\nNote: Players often try to jump through corners of walls. This is not possible. You must jump AWAY from a wall, not along or into it.\nABILITY 2: grapple\nPoint and press spacebar to grapple. Note: If the grapple is not strong enough to pull you away from the wall, you will simply stay stuck and the grapple will disappear. This happens if you accrue too much mass or grapple to something too close to you.\nSHORTCUTS:\nR to respawn at last checkpoint\nL to skip to the end and play the game winning cutscene";
const int TOP_WALLS = 50; 

const double JUMP_RESET = 5;

const size_t WIN_FRAMES = 63;
const double WIN_ANIMATION_PAUSE = 0.075;

const double BACKGROUND_X_1 = 4100;
const double BACKGROUND_Y_1 = 800;
const size_t NUM_BX_1 = 5;
const size_t NUM_BY_1 = 6;
const double PARALLAX_MULTIPLIER_1 = 0.3;

const double BACKGROUND_X_2 = 3580;
const double BACKGROUND_Y_2 = 800;
const size_t NUM_BX_2 = 5;
const size_t NUM_BY_2 = 6;
const double PARALLAX_MULTIPLIER_2 = 0.2;

const double BACKGROUND_X_3 = 5405;
const double BACKGROUND_Y_3 = 1000;
const size_t NUM_BX_3 = 2;
const size_t NUM_BY_3 = 4;
const double PARALLAX_MULTIPLIER_3 = 0.1;

const double DEATH_GROW_FACTOR = 1.1;
const double DEATH_ANIMATION_FRAME_PAUSE = 0.001;
const double RESPAWN_ANIMATION_FRAME_PAUSE = 0.01;
const double RESPAWN_GROW_FACTOR = 1.2;
const double ANIMATION_FRAMES = 13;

const double CHECKPOINT_FRAMES = 11;
const size_t NUM_CHECKPOINTS = 2;
const double CHECKPOINT_ANIMATION_PAUSE = 0.01;
const double LEFT_LEFT_WALL_Y_COORD = 20.0;
const double LEVEL_ONE_SPACING = 600;

// bottom wall constants
const int LEVEL_ONE_BOTTOM_WALLS = 70;

const int LEVEL_TWO_BOTTOM_WALLS = 42;
const double LEVEL_THREE_BOTTOM_WALLS = 43;
const double LEVEL_TWO_END_WALL_HEIGHT = 100;
const double LEVEL_ONE_TOP_WALLS_HEIGHT = 1400;
const double LEVEL_ONE_TOP_WALLS_WIDTH = 132;
const double LEVEL_ONE_CEILING_PLATFORMS_Y_COORD = 660;
const double LEVEL_ONE_CEILING_PICKUP_SPAWN = 7;
const double LEVEL_ONE_PLATFORM_WALLS = 12;
const double FIRST_LEVEL_END_X_COORD = 7100;

const int64_t TWENTY = 20;
const double PICKUP_CHECKPOINT = 50;

const double END_OF_GAME_RADIUS = 75;

//left wall constants
const int LEFT_WALLS = 50;

//right wall constants
const int LEV_ONE_END = 10;

//platform wall constants

const int UPWARD_TUNNEL = 5;
const double LEVEL_THREE_HORIZONTAL_PLATFORMS = 15;

const int LEVEL_TWO_PLATFORM_WALLS = 18;
const double LEVEL_ONE_END_WALLS_HEIGHT = 160;

const double DOWNWARD_TUNNEL_OBSTACLES = 5;

const double LEV_TWO_END = 25;
// //level three constants
// const int LEVEL_THREE_BOTTOM_WALLS = 70;
const int LEVEL_THREE_PLATFORM_WALLS = 10;
const int LEVEL_THREE_TOP_WALLS = 70;
const int DOWNWARD_TUNNEL = 11;

const size_t NUM_ARROWS = 5;
const double ARROW_SPACE = 25;
const double PROGRESSIVE_SPACE = 2;

const double JUMP_DAMPER = 0.25;

const double DECREASE_DIST = 10.0;
const double SLIME_MASS_LOSS_FACTOR = 0.99975;
const double SLIME_RADIUS_LOSS_FACTOR = 0.99975;
const double DEATH_RADIUS = 35;

const double MIN_WALL_WIDTH = 100.0;
const double MAX_WALL_WIDTH = 240.0;
const double MIN_WALL_HEIGHT =  160.0;
const double MAX_WALL_HEIGHT = 300.0;

const double MAX_CUSHION_WALL_WIDTH = 1000;
const double MAX_CUSHION_WALL_HEIGHT = 1000;
const double BOTTOM_CUSHION_X_COORD = -1000;
const double BOTTOM_CUSHION_Y_COORD = -6000;
const double TOP_CUSHION_X_COORD = -1000;
const double TOP_CUSHION_Y_COORD = 2800;
const double NUM_CUSHION_WALLS = 30;

const double BASE_BLOCK_X = 50;
const double BASE_BLOCK_Y = 50;

const double MAX_DT = 0.05;

const double GRAVITY = 800;
const double ONE_WAY_GRAVITY = 1000000;

const rgb_color_t INVISIBLE = {.r = 1.0, .g = 1.0, .b = 1.0, .a = 1.0};
const rgb_color_t SUPER_INVISIBLE = {.r = 1.0, .g = 1.0, .b = 1.0, .a = 0.0};

const double BORDER_LEN = 500;

const vector_t WINDOW = (vector_t){.x = 1200, .y = 800};
const vector_t WINDOW_CENTER = (vector_t){.x = 600, .y = 400};

const double HOOK_FURTHER_X = 250;
const double HOOK_FURTHER_Y = 200;


const double SLOW_FACTOR = 0.1;

const double SLIME_MASS = 12.5;
const double PICKUP_MASS = 5;
const rgb_color_t SLIME_COLOR = {.r = 0.0, .g = 1.0, .b = 0.0, .a = 1.0};
vector_t SLIME_SPAWN = {.x = 500, .y = 400};

const double SLIME_RADIUS_GROWTH_FACTOR = 1.1; 
const double SLIME_MASS_GROWTH_FACTOR = 1.05;
const size_t NUM_POINTS = 20;
double INITIAL_SLIME_RADIUS = 50;

const double PICKUP_RADIUS = 20;
const rgb_color_t PICKUP_COLOR = {.r = 0.0, .g = 0.0, .b = 1.0, .a = 1.0};
const vector_t TEST_PICKUP_SPAWN_POINT = {.x = 800, .y = 600};
const double MAX_DIST = 200;

const rgb_color_t REGULAR_WALL_COLOR = {.r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0};
const rgb_color_t THORNY_WALL_COLOR = {.r = 1.0, .g = 0.0, .b = 0.0, .a = 1.0};

const double HOOK_RADIUS = 20;
const rgb_color_t HOOK_COLOR = {.r = 0.0, .g = 1.0, .b = 0.0, .a = 1.0};
const double HOOK_VEL = 2000;
const double HOOK_SPRING_K = 100.0;
const double LINE_WIDTH = 10;
const rgb_color_t LINE_COLOR = {.r = 236.0/255.0, .g = 100.0/255.0, .b = 164.0/255.0, .a = 1.0};

const double MAX_JUMP_LEN = 500.0;
const double MIN_JUMP_LEN = 100.0;
const double MIN_MULT = 1.0;
const double MAX_MULT = 2.0;
const double JUMP_MULTIPLIER = 5000.0;

const double GRAPPLE_PORTION = 0.5;

//wall constants
const double WALL_HEIGHT = 40;
const double WALL_WIDTH = 200;

const double LEFT_WALL_WIDTH = 400;
const double LEFT_WALL_HEIGHT = 100;
const double BOTTOM_BOTTOM_WALL_HEIGHT = 400;
const double UPWARD_TUNNEL_X_COORD = 6900;

//checkpoint constants:
const double CHECK_POINT_LEN = 44; //+ 2.0/3.0;
const double CHECK_POINT_HEIGHT = 50;// + 2.0/3.0;
const double CHECK_POINT_MASS = 10;
const rgb_color_t CHECK_POINT_COLOR = {.r = 0.0, .g = 1.0, .b = 1.0, .a = 1.0};
const double CHECK_POINT_MAX_DIST = 100;

const double LEVEL_THREE_WALL_WIDTH = 100;

const double OPENING_ANIMATION_PAUSE = 0.05;
const size_t OPENING_ANIMATION_FRAMES = 56;

typedef struct state {
  scene_t *scene;
  vector_t shift;
  body_t *invis_wall1;
  body_t *invis_wall2;
  list_t *backgrounds_1;
  void *bimg_1;
  list_t *backgrounds_2;
  void *bimg_2;
  list_t *backgrounds_3;
  void *bimg_3;
  bool mouse_is_held;
  vector_t spawn_point;
  vector_t spawn_shift;
  double spawn_radius;
  double spawn_mass;
  void *wall_img;
  void *thorny_img;
  list_t *animation;
  double respawn;
  double dying;
  list_t *checkpoint_animation;
  list_t *checkpoints;
  list_t *checkpoint_statuses;
  ssize_t won;
  list_t *won_animation;
  void *final_pipe;
  double jump_timer;
  size_t opening;
  list_t *opening_animation;
} state_t;

typedef enum {
    SLIME,
    REGULAR_WALL,
    THORNY_WALL,
    SLIME_PICKUP,
    HOOK,
    LINE,
    BORDER,
    ARROW,
    CHECK_POINT,
    BACKGROUND,
    ANIMATOR
} body_type_t;

typedef struct object_info {
  body_type_t body_type;
  free_func_t info_freeer;
  void *info;
} object_info_t;

void object_info_freeer(object_info_t *object_info) {
  if (object_info->info != NULL && object_info->info_freeer != NULL) {
    object_info->info_freeer(object_info->info);
  }
  free(object_info);
}

typedef struct slime_info {
  bool frozen;
  vector_t frozen_pos;
  body_t *hook;
  body_t *line;
  list_t *arrows;
  vector_t old_pos;
  double radius;
  double old_radius;
  double dist_travelled;
} slime_info_t;

typedef struct checkpoint_info {
  size_t num;
} checkpoint_info_t;

body_type_t get_type(body_t *body) {
  return ((object_info_t *) body_get_info(body))->body_type;
}

bool is_frozen(body_t *slime) {
  return ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->frozen;
}

double get_radius(body_t *slime) {
  return ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->radius;
}

void set_radius(body_t *slime, double radius) {
  assert(radius > 0);
  ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->radius = radius;
  set_circle(slime, radius);
}

void set_dist(body_t *slime, double dist) {
  ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->dist_travelled = dist;
}

double get_dist(body_t *slime) {
  return ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->dist_travelled;
}

double get_num(body_t *checkpoint) {
  return ((checkpoint_info_t *) ((object_info_t *) body_get_info(checkpoint))->info)->num;
}

double get_old_radius(body_t *slime) {
  return ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->old_radius;
}

void set_old_radius(body_t *slime, double radius) {
  assert(radius > 0);
  ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->old_radius = radius;
}

vector_t get_old_pos(body_t *slime) {
  return ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->old_pos;
}

void set_old_pos(body_t *slime, vector_t old_pos) {
  ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->old_pos = old_pos;
}

void freeze(body_t *slime) {
  ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->frozen = true;
  ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->frozen_pos = body_get_centroid(slime);
  body_set_velocity(slime, VEC_ZERO);
}

void unfreeze(body_t *slime) {
  ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->frozen = false;
}

body_t *get_hook(body_t *slime) {
  return ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->hook;
}

body_t *get_line(body_t *slime) {
  return ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->line;
}

void set_hook(body_t *slime, body_t *hook) {
  ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->hook = hook;
}

void set_line(body_t *slime, body_t *line) {
  ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->line = line;
}

void unhook(body_t *slime) {
  if (get_hook(slime) != NULL) {
    body_remove(get_hook(slime));
    remove_img_from_body(get_hook(slime));
    set_hook(slime, NULL);
  }
  if (get_line(slime) != NULL) {
    body_remove(get_line(slime));
    set_line(slime, NULL);
  }
}

void stick_collision_force_creator(body_t *body1, body_t *body2, vector_t axis, void *aux, double overlap) {
  vector_t rel_dir = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
  if (vec_dot(rel_dir, axis) > 0) {
    body_set_centroid(body1, vec_subtract(body_get_centroid(body1), vec_multiply(overlap, axis)));
  }
  else {
    body_set_centroid(body1, vec_add(body_get_centroid(body1), vec_multiply(overlap, axis)));
  }
  freeze(body1);
  if (get_hook(body1) != NULL) {
    if (body_get_velocity(get_hook(body1)).x == 0 && body_get_velocity(get_hook(body1)).y == 0) {
      unhook(body1);
    }
  }
}

void update_circle_radius(list_t *points, vector_t center, double radius) {
  for (size_t i = 0; i < list_size(points); i++) {
    vector_t oldpoint = *(vector_t *)list_get(points, i);
    vector_t newpoint = vec_add(center, vec_multiply(radius, 
    vec_normalized(vec_subtract(oldpoint, center))));
    *(vector_t *)list_get(points, i) = newpoint;
  }
}

void increase_slime_collision_force(body_t *body1, body_t *body2,
                                     vector_t axis, void *aux, double overlap) {
  set_radius(body1, get_radius(body1)*SLIME_RADIUS_GROWTH_FACTOR);
  update_circle_radius(body_get_shape_shallow(body1), body_get_centroid(body1), get_radius(body1));
  body_set_mass(body1, body_get_mass(body1) * SLIME_MASS_GROWTH_FACTOR);
  body_remove(body2);
  if (body_get_img(body2) != NULL) {
    remove_img_from_body(body2);
  }
}

void one_way_slime_collision_force_creator(body_t *body1, body_t *body2,
                                                 vector_t axis, void *aux, double overlap) {
  state_t *state = aux;
  state->dying = 0;
  remove_img_from_body(body1);
}

void create_one_way_slime_collision(state_t *state, body_t *body1,
                                          body_t *body2) {
  create_collision(state->scene, body1, body2,
                   one_way_slime_collision_force_creator, state, NULL);
}


void create_stick_collision(scene_t *scene, body_t *body1, body_t *body2) {
  create_stronger_post_collision(scene, body1, body2, stick_collision_force_creator, (void *) scene, NULL);
}

void hook_collision_force_creator(body_t *body1, body_t *body2, vector_t axis, void *aux, double overlap) {
  body_set_velocity(body1, VEC_ZERO);
  vector_t rel_dir = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
  if (vec_dot(rel_dir, axis) > 0) {
    body_set_centroid(body1, vec_subtract(body_get_centroid(body1), vec_multiply(overlap, axis)));
  }
  else {
    body_set_centroid(body1, vec_add(body_get_centroid(body1), vec_multiply(overlap, axis)));
  }
  scene_t *scene = aux;
  // if (is_dead(scene)) return;
  create_springy_rope(scene, HOOK_SPRING_K, scene_get_body(scene, 0), body1, GRAPPLE_PORTION);
  unfreeze(scene_get_body(scene, 0));
}

void create_hook_collision(scene_t *scene, body_t *body1, body_t *body2) {
  create_post_collision(scene, body1, body2, hook_collision_force_creator, (void *) scene, NULL);
}

void checkpoint_collision_force_creator(body_t *slime, body_t *checkpoint, vector_t axis, void *aux, double overlap) {
  state_t *state = aux;
  state->spawn_point = body_get_centroid(slime);
  state->spawn_shift = vec_add(state->shift, vec_subtract(body_get_centroid(slime), SLIME_SPAWN));
  state->spawn_mass = body_get_mass(slime);
  state->spawn_radius = get_radius(slime);
  if (*(size_t *)list_get(state->checkpoint_statuses, get_num(checkpoint)) == 0) {
    size_t *status = list_get(state->checkpoint_statuses, get_num(checkpoint));
    *status = *status + 1;
  }
}

void create_checkpoint_collision(state_t *state, body_t *slime, body_t *checkpoint) {
  create_post_collision(state->scene, slime, checkpoint, checkpoint_collision_force_creator, (void *) state, NULL);
}

void winning_collision_force_creator(body_t *slime, body_t *checkpoint, vector_t axis, void *aux, double overlap) {
  state_t *state = aux;
  if (state->won == -1) {
    state->won = 0;
  }
}

void create_winning_collision(state_t *state, body_t *slime, body_t *platform) {
  create_post_collision(state->scene, slime, platform, winning_collision_force_creator, (void *) state, NULL);
}

void unhook_collision_force_creator(body_t *body1, body_t *body2, vector_t axis, void *aux, double overlap) {
  // if (is_dead((scene_t *) aux)) return;
  unhook(scene_get_body((scene_t *) aux, 0)); 
}

void create_unhook_collision(scene_t *scene, body_t *line, body_t *body2) {
  create_collision(scene, line, body2, unhook_collision_force_creator, (void *) scene, NULL);
}

list_t *make_wall(vector_t bottom_left, double len, double height) {
  size_t x_blocks = len/BASE_BLOCK_X;
  size_t y_blocks = height/BASE_BLOCK_Y;
  list_t *blocks = list_init(x_blocks * y_blocks, NULL);
  vector_t pos = VEC_ZERO;
  for (size_t x = 0; x < x_blocks; x++) {
    pos.x = bottom_left.x + x*BASE_BLOCK_X;
    for (size_t y = 0; y < y_blocks; y++) {
      pos.y = bottom_left.y + y*BASE_BLOCK_Y;
      list_add(blocks, make_rect(pos, BASE_BLOCK_X, BASE_BLOCK_Y));
    }
  }
  return blocks;
}

void slime_jump(state_t *state, vector_t mouse_pos) {
  scene_t *scene = state->scene;
  body_t *slime = scene_get_body(scene, 0);
  if (!is_frozen(slime) && state->jump_timer < JUMP_RESET) {
    return;
  }
  state->jump_timer = 0.0;
  jumpSoundEffects();
  unfreeze(slime);
  unhook(scene_get_body(scene, 0));
  body_set_velocity(slime, vec_multiply(JUMP_DAMPER, body_get_velocity(slime)));
  vector_t slime_pos = body_get_centroid(slime);
  vector_t direction = vec_subtract(mouse_pos, slime_pos);
  if (vec_magnitude(direction) < MIN_JUMP_LEN) {
    direction = vec_multiply(MIN_JUMP_LEN, vec_normalized(direction));
  }
  else if (vec_magnitude(direction) > MAX_JUMP_LEN) {
    direction = vec_multiply(MAX_JUMP_LEN, vec_normalized(direction));
  }
  double percent = (vec_magnitude(direction)-MIN_JUMP_LEN)/(MAX_JUMP_LEN-MIN_JUMP_LEN);
  double mult = MIN_MULT + percent*MAX_MULT;
  double scalar = JUMP_MULTIPLIER * mult;
  body_add_impulse(slime, vec_multiply(scalar, vec_normalized(direction)));
}

void initialize_arrows(scene_t *scene) {
  body_t *slime = scene_get_body(scene, 0);
  list_t *arrows = list_init(NUM_ARROWS, NULL);
  ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->arrows = arrows;
  vector_t bottom_left = body_get_centroid(slime);
  bottom_left.x -= get_radius(slime);
  bottom_left.y += get_radius(slime);
  for (size_t i = 0; i < NUM_ARROWS; i++) {
    list_t *arrow_points = make_rect(bottom_left, 2*get_radius(slime), ARROW_SPACE);
    bottom_left.y += ARROW_SPACE;
    bottom_left.y += PROGRESSIVE_SPACE*i;
    object_info_t *a_info = malloc(sizeof(object_info_t));
    a_info->body_type = ARROW;
    a_info->info = NULL;
    a_info->info_freeer = NULL;
    body_t *arrow = body_init_with_info(arrow_points, INFINITY, SUPER_INVISIBLE, a_info, (free_func_t) object_info_freeer);
    scene_add_body(scene, arrow);
    list_add(arrows, arrow);
  }
}

void reset_arrows(scene_t *scene) {
  body_t *slime = scene_get_body(scene, 0);
  list_t *arrows = ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->arrows;
  for (size_t i = 0; i < list_size(arrows); i++) {
    body_t *arrow = list_get(arrows, i);
    remove_img_from_body(arrow);
  }
}

void update_arrows(state_t *state) {
  scene_t *scene = state->scene;
  body_t *slime = scene_get_body(scene, 0);
  vector_t old = get_old_pos(slime);
  vector_t slime_pos = body_get_centroid(slime);
  list_t *arrows = ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->arrows;
  for (size_t i = 0; i < NUM_ARROWS; i++) {
    body_t *arrow = list_get(arrows, i);
    body_set_centroid(arrow, vec_add(body_get_centroid(arrow), vec_subtract(slime_pos, old)));
    if (get_old_radius(slime) != get_radius(slime)) {
      double old = get_old_radius(slime);
      double curr = get_radius(slime);
      body_set_centroid(arrow, vec_add(body_get_centroid(arrow), 
      vec_multiply(curr - old, vec_normalized(vec_subtract(body_get_centroid(arrow), body_get_centroid(slime))))));
    }
  }
  if (state->mouse_is_held) {
    char arrow_path[20];
    strcpy(arrow_path, "assets/arrow1.png");
    size_t path_num_index = 12;
    size_t num_arrows = 0;
    vector_t mouse_pos = sdl_mouse_pos();
    mouse_pos.y = WINDOW.y - mouse_pos.y;
    vector_t from_slime_to_mouse = vec_subtract(mouse_pos, slime_pos);
    double len = vec_magnitude(from_slime_to_mouse);
    if (len < MIN_JUMP_LEN) {
      num_arrows = 1;
    }
    else if (len > MAX_JUMP_LEN) {
      num_arrows = NUM_ARROWS;
    }
    else {
      double portion = (len - MIN_JUMP_LEN)/(MAX_JUMP_LEN - MIN_JUMP_LEN);
      num_arrows = 1 + portion*(NUM_ARROWS - 1);
    }
    double angle = atan2(from_slime_to_mouse.y, from_slime_to_mouse.x) - M_PI/2;
    if (angle < 0) {
      angle += 2*M_PI;
    }
    num_arrows -= 1;
    for (size_t i = 0; i < list_size(arrows); i++) {
      body_t *arrow = list_get(arrows, i);
      body_set_rotation_about_point(arrow, angle, body_get_centroid(slime));
      if (i <= num_arrows) {
        if (body_get_img(arrow) == NULL) {
          arrow_path[path_num_index] += i;
          body_set_img(arrow, get_image_from_path(arrow_path));
          arrow_path[path_num_index] -= i;
        }
      }
      else {
        remove_img_from_body(arrow);
      }
    }
  }
}

body_t *initialize_slime(state_t *state) {
  object_info_t *slime_info = malloc(sizeof(object_info_t));
  slime_info->body_type = SLIME;
  slime_info_t *sub_slime_info = malloc(sizeof(slime_info_t));
  sub_slime_info->frozen = false;
  sub_slime_info->hook = NULL;
  sub_slime_info->line = NULL;
  sub_slime_info->old_pos = vec_add(state->spawn_point, state->spawn_shift);
  sub_slime_info->old_radius = state->spawn_radius;
  slime_info->info = sub_slime_info;
  slime_info->info_freeer = free;
  body_t *slime = body_init_with_info(make_circle(vec_add(state->spawn_point, state->spawn_shift), state->spawn_radius, 
    NUM_POINTS), state->spawn_mass, SUPER_INVISIBLE, slime_info, (free_func_t) object_info_freeer);
  set_dist(slime, 0.0);
  set_radius(slime, state->spawn_radius);
  set_circle(slime, state->spawn_radius);
  return slime;
}

body_t *initialize_checkpoint_flag(vector_t bottom_left, state_t *state) {
  object_info_t *checkpoint_info = malloc(sizeof(object_info_t));
  checkpoint_info->body_type = CHECK_POINT;
  checkpoint_info_t *sub_info = malloc(sizeof(checkpoint_info_t));
  sub_info->num = list_size(state->checkpoints);
  checkpoint_info->info_freeer = free;
  checkpoint_info->info = sub_info;
  body_t *check_point = body_init_with_info(make_rect(bottom_left, CHECK_POINT_LEN, CHECK_POINT_HEIGHT),
                                            CHECK_POINT_MASS, SUPER_INVISIBLE, checkpoint_info,
                                            (free_func_t) object_info_freeer);

  return check_point;
}

void add_normal_wall(state_t *state, vector_t bottom_left, double len, double height) {
  scene_t *scene = state->scene;
  list_t *wall_points = make_rect(bottom_left, len, height);
  object_info_t *w_info = malloc(sizeof(object_info_t));
  w_info->body_type = REGULAR_WALL;
  w_info->info = NULL;
  w_info->info_freeer = NULL;
  body_t *wall = body_init_with_info(wall_points, INFINITY, SUPER_INVISIBLE, w_info, (free_func_t) object_info_freeer);
  set_rect(wall, len, height);
  body_set_img(wall, state->wall_img);
  scene_add_body(scene, wall);
  create_stick_collision(scene, scene_get_body(scene, 0), wall);
}

void add_winning_wall(state_t *state, vector_t center, double radius) {
  scene_t *scene = state->scene;
  list_t *wall_points = make_circle(center, radius, NUM_POINTS);
  object_info_t *w_info = malloc(sizeof(object_info_t));
  w_info->body_type = REGULAR_WALL;
  w_info->info = NULL;
  w_info->info_freeer = NULL;
  body_t *wall = body_init_with_info(wall_points, INFINITY, SUPER_INVISIBLE, w_info, (free_func_t) object_info_freeer);
  set_circle(wall, radius);
  body_set_img(wall, state->final_pipe);
  scene_add_body(scene, wall);
  create_winning_collision(state, scene_get_body(scene, 0), wall);
}

void add_thorny_wall(state_t *state, vector_t bottom_left, double len, double height) {
  scene_t *scene = state->scene;
  list_t *wall_points = make_rect(bottom_left, len, height);
  object_info_t *w_info = malloc(sizeof(object_info_t));
  w_info->body_type = THORNY_WALL;
  w_info->info = NULL;
  w_info->info_freeer = NULL;
  body_t *wall = body_init_with_info(wall_points, INFINITY, SUPER_INVISIBLE, w_info, (free_func_t) object_info_freeer);
  set_rect(wall, len, height);
  body_set_img(wall, state->thorny_img);
  scene_add_body(scene, wall);
  create_one_way_slime_collision(state, scene_get_body(scene, 0), wall);
}

int make_mod_twenty(int length){
  int remainder = length % TWENTY;
  return remainder;
}

int generate_random_width() {
  int width = rand() % (int)(MAX_WALL_WIDTH - MIN_WALL_WIDTH) + MIN_WALL_WIDTH;
  return width;
}

int generate_random_height() { 
  int height = rand() % (int)(MAX_WALL_HEIGHT - MIN_WALL_HEIGHT) + MIN_WALL_HEIGHT;
  int remainder = make_mod_twenty(height);
  height = (height - remainder);
  return height;
}

void add_bottom_cushion_thorny(state_t *state){
  double x_coord = BOTTOM_CUSHION_X_COORD; 
  for(size_t i = 0; i < NUM_CUSHION_WALLS; i ++){
    double width = MAX_CUSHION_WALL_WIDTH;
    double height= MAX_CUSHION_WALL_HEIGHT;
    vector_t bottom_left = {.x = x_coord, .y = BOTTOM_CUSHION_Y_COORD};
    add_thorny_wall(state, bottom_left, width, height);
    x_coord += width;
  }
}

void add_top_cushion_thorny(state_t *state){
  double x_coord = TOP_CUSHION_X_COORD; 
  for(size_t i = 0; i < NUM_CUSHION_WALLS; i ++){
    double width = MAX_CUSHION_WALL_WIDTH;
    double height= MAX_CUSHION_WALL_HEIGHT;
    vector_t bottom_left = {.x = x_coord, .y = TOP_CUSHION_Y_COORD};
    add_thorny_wall(state, bottom_left, width, height);
    x_coord += width;
  }
}


void level_one_top_walls(state_t *state){
  double x_coord = VEC_ZERO.x; 
  for (size_t i = 0; i < TOP_WALLS; i++) {
    double width = LEVEL_ONE_TOP_WALLS_WIDTH;
    double height = LEVEL_ONE_TOP_WALLS_HEIGHT;
    double y_coord = WINDOW.y - WALL_HEIGHT; 
    vector_t bottom_left = {.x = x_coord, .y = y_coord};
    add_normal_wall(state, bottom_left, width, height);
    x_coord += width;
  }
}

void level_one_bottom_walls(state_t *state) {
  double x_coord = VEC_ZERO.x; 
  for (size_t i = 0; i < LEVEL_ONE_BOTTOM_WALLS; i++) {
    int width = generate_random_width();
    int height = generate_random_height();
    vector_t bottom_left = {.x = x_coord, .y = VEC_ZERO.y};
    add_thorny_wall(state, bottom_left, width, height);
    x_coord += width; 
  } 
}

void level_one_bottom_bottom_walls(state_t *state) {
  double x_coord = VEC_ZERO.x; 
  for (size_t i = 0; i < LEVEL_ONE_BOTTOM_WALLS; i++) {
    int width = WALL_WIDTH;
    int height = BOTTOM_BOTTOM_WALL_HEIGHT;
    vector_t bottom_left = {.x = x_coord, .y = -BOTTOM_BOTTOM_WALL_HEIGHT};
    add_thorny_wall(state, bottom_left, width, height);
    x_coord += width; 
  } 
}

void make_pickup(scene_t *scene, vector_t spawn) {
  object_info_t *pickup_info = malloc(sizeof(object_info_t));
  pickup_info->body_type = SLIME_PICKUP;
  pickup_info->info = NULL;
  pickup_info->info_freeer = NULL;
  body_t *pickup = body_init_with_info(make_circle(spawn, PICKUP_RADIUS, 
                                        NUM_POINTS), PICKUP_MASS, SUPER_INVISIBLE, pickup_info, (free_func_t) object_info_freeer);
  scene_add_body(scene, pickup);
  create_one_way_gravity(MAX_DIST, scene, ONE_WAY_GRAVITY, scene_get_body(scene, 0), pickup);
  create_collision(scene, scene_get_body(scene, 0), pickup, increase_slime_collision_force, NULL, NULL);
  body_set_img(pickup, get_image_from_path("assets/pickup.png"));
}

void make_checkpoint(state_t *state, vector_t spawn) {
  body_t *checkpoint = initialize_checkpoint_flag((vector_t){.x = spawn.x, spawn.y + PICKUP_CHECKPOINT}, state);
  scene_add_body(state->scene, checkpoint);
  body_set_img(checkpoint, get_image_from_path(list_get(state->checkpoint_animation, 0)));
  list_add(state->checkpoints, checkpoint);
  create_checkpoint_collision(state, scene_get_body(state->scene, 0), checkpoint);
}

void init_left_left_wall(state_t *state){
  double y_coord = LEFT_LEFT_WALL_Y_COORD;
  for (size_t i = 0; i < LEFT_WALLS; i++) {
    double width = LEFT_WALL_WIDTH;
    double height = LEFT_WALL_HEIGHT;
    double x_coord = -LEFT_WALL_WIDTH; 
    vector_t top_left = {.x = x_coord, .y = y_coord};
    add_thorny_wall(state, top_left, width, height);
    y_coord += height;
  }
}

void init_left_wall(state_t *state){
  double y_coord = LEFT_LEFT_WALL_Y_COORD;
  for (size_t i = 0; i < LEFT_WALLS; i++) {
    double width = generate_random_width();
    double height = LEFT_WALL_HEIGHT;
    double x_coord = VEC_ZERO.x; 
    vector_t top_left = {.x = x_coord, .y = y_coord};
    add_thorny_wall(state, top_left, width, height);
    y_coord += height;
  }
}

void level_one_ceiling_platforms(state_t *state){
  scene_t *scene = state->scene;
  list_t *store_platforms = list_init(7, free);
  double spacing = LEVEL_ONE_SPACING;
  double x_coord = WALL_WIDTH; 
  for (size_t i = 0; i < LEVEL_ONE_PLATFORM_WALLS; i++) {
    double width = WALL_WIDTH;
    double height = WALL_HEIGHT;
    double y_coord = LEVEL_ONE_CEILING_PLATFORMS_Y_COORD; 
    vector_t bottom_left = {.x = x_coord + spacing, .y = y_coord};
    add_normal_wall(state, bottom_left, width, height);
    x_coord += height + spacing;

    body_t *curr_platform = scene_get_body(scene, scene_bodies(scene)-1);
    list_add(store_platforms, curr_platform);
  }
  vector_t spawn_point = (vector_t) {.x = SLIME_SPAWN.x, .y = SLIME_SPAWN.y};

  spawn_point = body_get_centroid(list_get(store_platforms, LEVEL_ONE_CEILING_PICKUP_SPAWN));
  spawn_point = (vector_t) {.x = spawn_point.x, .y = spawn_point.y - PICKUP_CHECKPOINT};

  make_pickup(scene, spawn_point);
}

void level_one_base_platforms(state_t *state){
  scene_t *scene = state->scene;
  list_t *store_platforms = list_init(6, free);
  double spacing = LEVEL_ONE_SPACING;
  double x_coord = -WALL_WIDTH; 
  for (size_t i = 0; i < LEVEL_ONE_PLATFORM_WALLS; i++) {
    double width = WALL_WIDTH;
    double height = WALL_HEIGHT;
    double y_coord = WALL_WIDTH + MIN_WALL_WIDTH; 
    vector_t bottom_left = {.x = x_coord + spacing, .y = y_coord};
    add_normal_wall(state, bottom_left, width, height);
    x_coord += height + spacing;

    body_t *curr_platform = scene_get_body(scene, scene_bodies(scene)-1);
    list_add(store_platforms, curr_platform);
  }

  vector_t spawn_point = (vector_t) {.x = SLIME_SPAWN.x, .y = SLIME_SPAWN.y};

  spawn_point = body_get_centroid(list_get(store_platforms, 2));
  spawn_point = (vector_t) {.x = spawn_point.x, .y = spawn_point.y + PICKUP_CHECKPOINT};

  make_pickup(scene, spawn_point);
}

void make_upward_tunnel(state_t *state){
  scene_t *scene = state->scene;
  list_t *store_walls = list_init(13, free);
  double spacing = WALL_WIDTH;
  double y_coord = LEFT_WALL_HEIGHT - WALL_HEIGHT; 
  for (size_t i = 0; i < UPWARD_TUNNEL; i++) {
    double width = generate_random_width();
    double height = generate_random_height();
    double x_coord = UPWARD_TUNNEL_X_COORD; 
    vector_t top_left = {.x = x_coord, .y = y_coord + spacing};
    add_normal_wall(state, top_left, width, width);
    y_coord += height + spacing;

    body_t *curr_wall = scene_get_body(scene, scene_bodies(scene)-1);
    list_add(store_walls, curr_wall);
  }
  
  vector_t spawn_point = body_get_centroid(list_get(store_walls, 0));
  spawn_point = (vector_t) {.x = spawn_point.x, .y = spawn_point.y + 150};

  make_pickup(scene, spawn_point);
}

void first_level_end(state_t *state) {
  double y_coord = LEFT_LEFT_WALL_Y_COORD; 
  for (size_t i = 0; i < LEV_ONE_END; i++) {
    double width = generate_random_width();
    double height = LEVEL_ONE_END_WALLS_HEIGHT;
    double x_coord = FIRST_LEVEL_END_X_COORD; 
    vector_t top_left = {.x = x_coord, .y = y_coord};
    add_thorny_wall(state, top_left, width, height);
    y_coord += height;
  }
}


void level_two_thorny_base(state_t *state) {
  double x_coord = FIRST_LEVEL_END_X_COORD; 
  for (size_t i = 0; i < LEVEL_TWO_BOTTOM_WALLS; i++) {
    int width = WALL_WIDTH;
    int height = generate_random_height();
    double y_coord = WINDOW.y + LEVEL_ONE_CEILING_PLATFORMS_Y_COORD;
    vector_t bottom_left = {.x = x_coord, .y = y_coord};
    add_thorny_wall(state, bottom_left, width, height);
    x_coord += width; 
  }
}

void level_two_thorny_base_base(state_t *state) {
  double x_coord = FIRST_LEVEL_END_X_COORD; 
  for (size_t i = 0; i < LEVEL_TWO_BOTTOM_WALLS; i++) {
    int width = WALL_WIDTH;
    int height = LEFT_WALL_HEIGHT;
    double y_coord = WINDOW.y + LEVEL_ONE_CEILING_PLATFORMS_Y_COORD;
    vector_t bottom_left = {.x = x_coord, .y = y_coord - LEFT_WALL_HEIGHT};
    add_thorny_wall(state, bottom_left, width, height);
    x_coord += width; 
  }
}

void level_two_platforms(state_t *state){ // add checkpoint on first platform of level 2
  scene_t *scene = state->scene;
  list_t *store_walls = list_init(10, free);
  double spacing = 300;
  double x_coord = 7000; 
  int interval = 3;
  list_t *base_platforms = list_init(LEVEL_TWO_PLATFORM_WALLS, NULL);
  for (size_t i = 0; i < LEVEL_TWO_PLATFORM_WALLS; i++) {
    int width = generate_random_width();
    double height = 60;
    vector_t bottom_left = {.x = x_coord + spacing, .y = WINDOW.y + 1000};    
    if ((rand() % interval) == 1 && i != 0) {
      add_thorny_wall(state, bottom_left, width, height);
      body_t *latest_wall = scene_get_body(scene, scene_bodies(scene) - 1);
      list_add(base_platforms, latest_wall);
    } else {
      add_normal_wall(state, bottom_left, width, height);
      body_t *latest_wall = scene_get_body(scene, scene_bodies(scene) - 1);
      list_add(base_platforms, latest_wall);

      body_t *curr_wall = scene_get_body(scene, scene_bodies(scene) - 1);
      list_add(store_walls, curr_wall);
    }
    x_coord += width + spacing;
  }
  
  vector_t checkpoint_spawn = body_get_centroid(list_get(store_walls, 0));
  make_checkpoint(state, (vector_t){.x = checkpoint_spawn.x, checkpoint_spawn.y}); 

  vector_t spawn_point = body_get_centroid(list_get(store_walls, 4));
  spawn_point = (vector_t) {.x = spawn_point.x, .y = spawn_point.y + 50};

  make_pickup(scene, spawn_point);

  for(size_t i = 0; i < list_size(base_platforms); i ++){
    body_t *curr_body = list_get(base_platforms, i);
    if(get_type(curr_body) == THORNY_WALL){
      int width = generate_random_width();
      double height = 50;
      vector_t centroid = body_get_centroid(curr_body);   
      vector_t bottom_left = {.x = centroid.x - height, .y = centroid.y + 300};
      add_normal_wall(state, bottom_left, width, height);
    }
  }
  list_free(base_platforms);
}

void second_level_end(state_t *state) {
  list_t *store_walls = list_init(25, free);
  double y_coord = -700; 
  for (size_t i = 0; i < LEV_TWO_END; i++) {
    double width = generate_random_width();
    double height = LEVEL_TWO_END_WALL_HEIGHT;
    double x_right = 15500; 
    vector_t bottom_left = {.x = x_right, .y = y_coord};
    add_normal_wall(state, bottom_left, width, height);
    y_coord += height;
    body_t *curr_wall = scene_get_body(state->scene, scene_bodies(state->scene) - 1);
    list_add(store_walls, curr_wall);
  }
  vector_t checkpoint_spawn = body_get_centroid(list_get(store_walls, 24));
  make_checkpoint(state, (vector_t){.x = checkpoint_spawn.x + 150, checkpoint_spawn.y - 150});
}

void second_level_end_checkpoint_platform(state_t *state){
  double width = WALL_WIDTH;
  double height = WALL_HEIGHT;
  vector_t bottom_left = {.x = 15700, .y = 1600};
  add_normal_wall(state, bottom_left, width, height);
}

void make_downward_tunnel(state_t *state){
  list_t *store_walls = list_init(11, free);
  double y_coord = 100.0; 
  for (size_t i = 0; i < DOWNWARD_TUNNEL; i++) {
    double width = generate_random_width();
    double height = generate_random_height();
    double x_coord = 16200; 
    vector_t top_left = {.x = x_coord, .y = y_coord};
    add_normal_wall(state, top_left, width, height);
    y_coord += height;
    body_t *curr_wall = scene_get_body(state->scene, scene_bodies(state->scene) -1);
    list_add(store_walls, curr_wall);
  }
  vector_t checkpoint_spawn = body_get_centroid(list_get(store_walls, 3));
  make_pickup(state->scene, (vector_t){.x = checkpoint_spawn.x - 150, checkpoint_spawn.y});
}

void make_downward_tunnel_obstacles(state_t *state){
  double spacing = 200;
  double y_coord = 1400; 
  for (size_t i = 0; i < DOWNWARD_TUNNEL_OBSTACLES; i++) {
    double width = LEVEL_THREE_WALL_WIDTH;
    double height = WALL_HEIGHT;
    double x_right = rand() % (16100 - 15600) + 15600;
    vector_t top_left = {.x = x_right, .y = y_coord - spacing};
    add_thorny_wall(state, top_left, width, width);
    y_coord -= height + spacing;  // Update the total width for the downward tunnel
  }
}

void level_three_bottom_walls(state_t *state) {
  double bottom_total_width = 15500; 
  double y_coord = -700;
  for (size_t i = 0; i < LEVEL_THREE_BOTTOM_WALLS; i++) {
    int width = generate_random_width();
    int height = generate_random_height();
    vector_t bottom_left = {.x = bottom_total_width, .y = y_coord};
    add_thorny_wall(state, bottom_left, width, height);
    bottom_total_width += width; 
  } 
}

void level_three_top_walls(state_t *state) {
  double x_coord = 16200; 
  double y_coord = -60;
  for (size_t i = 0; i < LEVEL_TWO_BOTTOM_WALLS; i++) {
    int width = generate_random_width();
    int height = generate_random_height();
    vector_t bottom_left = {.x = x_coord, .y = y_coord};
    add_thorny_wall(state, bottom_left, width, height);
    x_coord += width; 
  } 
}

void level_three_horizontal_platforms(state_t *state){
  scene_t *scene = state->scene;
  list_t *store_walls = list_init(13, free);
  double spacing = 400;
  double x_coord = 16050; 
  for (size_t i = 0; i < LEVEL_THREE_HORIZONTAL_PLATFORMS; i++) {
    double width = LEVEL_THREE_WALL_WIDTH;
    double height = WALL_HEIGHT;
    double y_coord = -300;
    vector_t bottom_left = {.x = x_coord + spacing, .y = y_coord};
    add_normal_wall(state, bottom_left, width, height);
    x_coord += height + spacing;

    body_t *curr_wall = scene_get_body(scene, scene_bodies(scene) - 1);
    list_add(store_walls, curr_wall);
  }
  vector_t spawn_point = body_get_centroid(list_get(store_walls, 3));
  spawn_point = (vector_t) {.x = spawn_point.x, .y = spawn_point.y + 50};

  make_pickup(scene, spawn_point);
}

void end_of_game_platform(state_t *state){
   double x_coord = 23175;
   double y_coord = -250;
   vector_t bottom_left = {.x = x_coord, .y = y_coord}; 
   add_winning_wall(state, bottom_left, END_OF_GAME_RADIUS);
}


void initialize_level_design(state_t *state) {
  // add_normal_wall(state, (vector_t) {.x = 200, .y = 200}, 140, 100);
  // add_normal_wall(state, (vector_t) {.x = 340, .y = 280}, 100, 120);
  add_bottom_cushion_thorny(state);
  add_top_cushion_thorny(state);
  level_one_bottom_walls(state);
  level_one_bottom_bottom_walls(state);
  level_one_top_walls(state);
  init_left_left_wall(state);
  init_left_wall(state);
  level_one_base_platforms(state);
  level_one_ceiling_platforms(state);
  make_upward_tunnel(state);
  first_level_end(state);
  level_two_thorny_base(state);
  level_two_thorny_base_base(state);
  level_two_platforms(state);
  second_level_end(state);
  second_level_end_checkpoint_platform(state);
  make_downward_tunnel(state);
  make_downward_tunnel_obstacles(state);
  level_three_bottom_walls(state);
  level_three_top_walls(state);
  level_three_horizontal_platforms(state);
  end_of_game_platform(state);
}

void enforce_freeze(scene_t *scene) {
  body_t *slime = scene_get_body(scene, 0);
  if (is_frozen(slime)) {
    body_set_centroid(slime, ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->frozen_pos);
    body_set_velocity(slime, VEC_ZERO);
  }
}

void update_line(scene_t *scene, list_t *line_points) {
  body_t *slime = scene_get_body(scene, 0);
  body_t *hook = get_hook(slime);
  vector_t slime_cen = body_get_centroid(slime);
  vector_t hook_cen = body_get_centroid(hook);
  vector_t direction = vec_subtract(slime_cen, hook_cen);
  vector_t perp_dir = {.x = -direction.y, .y = direction.x};
  perp_dir = vec_multiply(LINE_WIDTH, vec_normalized(perp_dir));
  *(vector_t *)list_get(line_points, 0) = vec_subtract(slime_cen, perp_dir);
  *(vector_t *)list_get(line_points, 1) = vec_add(slime_cen, perp_dir);
  *(vector_t *)list_get(line_points, 2) = vec_add(hook_cen, perp_dir);
  *(vector_t *)list_get(line_points, 3) = vec_subtract(hook_cen, perp_dir);
}

body_t *initialize_line(scene_t *scene) {
  list_t *line_points = list_init(4, free);
  for (size_t i = 0; i < 4; i++) {
    vector_t *v = malloc(sizeof(vector_t));
    *v = VEC_ZERO;
    list_add(line_points, v);
  }
  update_line(scene, line_points);
  object_info_t *info = malloc(sizeof(object_info_t));
  info->body_type = LINE;
  info->info = NULL;
  info->info_freeer = NULL;
  body_t *line = body_init_with_info(line_points, INFINITY, LINE_COLOR, info, (free_func_t) object_info_freeer);
  return line;
}

body_t *initialize_hook(list_t *points) {
  object_info_t *hook_info = malloc(sizeof(object_info_t));
  hook_info->body_type = HOOK;
  hook_info->info = NULL;
  hook_info->info_freeer = NULL;
  body_t *hook = body_init_with_info(points, INFINITY, SUPER_INVISIBLE, hook_info, (free_func_t) object_info_freeer);
  set_circle(hook, HOOK_RADIUS);
  body_set_img(hook, get_image_from_path("assets/hook.png"));
  return hook;
}

void slime_grapple(state_t *state, vector_t mouse_pos) {
  scene_t *scene = state->scene;
  body_t *slime = scene_get_body(scene, 0);
  grappleSoundEffects();
  unfreeze(slime);
  unhook(scene_get_body(scene, 0));
  vector_t slime_pos = body_get_centroid(slime);
  vector_t direction = vec_subtract(mouse_pos, slime_pos);
  direction = vec_normalized(direction);
  list_t *hook_points = make_circle(vec_add(slime_pos, vec_multiply(get_radius(slime) + HOOK_RADIUS, direction)), HOOK_RADIUS, NUM_POINTS);
  body_t *hook = initialize_hook(hook_points);
  scene_add_body(state->scene, hook);
  body_set_velocity(hook, vec_multiply(HOOK_VEL, direction));
  set_hook(slime, hook);
  body_t *line = initialize_line(scene);
  scene_add_body(state->scene, line);
  set_line(slime, line);
  // loop through all walls to create hook and line collisions
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *body = scene_get_body(scene, i);
    if (get_type(body) == REGULAR_WALL) {
      create_hook_collision(scene, hook, body);
      create_unhook_collision(scene, line, body);
    }

    if (get_type(body) == THORNY_WALL) {
      create_unhook_collision(scene, line, body);
      create_unhook_collision(scene, hook, body);
    }
  }
}

void limit_hook(scene_t *scene, body_t *hook) {
  vector_t hook_loc = body_get_centroid(hook);
  if (hook_loc.x > WINDOW.x + HOOK_FURTHER_X || hook_loc.x < -HOOK_FURTHER_X 
  || hook_loc.y > WINDOW.y + HOOK_FURTHER_Y || hook_loc.y < -HOOK_FURTHER_Y) {
    unhook(scene_get_body(scene, 0));
  }
}

void hook_line_updater(scene_t *scene) {
  body_t *slime = scene_get_body(scene, 0);
  body_t *hook = get_hook(slime);
  body_t *line = get_line(slime);
  if (hook != NULL && !body_is_removed(hook)) {
    if (line != NULL && !body_is_removed(line)) {
      update_line(scene, body_get_shape_shallow(line));
    }
    limit_hook(scene, hook);
  }
  
}

void on_mouse(char mouse, mouse_event_type_t type, double held_time, state_t *state) {
  if (type == MOUSE_RELEASED) {
    switch (mouse) {
      case MOUSE_LEFT: {
        vector_t mouse = sdl_mouse_pos();
        mouse.y = WINDOW.y - mouse.y;
        slime_jump(state, mouse);
        state->mouse_is_held = false;
        reset_arrows(state->scene);
        break;
      }
    }
  }
  else {
    state->mouse_is_held = true;
  }
}

void shift_scene(scene_t *scene, vector_t camera_shift) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *body = scene_get_body(scene, i);
    if (get_type(body) != BORDER) {
      body_set_centroid(body, vec_subtract(body_get_centroid(body), camera_shift));
    }
  }
  body_t *slime = scene_get_body(scene, 0);
  if (is_frozen(slime)) {
    freeze(slime); //updates frozen_pos
  }
}

void shift_background(state_t *state, vector_t camera_shift) {
  vector_t b1_shift = vec_multiply(PARALLAX_MULTIPLIER_1, camera_shift);
  for (size_t i = 0; i < list_size(state->backgrounds_1); i++) {
    body_t *background = list_get(state->backgrounds_1, i);
    body_set_centroid(background, vec_subtract(body_get_centroid(background), b1_shift));
  }
  vector_t b2_shift = vec_multiply(PARALLAX_MULTIPLIER_2, camera_shift);
  for (size_t i = 0; i < list_size(state->backgrounds_2); i++) {
    body_t *background = list_get(state->backgrounds_2, i);
    body_set_centroid(background, vec_subtract(body_get_centroid(background), b2_shift));
  }
  vector_t b3_shift = vec_multiply(PARALLAX_MULTIPLIER_3, camera_shift);
  for (size_t i = 0; i < list_size(state->backgrounds_3); i++) {
    body_t *background = list_get(state->backgrounds_3, i);
    body_set_centroid(background, vec_subtract(body_get_centroid(background), b3_shift));
  }
}

void shift_camera(state_t *state) {
  // if (is_dead(state->scene)) return;
  vector_t slime_pos = body_get_centroid(scene_get_body(state->scene, 0));
  vector_t new_shift = vec_subtract(slime_pos, SLIME_SPAWN);
  if (new_shift.x != 0 || new_shift.y != 0) {
    state->shift = vec_add(state->shift, new_shift);
    shift_scene(state->scene, new_shift);
    shift_background(state, new_shift);
  }
}

void create_background(state_t *state) {
  state->backgrounds_1 = list_init(NUM_BX_1*NUM_BY_1, (free_func_t) body_free);
  vector_t bottom_left = VEC_ZERO;
  for (size_t i = 0; i < NUM_BX_1; i++) {
    for (size_t j = 0; j < NUM_BY_1; j++) {
      bottom_left.x = i*BACKGROUND_X_1;
      bottom_left.y = -2*BACKGROUND_Y_1 + j*BACKGROUND_Y_1;
      list_t *b1 = make_rect(bottom_left, BACKGROUND_X_1, BACKGROUND_Y_1);
      object_info_t *b1_info = malloc(sizeof(object_info_t));
      b1_info->body_type = BACKGROUND;
      b1_info->info = NULL;
      b1_info->info_freeer = NULL;
      body_t *background1 = body_init_with_info(b1, INFINITY, SUPER_INVISIBLE, b1_info, (free_func_t) object_info_freeer);
      body_set_img(background1, state->bimg_1);
      set_rect(background1, BACKGROUND_X_1, BACKGROUND_Y_1);
      list_add(state->backgrounds_1, background1);
    }
  }

  state->backgrounds_2 = list_init(NUM_BX_2*NUM_BY_2, (free_func_t) body_free);
  bottom_left = VEC_ZERO;
  bottom_left.y -= 2*BACKGROUND_Y_2;
  for (size_t i = 0; i < NUM_BX_2; i++) {
    for (size_t j = 0; j < NUM_BY_2; j++) {
      bottom_left.x = i*BACKGROUND_X_2;
      bottom_left.y = -2*BACKGROUND_Y_2 + j*BACKGROUND_Y_2;
      list_t *b1 = make_rect(bottom_left, BACKGROUND_X_2, BACKGROUND_Y_2);
      object_info_t *b1_info = malloc(sizeof(object_info_t));
      b1_info->body_type = BACKGROUND;
      b1_info->info = NULL;
      b1_info->info_freeer = NULL;
      body_t *background1 = body_init_with_info(b1, INFINITY, SUPER_INVISIBLE, b1_info, (free_func_t) object_info_freeer);
      body_set_img(background1, state->bimg_2);
      set_rect(background1, BACKGROUND_X_2, BACKGROUND_Y_2);
      list_add(state->backgrounds_2, background1);
    }
  }

  state->backgrounds_3 = list_init(NUM_BX_3*NUM_BY_3, (free_func_t) body_free);
  bottom_left = VEC_ZERO;
  bottom_left.y -= 2*BACKGROUND_Y_3;
  for (size_t i = 0; i < NUM_BX_3; i++) {
    for (size_t j = 0; j < NUM_BY_3; j++) {
      bottom_left.x = i*BACKGROUND_X_3;
      bottom_left.y = -2*BACKGROUND_Y_3 + j*BACKGROUND_Y_3;
      list_t *b1 = make_rect(bottom_left, BACKGROUND_X_3, BACKGROUND_Y_3);
      object_info_t *b1_info = malloc(sizeof(object_info_t));
      b1_info->body_type = BACKGROUND;
      b1_info->info = NULL;
      b1_info->info_freeer = NULL;
      body_t *background1 = body_init_with_info(b1, INFINITY, SUPER_INVISIBLE, b1_info, (free_func_t) object_info_freeer);
      body_set_img(background1, state->bimg_3);
      set_rect(background1, BACKGROUND_X_3, BACKGROUND_Y_3);
      list_add(state->backgrounds_3, background1);
    }
  }
}

void add_background(state_t *state) {
  for (size_t i = 0; i < list_size(state->backgrounds_1); i++) {
    scene_add_body_front(state->scene, list_get(state->backgrounds_1, i));
  }
  for (size_t i = 0; i < list_size(state->backgrounds_2); i++) {
    scene_add_body_front(state->scene, list_get(state->backgrounds_2, i));
  }
  for (size_t i = 0; i < list_size(state->backgrounds_3); i++) {
    scene_add_body_front(state->scene, list_get(state->backgrounds_3, i));
  }
}

void remove_background(state_t *state) {
  for (size_t i = 0; i < list_size(state->backgrounds_1); i++) {
    scene_remove_body_override(state->scene, 0);
  }
  for (size_t i = 0; i < list_size(state->backgrounds_2); i++) {
    scene_remove_body_override(state->scene, 0);
  }
  for (size_t i = 0; i < list_size(state->backgrounds_3); i++) {
    scene_remove_body_override(state->scene, 0);
  }
}

void create_invisible_walls(state_t *state) {
  list_t *wall1_points = make_rect((vector_t){.x = -BORDER_LEN, .y = 0}, BORDER_LEN, WINDOW.y);
  object_info_t *w1_info = malloc(sizeof(object_info_t));
  w1_info->body_type = BORDER;
  w1_info->info = NULL;
  w1_info->info_freeer = NULL;
  body_t *wall1 = body_init_with_info(wall1_points, INFINITY, INVISIBLE, w1_info, (free_func_t) object_info_freeer);
  body_set_img(wall1, get_image_from_path("assets/white.jpg"));
  set_rect(wall1, BORDER_LEN, WINDOW.y);
  state->invis_wall1 = wall1;
  list_t *wall2_points = make_rect((vector_t){.x = WINDOW.x, .y = 0}, BORDER_LEN, WINDOW.y);
  object_info_t *w2_info = malloc(sizeof(object_info_t));
  w2_info->body_type = BORDER;
  w2_info->info = NULL;
  w2_info->info_freeer = NULL;
  body_t *wall2 = body_init_with_info(wall2_points, INFINITY, INVISIBLE, w2_info, (free_func_t) object_info_freeer);
  body_set_img(wall2, get_image_from_path("assets/white.jpg"));
  set_rect(wall2, BORDER_LEN, WINDOW.y);
  state->invis_wall2 = wall2;
}

void add_invisible_walls(state_t *state) {
  scene_t *scene = state->scene;
  scene_add_body(scene, state->invis_wall1);
  scene_add_body(scene, state->invis_wall2);
}

void remove_invisible_walls(state_t *state) {
  scene_t *scene = state->scene;
  for (size_t i = 0; i < 2; i++) {
    scene_remove_body_override(scene, scene_bodies(scene) - 1); 
  }
}

void initialize_scene(state_t *state) {
  scene_t *scene = state->scene;
  body_t *slime = initialize_slime(state);
  scene_add_body(scene, slime);
  create_downward_gravity(scene, GRAVITY, slime);
  initialize_arrows(scene);
  initialize_level_design(state);
}

void reset_images(scene_t *scene) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *body = scene_get_body(scene, i);
    if (get_type(body) == SLIME_PICKUP || get_type(body) == HOOK || get_type(body) == LINE || get_type(body) == CHECK_POINT || get_type(body) == ARROW){
      remove_img_from_body(body);
    }
  }
}

void clear_scene(scene_t *scene) {
  reset_images(scene);
  // reset_arrows(scene);
  scene_free(scene);
}

void reset_background_pos(state_t *state) {
  size_t count = 0;
  for (size_t i = 0; i < NUM_BX_1; i++) {
    for(size_t j = 0; j < NUM_BY_1; j++) {
      body_t *background = list_get(state->backgrounds_1, count);
      count += 1;
      body_set_centroid(background, (vector_t){ .x = i*BACKGROUND_X_1 + BACKGROUND_X_1/2, .y = j*BACKGROUND_Y_1 + BACKGROUND_Y_1/2 - 2*BACKGROUND_Y_1});
    }
  }
  count = 0;
  for (size_t i = 0; i < NUM_BX_2; i++) {
    for(size_t j = 0; j < NUM_BY_2; j++) {
      body_t *background = list_get(state->backgrounds_2, count);
      count += 1;
      body_set_centroid(background, (vector_t){ .x = i*BACKGROUND_X_2 + BACKGROUND_X_2/2, .y = j*BACKGROUND_Y_2 + BACKGROUND_Y_2/2 - 2*BACKGROUND_Y_2});
    }
  }
  count = 0;
  for (size_t i = 0; i < NUM_BX_3; i++) {
    for(size_t j = 0; j < NUM_BY_3; j++) {
      body_t *background = list_get(state->backgrounds_3, count);
      count += 1;
      body_set_centroid(background, (vector_t){ .x = i*BACKGROUND_X_3 + BACKGROUND_X_3/2, .y = j*BACKGROUND_Y_3 + BACKGROUND_Y_3/2 - 2*BACKGROUND_Y_3});
    }
  }
}

void restart(state_t *state) {
  state->dying = -1;
  state->respawn = 0;
  state->mouse_is_held = false;
  state->shift = VEC_ZERO;
  state->checkpoints = list_init(2, NULL);
  state->jump_timer = JUMP_RESET;

  state->scene = scene_init();
  initialize_scene(state);
  reset_background_pos(state);
  shift_camera(state);
}

void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  if (type == KEY_PRESSED) {
    switch (key) {
      case ' ': {
        vector_t mouse = sdl_mouse_pos();
        mouse.y = WINDOW.y - mouse.y;
        slime_grapple(state, mouse);
        break;
      }
      case 'r':
      case 'R':
      {
        clear_scene(state->scene);
        restart(state);
        break;
      }
      case 'e':
      case 'E':
      {
        emscripten_free(state);
        exit(0);
        break;
      }
      case 'l':
      case 'L':
      {
        state->won = 0;
        break;
      }
    }
  }
}

void update_dist(body_t *slime) {
  vector_t curr = body_get_centroid(slime);
  vector_t old = get_old_pos(slime);
  double movement = vec_magnitude(vec_subtract(curr, old));
  set_dist(slime, get_dist(slime) + movement);
}

void make_animator(state_t *state) {
  scene_t *scene = state->scene;
  list_t *wall_points = make_rect(VEC_ZERO, WINDOW.x, WINDOW.y);
  object_info_t *w_info = malloc(sizeof(object_info_t));
  w_info->body_type = ANIMATOR;
  w_info->info = NULL;
  w_info->info_freeer = NULL;
  body_t *wall = body_init_with_info(wall_points, INFINITY, SUPER_INVISIBLE, w_info, (free_func_t) object_info_freeer);
  set_rect(wall, WINDOW.x, WINDOW.y);
  scene_add_body(scene, wall);
}

void update_slime_loss(state_t *state) {
  body_t *slime = scene_get_body(state->scene, 0);
  if (get_dist(slime) >= DECREASE_DIST) {
    set_dist(slime, get_dist(slime) - DECREASE_DIST);
    set_radius(slime, get_radius(slime)*SLIME_RADIUS_LOSS_FACTOR);
    update_circle_radius(body_get_shape_shallow(slime), body_get_centroid(slime), get_radius(slime));
    body_set_mass(slime, body_get_mass(slime) * SLIME_MASS_LOSS_FACTOR);
  }
  if (get_radius(slime) < DEATH_RADIUS) {
    state->dying = 0;
    remove_img_from_body(slime);
  }
}

void update_checkpoints(state_t *state) {
  for (size_t i = 0; i < list_size(state->checkpoints); i++) {
    body_t *checkpoint = list_get(state->checkpoints, i);
    if (*(size_t *)list_get(state->checkpoint_statuses, get_num(checkpoint)) <= CHECKPOINT_FRAMES - 1 && *(size_t *)list_get(state->checkpoint_statuses, get_num(checkpoint)) > 0) {
      remove_img_from_body(checkpoint);
      body_set_img(checkpoint, get_image_from_path(list_get(state->checkpoint_animation, *(size_t *)list_get(state->checkpoint_statuses, get_num(checkpoint)))));
      size_t *status = list_get(state->checkpoint_statuses, get_num(checkpoint));
      *status = *status + 1;
      struct timespec ts;
      ts.tv_nsec = CHECKPOINT_ANIMATION_PAUSE*pow(10, 9);
      ts.tv_sec = 0;
      nanosleep(&ts, &ts);
    }
    else if (*(size_t *)list_get(state->checkpoint_statuses, get_num(checkpoint)) == CHECKPOINT_FRAMES) {
      remove_img_from_body(checkpoint);
      body_set_img(checkpoint, get_image_from_path(list_get(state->checkpoint_animation, *(size_t *)list_get(state->checkpoint_statuses, get_num(checkpoint)) - 1)));
    }
  }
}

void render_all(state_t *state) {
  add_background(state);
  add_invisible_walls(state);
  sdl_render_scene(state->scene);
  remove_invisible_walls(state);
  remove_background(state);
}

void pause(double seconds) {
  struct timespec ts;
    ts.tv_nsec = seconds*pow(10, 9);
    ts.tv_sec = 0;
    nanosleep(&ts, &ts);
}

void populate_all_animations(state_t *state) {
  state->opening_animation = list_init(OPENING_ANIMATION_FRAMES, free);
  list_add(state->opening_animation, "assets/onhand/1.png");
  list_add(state->opening_animation, "assets/onhand/2.png");
  list_add(state->opening_animation, "assets/onhand/3.png");
  list_add(state->opening_animation, "assets/onhand/4.png");
  list_add(state->opening_animation, "assets/onhand/5.png");
  list_add(state->opening_animation, "assets/onhand/6.png");
  list_add(state->opening_animation, "assets/onhand/7.png");
  list_add(state->opening_animation, "assets/onhand/8.png");
  list_add(state->opening_animation, "assets/onhand/9.png");
  list_add(state->opening_animation, "assets/onhand/10.png");
  list_add(state->opening_animation, "assets/onhand/11.png");
  list_add(state->opening_animation, "assets/onhand/12.png");
  list_add(state->opening_animation, "assets/onhand/13.png");
  list_add(state->opening_animation, "assets/onhand/14.png");
  list_add(state->opening_animation, "assets/onhand/15.png");
  list_add(state->opening_animation, "assets/onhand/16.png");
  list_add(state->opening_animation, "assets/onhand/17.png");
  list_add(state->opening_animation, "assets/onhand/18.png");
  list_add(state->opening_animation, "assets/onhand/19.png");
  list_add(state->opening_animation, "assets/onhand/20.png");
  list_add(state->opening_animation, "assets/onhand/21.png");
  list_add(state->opening_animation, "assets/onhand/22.png");
  list_add(state->opening_animation, "assets/onhand/23.png");
  list_add(state->opening_animation, "assets/onhand/24.png");
  list_add(state->opening_animation, "assets/onhand/25.png");
  list_add(state->opening_animation, "assets/onhand/26.png");
  list_add(state->opening_animation, "assets/onhand/27.png");
  list_add(state->opening_animation, "assets/onhand/28.png");
  list_add(state->opening_animation, "assets/onhand/29.png");
  list_add(state->opening_animation, "assets/fall/0.png");
  list_add(state->opening_animation, "assets/fall/1.png");
  list_add(state->opening_animation, "assets/fall/2.png");
  list_add(state->opening_animation, "assets/fall/3.png");
  list_add(state->opening_animation, "assets/fall/4.png");
  list_add(state->opening_animation, "assets/fall/5.png");
  list_add(state->opening_animation, "assets/fall/6.png");
  list_add(state->opening_animation, "assets/fall/7.png");
  list_add(state->opening_animation, "assets/fall/8.png");
  list_add(state->opening_animation, "assets/fall/9.png");
  list_add(state->opening_animation, "assets/intosewer/0.png");
  list_add(state->opening_animation, "assets/intosewer/1.png");
  list_add(state->opening_animation, "assets/intosewer/2.png");
  list_add(state->opening_animation, "assets/intosewer/3.png");
  list_add(state->opening_animation, "assets/intosewer/4.png");
  list_add(state->opening_animation, "assets/intosewer/5.png");
  list_add(state->opening_animation, "assets/intosewer/6.png");
  list_add(state->opening_animation, "assets/intosewer/7.png");
  list_add(state->opening_animation, "assets/intosewer/8.png");
  list_add(state->opening_animation, "assets/intosewer/9.png");
  list_add(state->opening_animation, "assets/intosewer/10.png");
  list_add(state->opening_animation, "assets/intosewer/11.png");
  list_add(state->opening_animation, "assets/intosewer/12.png");
  list_add(state->opening_animation, "assets/intosewer/13.png");
  list_add(state->opening_animation, "assets/intosewer/14.png");
  list_add(state->opening_animation, "assets/intosewer/15.png");
  list_add(state->opening_animation, "assets/intosewer/16.png");

  state->won_animation = list_init(WIN_FRAMES, free);
  list_add(state->won_animation, "assets/sewer/0.png");
  list_add(state->won_animation, "assets/sewer/1.png");
  list_add(state->won_animation, "assets/sewer/2.png");
  list_add(state->won_animation, "assets/sewer/3.png");
  list_add(state->won_animation, "assets/sewer/4.png");
  list_add(state->won_animation, "assets/sewer/5.png");
  list_add(state->won_animation, "assets/sewer/6.png");
  list_add(state->won_animation, "assets/sewer/7.png");
  list_add(state->won_animation, "assets/sewer/8.png");
  list_add(state->won_animation, "assets/sewer/9.png");
  list_add(state->won_animation, "assets/sewer/10.png");
  list_add(state->won_animation, "assets/sewer/11.png");
  list_add(state->won_animation, "assets/sewer/12.png");
  list_add(state->won_animation, "assets/sewer/13.png");
  list_add(state->won_animation, "assets/sewer/14.png");
  list_add(state->won_animation, "assets/sewer/15.png");
  list_add(state->won_animation, "assets/sewer/16.png");
  list_add(state->won_animation, "assets/sewer/17.png");
  list_add(state->won_animation, "assets/sewer/18.png");
  list_add(state->won_animation, "assets/sewer/19.png");
  list_add(state->won_animation, "assets/sewer/20.png");
  list_add(state->won_animation, "assets/exit/0.png");
  list_add(state->won_animation, "assets/exit/1.png");
  list_add(state->won_animation, "assets/exit/2.png");
  list_add(state->won_animation, "assets/exit/3.png");
  list_add(state->won_animation, "assets/exit/4.png");
  list_add(state->won_animation, "assets/exit/5.png");
  list_add(state->won_animation, "assets/exit/6.png");
  list_add(state->won_animation, "assets/exit/7.png");
  list_add(state->won_animation, "assets/exit/8.png");
  list_add(state->won_animation, "assets/exit/9.png");
  list_add(state->won_animation, "assets/exit/10.png");
  list_add(state->won_animation, "assets/exit/11.png");
  list_add(state->won_animation, "assets/exit/12.png");
  list_add(state->won_animation, "assets/exit/13.png");
  list_add(state->won_animation, "assets/exit/14.png");
  list_add(state->won_animation, "assets/exit/15.png");
  list_add(state->won_animation, "assets/pet/0.png");
  list_add(state->won_animation, "assets/pet/1.png");
  list_add(state->won_animation, "assets/pet/2.png");
  list_add(state->won_animation, "assets/pet/3.png");
  list_add(state->won_animation, "assets/pet/4.png");
  list_add(state->won_animation, "assets/pet/5.png");
  list_add(state->won_animation, "assets/pet/6.png");
  list_add(state->won_animation, "assets/pet/7.png");
  list_add(state->won_animation, "assets/pet/8.png");
  list_add(state->won_animation, "assets/pet/9.png");
  list_add(state->won_animation, "assets/pet/10.png");
  list_add(state->won_animation, "assets/pet/11.png");
  list_add(state->won_animation, "assets/pet/12.png");
  list_add(state->won_animation, "assets/pet/13.png");
  list_add(state->won_animation, "assets/pet/14.png");
  list_add(state->won_animation, "assets/pet/15.png");
  list_add(state->won_animation, "assets/pet/16.png");
  list_add(state->won_animation, "assets/pet/17.png");
  list_add(state->won_animation, "assets/pet/18.png");
  list_add(state->won_animation, "assets/pet/19.png");
  list_add(state->won_animation, "assets/pet/20.png");
  list_add(state->won_animation, "assets/pet/21.png");
  list_add(state->won_animation, "assets/pet/22.png");
  list_add(state->won_animation, "assets/pet/23.png");
  list_add(state->won_animation, "assets/pet/24.png");
  list_add(state->won_animation, "assets/pet/25.png");

  state->checkpoint_animation = list_init(CHECKPOINT_FRAMES, free);
  list_add(state->checkpoint_animation, "assets/checkpoint/0.png");
  list_add(state->checkpoint_animation, "assets/checkpoint/1.png");
  list_add(state->checkpoint_animation, "assets/checkpoint/2.png");
  list_add(state->checkpoint_animation, "assets/checkpoint/3.png");
  list_add(state->checkpoint_animation, "assets/checkpoint/4.png");
  list_add(state->checkpoint_animation, "assets/checkpoint/5.png");
  list_add(state->checkpoint_animation, "assets/checkpoint/6.png");
  list_add(state->checkpoint_animation, "assets/checkpoint/7.png");
  list_add(state->checkpoint_animation, "assets/checkpoint/8.png");
  list_add(state->checkpoint_animation, "assets/checkpoint/9.png");
  list_add(state->checkpoint_animation, "assets/checkpoint/10.png");
  list_add(state->checkpoint_animation, "assets/checkpoint/11.png");

  state->animation = list_init(ANIMATION_FRAMES, free);
  list_add(state->animation, "assets/death1.png");
  list_add(state->animation, "assets/death2.png");
  list_add(state->animation, "assets/death3.png");
  list_add(state->animation, "assets/death4.png");
  list_add(state->animation, "assets/death5.png");
  list_add(state->animation, "assets/death6.png");
  list_add(state->animation, "assets/death7.png");
  list_add(state->animation, "assets/death8.png");
  list_add(state->animation, "assets/death9.png");
  list_add(state->animation, "assets/death10.png");
  list_add(state->animation, "assets/death11.png");
  list_add(state->animation, "assets/death12.png");
  list_add(state->animation, "assets/death13.png");
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);

  sdl_init(VEC_ZERO, WINDOW);
  create_invisible_walls(state);
  state->bimg_1 = get_image_from_path("assets/background1.png");
  state->bimg_2 = get_image_from_path("assets/background2.png");
  state->bimg_3 = get_image_from_path("assets/background3.png");
  create_background(state);

  state->spawn_point = SLIME_SPAWN;
  state->spawn_shift = VEC_ZERO;
  state->spawn_radius = INITIAL_SLIME_RADIUS;
  state->spawn_mass = SLIME_MASS;
  state->wall_img = get_image_from_path("assets/wall.png");
  state->thorny_img = get_image_from_path("assets/thorny.png");
  state->final_pipe = get_image_from_path("assets/final.png");

  populate_all_animations(state);
  state->won = -1;
  state->opening = 0;

  state->checkpoint_statuses = list_init(NUM_CHECKPOINTS, free);
  for (size_t i = 0; i < NUM_CHECKPOINTS; i++) {
    size_t *status = malloc(sizeof(size_t));
    *status = 0;
    list_add(state->checkpoint_statuses, status);
  }

  restart(state);

  sdl_on_click((mouse_handler_t) on_mouse);
  sdl_on_key((key_handler_t) on_key);

  printf("%s\n", instructions);
  return state;
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  body_t *slime = scene_get_body(state->scene, 0);
  if (dt > MAX_DT) {
    dt = MAX_DT;
  }
  state->jump_timer += dt;
  if (state->opening > OPENING_ANIMATION_FRAMES) {
    if (state->won >= 0) {
      if (state->won >= WIN_FRAMES) {
        emscripten_cancel_main_loop();
        emscripten_force_exit(0);
      }
      else {
        if (state->won == 0) {
          make_animator(state);
          freeMusic(); 
          sdl_on_key(NULL);
          sdl_on_click(NULL);
        }
        body_t *animator = scene_get_body(state->scene, scene_bodies(state->scene) - 1);
        remove_img_from_body(animator);
        body_set_img(animator, get_image_from_path(list_get(state->won_animation, state->won)));
        state->won += 1;
        pause(WIN_ANIMATION_PAUSE);
      }
    }
    else {
      if (state->dying == -1) {
        update_checkpoints(state);
        if (state->respawn < ANIMATION_FRAMES) {
          remove_img_from_body(slime);
          body_set_img(slime, get_image_from_path(list_get(state->animation, ANIMATION_FRAMES - 1 - state->respawn)));
          state->respawn += 1;

          pause(RESPAWN_ANIMATION_FRAME_PAUSE);
        }
        set_old_pos(slime, body_get_centroid(slime));
        set_old_radius(slime, get_radius(slime));
        update_slime_loss(state);
        if (state->mouse_is_held) {
          dt *= SLOW_FACTOR;
        }
        scene_tick(state->scene, dt);
      }
      if (state->dying >= 0){
        sdl_on_click(NULL);
        sdl_on_key(NULL);
        if(state->dying <= ANIMATION_FRAMES - 1){
          set_radius(slime, get_radius(slime)*DEATH_GROW_FACTOR);
          remove_img_from_body(slime);
          body_set_img(slime, get_image_from_path(list_get(state->animation, state->dying)));
          state->dying += 1;

          pause(DEATH_ANIMATION_FRAME_PAUSE);
        } 
        else{
          clear_scene(state->scene);
          restart(state);
          sdl_on_click((mouse_handler_t) on_mouse);
          sdl_on_key((key_handler_t) on_key);
          return;
        }
      }
    if (state->dying == -1){
        update_dist(scene_get_body(state->scene, 0));
        hook_line_updater(state->scene);
        enforce_freeze(state->scene);
        update_arrows(state);
        shift_camera(state);
      }
    }
  }
  else {
    if (state->opening == OPENING_ANIMATION_FRAMES) {
      body_t *animator = scene_get_body(state->scene, scene_bodies(state->scene) - 1);
      scene_remove_body_override(state->scene, scene_bodies(state->scene) - 1);
      body_free(animator);
      backgroundMusic();
      sdl_on_key(on_key);
      sdl_on_click(on_mouse);
      state->opening += 1;
    }
    else {
      if (state->opening == 0) {
        make_animator(state);
        sdl_on_key(NULL);
        sdl_on_click(NULL);
      }
      body_t *animator = scene_get_body(state->scene, scene_bodies(state->scene) - 1);
      remove_img_from_body(animator);
      body_set_img(animator, get_image_from_path(list_get(state->opening_animation, state->opening)));
      state->opening += 1;
      pause(OPENING_ANIMATION_PAUSE);
    } 
  }
  render_all(state);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  body_free(state->invis_wall1);
  body_free(state->invis_wall2);
  list_free(state->animation);
  list_free(state->checkpoint_animation);
  list_free(state->checkpoint_statuses);
  list_free(state->won_animation);
  list_free(state->opening_animation);
  list_free(state->backgrounds_1);
  list_free(state->backgrounds_2);
  list_free(state->backgrounds_3);
  sdl_destroy_texture(state->bimg_1);
  sdl_destroy_texture(state->final_pipe);
  sdl_destroy_texture(state->bimg_2);
  sdl_destroy_texture(state->bimg_3);
  sdl_destroy_texture(state->wall_img);
  sdl_destroy_texture(state->thorny_img);
  free(state);
}