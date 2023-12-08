#include "forces.h"
#include "collision.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


//top wall constants
const int TOP_WALLS = 50; 

// bottom wall constants
const int LEVEL_ONE_BOTTOM_WALLS = 80;

const int LEVEL_TWO_BOTTOM_WALLS = 69;

//left wall constants
const int LEFT_WALLS = 50;

//right wall constants
const int CHECKPOINT_WALLS = 10;

//platform wall constants
const int LEVEL_ONE_PLATFORM_WALLS = 18;
const int UPWARD_TUNNEL = 5;

const int LEVEL_TWO_PLATFORM_WALLS = 20;

const double DOWNWARD_TUNNEL_OBSTACLES = 7;

const double SECOND_CHECKPOINT_WALLS = 13;
//level three constants
const int LEVEL_THREE_BOTTOM_WALLS = 70;
const int LEVEL_THREE_PLATFORM_WALLS = 10;
const int LEVEL_THREE_TOP_WALLS = 70;
const int DOWNWARD_TUNNEL = 10;

const double MIN_WALL_WIDTH = 100.0;
const double MAX_WALL_WIDTH = 150.0;
const double MIN_WALL_HEIGHT =  100.0;
const double MAX_WALL_HEIGHT = 300.0;

const double BASE_BLOCK_X = 50;
const double BASE_BLOCK_Y = 50;

const double MAX_DT = 0.05;

const double GRAVITY = 800;

const double TWOPI = 2.0 * M_PI;

const rgb_color_t INVISIBLE = {.r = 1.0, .g = 1.0, .b = 1.0, .a = 1.0};
const double BORDER_LEN = 500;

const vector_t WINDOW = (vector_t){.x = 1200, .y = 800};
const vector_t WINDOW_CENTER = (vector_t){.x = 600, .y = 400};
const double MAX_X = 700;
const double MIN_X = 400;
const double MAX_Y = 500;
const double MIN_Y = 250;

const double HOOK_FURTHER_X = 250;
const double HOOK_FURTHER_Y = 200;

const size_t NUM_POINTS = 20;

const double SLIME_RADIUS = 50;
const double SLIME_MASS = 10;
const rgb_color_t SLIME_COLOR = {.r = 0.0, .g = 1.0, .b = 0.0, .a = 1.0};
const vector_t SLIME_SPAWN = {.x = 500, .y = 400};

const rgb_color_t REGULAR_WALL_COLOR = {.r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0};
const rgb_color_t THORNY_WALL_COLOR = {.r = 1.0, .g = 0.0, .b = 0.0, .a = 1.0};

const double HOOK_RADIUS = 20;
const rgb_color_t HOOK_COLOR = {.r = 0.0, .g = 1.0, .b = 0.0, .a = 1.0};
const double HOOK_VEL = 2000;
const double HOOK_SPRING_K = 100.0;
const double LINE_WIDTH = 10;
const rgb_color_t LINE_COLOR = {.r = 0.0, .g = 1.0, .b = 0.0, .a = 1.0};

const double MAX_JUMP_LEN = 500.0;
const double MIN_JUMP_LEN = 100.0;
const double MIN_MULT = 1.0;
const double MAX_MULT = 2.0;
const double JUMP_MULTIPLIER = 5000.0;

const double GRAPPLE_PORTION = 0.9;

//wall constants
const double WALL_HEIGHT = 100;
const double WALL_WIDTH = 200;

typedef struct state {
  scene_t *scene;
  vector_t shift;
  body_t *invis_wall1;
  body_t *invis_wall2;
} state_t;

typedef enum {
    SLIME,
    REGULAR_WALL,
    THORNY_WALL,
    HOOK,
    LINE,
    BORDER
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
} slime_info_t;

typedef struct hook_info {
  body_t *wall;
} hook_into_t;

body_type_t get_type(body_t *body) {
  return ((object_info_t *) body_get_info(body))->body_type;
}

bool is_frozen(body_t *slime) {
  return ((slime_info_t *) ((object_info_t *) body_get_info(slime))->info)->frozen;
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
    set_hook(slime, NULL);
  }
  if (get_line(slime) != NULL) {
    body_remove(get_line(slime));
    set_line(slime, NULL);
  }
}

bool is_dead(scene_t *scene) {
  return (scene_bodies(scene) == 0 || get_type(scene_get_body(scene, 0)) != SLIME || body_is_removed(scene_get_body(scene, 0)));
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
  if (is_dead(scene)) return;
  create_springy_rope(scene, HOOK_SPRING_K, scene_get_body(scene, 0), body1, GRAPPLE_PORTION);
  unfreeze(scene_get_body(scene, 0));
}

void create_hook_collision(scene_t *scene, body_t *body1, body_t *body2) {
  create_post_collision(scene, body1, body2, hook_collision_force_creator, (void *) scene, NULL);
}

void unhook_collision_force_creator(body_t *body1, body_t *body2, vector_t axis, void *aux, double overlap) {
  if (is_dead((scene_t *) aux)) return;
  unhook(scene_get_body((scene_t *) aux, 0)); 
}

void create_unhook_collision(scene_t *scene, body_t *line, body_t *body2) {
  create_collision(scene, line, body2, unhook_collision_force_creator, (void *) scene, NULL);
}

list_t *make_circle(vector_t center, double radius, size_t num_points) { //move into new file for making standard shapes? (circle, rectangle)
  list_t *circle = list_init(num_points, (free_func_t) free);
  double curr_angle = 0;
  double vert_angle = TWOPI / num_points;
  for (size_t i = 0; i < num_points; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);
    point->x = cos(curr_angle) * radius + center.x;
    point->y = sin(curr_angle) * radius + center.y;
    list_add(circle, point);
    curr_angle += vert_angle;
  }
  return circle;
}

list_t *make_rect(vector_t bottom_left, double len, double height) { //move into new file for making standard shapes? (circle, rectangle)
  list_t *rect = list_init(4, (free_func_t) free);

  vector_t *point = malloc(sizeof(vector_t));
  assert(point != NULL);
  *point = bottom_left;
  list_add(rect, point);

  point = malloc(sizeof(vector_t));
  assert(point != NULL);
  bottom_left.x += len;
  *point = bottom_left;
  list_add(rect, point);

  point = malloc(sizeof(vector_t));
  assert(point != NULL);
  bottom_left.y += height;
  *point = bottom_left;
  list_add(rect, point);

  point = malloc(sizeof(vector_t));
  assert(point != NULL);
  bottom_left.x -= len;
  *point = bottom_left;
  list_add(rect, point);

  return rect;
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

void slime_jump(scene_t *scene, vector_t mouse_pos) {
  body_t *slime = scene_get_body(scene, 0);
  unfreeze(slime);
  unhook(scene_get_body(scene, 0));
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

body_t *initialize_slime() {
  object_info_t *slime_info = malloc(sizeof(object_info_t));
  slime_info->body_type = SLIME;
  slime_info_t *sub_slime_info = malloc(sizeof(slime_info_t));
  sub_slime_info->frozen = false;
  sub_slime_info->hook = NULL;
  sub_slime_info->line = NULL;
  slime_info->info = sub_slime_info;
  slime_info->info_freeer = free;
  body_t *slime = body_init_with_info(make_circle(SLIME_SPAWN, SLIME_RADIUS, 
    NUM_POINTS), SLIME_MASS, SLIME_COLOR, slime_info, (free_func_t) object_info_freeer);
  return slime;
}

void add_normal_wall(scene_t *scene, vector_t bottom_left, double len, double height) {
  list_t *wall_points = make_rect(bottom_left, len, height);
  object_info_t *w_info = malloc(sizeof(object_info_t));
  w_info->body_type = REGULAR_WALL;
  w_info->info = NULL;
  w_info->info_freeer = NULL;
  body_t *wall = body_init_with_info(wall_points, INFINITY, REGULAR_WALL_COLOR, w_info, (free_func_t) object_info_freeer);
  scene_add_body(scene, wall);
  create_stick_collision(scene, scene_get_body(scene, 0), wall);
}

void add_thorny_wall(scene_t *scene, vector_t bottom_left, double len, double height) {
    list_t *wall_points = make_rect(bottom_left, len, height);
    object_info_t *w_info = malloc(sizeof(object_info_t));
    w_info->body_type = THORNY_WALL;
    w_info->info = NULL;
    w_info->info_freeer = NULL;
    body_t *wall = body_init_with_info(wall_points, INFINITY, THORNY_WALL_COLOR, w_info, (free_func_t) object_info_freeer);
    scene_add_body(scene, wall);
    create_one_way_destructive_collision(scene, scene_get_body(scene, 0), wall);
}

int make_mod_ten(int length){
  int remainder = length % 20;
  return remainder;
}

int generate_random_width() {
  int width = rand() % (int)(MAX_WALL_WIDTH - MIN_WALL_WIDTH) + MIN_WALL_WIDTH;
  return width;
}

int generate_random_height() { 
  int height = rand() % (int)(MAX_WALL_HEIGHT - MIN_WALL_HEIGHT) + MIN_WALL_HEIGHT;
  int remainder = make_mod_ten(height);
  height = (height - remainder);
  return height;
}

void level_one_top_walls(scene_t *scene){
  double x_coord = 0.0; 
  // Create walls along the top of the screen
  for (size_t i = 0; i < TOP_WALLS; i++) {
    double width = 132;
    double height = 1400;
    double y_coord = WINDOW.y - 40; // Y-coordinate for top walls
    vector_t top_left = {.x = x_coord, .y = y_coord};
        add_normal_wall(scene, top_left, width, height);
    x_coord += width; // Update the total width for top walls
  }
}

void level_one_bottom_walls(scene_t *scene) {
  double x_coord = 0.0; 
  for (size_t i = 0; i < LEVEL_ONE_BOTTOM_WALLS; i++) {
    int width = generate_random_width();
    int height = generate_random_height();
    vector_t bottom_left = {.x = x_coord, .y = 0};
    add_thorny_wall(scene, bottom_left, width, height);
    x_coord += width; 
  } 
}

void init_left_wall(scene_t *scene){
  double y_coord = 0.0;
  for (size_t i = 0; i < LEFT_WALLS; i++) {
    double width = generate_random_width();
    double height = generate_random_height();
    double x_coord = 0; 
    vector_t top_left = {.x = x_coord, .y = y_coord};
    add_thorny_wall(scene, top_left, width, height);
    y_coord += height;
  }
}

void level_one_horizontal_platforms(scene_t *scene){
  double spacing = 200;
  double x_coord = 0.0; 
  for (size_t i = 0; i < LEVEL_ONE_PLATFORM_WALLS; i++) {
    double width = generate_random_width();
    double height = generate_random_height();
    double y_coord = 600; 
    vector_t top_left = {.x = x_coord + spacing, .y = y_coord};
    add_normal_wall(scene, top_left, width, width);
    x_coord += height + spacing;
  }
}

void make_upward_tunnel(scene_t *scene){
  double spacing = 200;
  double y_coord = 50.0; 
  for (size_t i = 0; i < UPWARD_TUNNEL; i++) {
    double width = generate_random_width();
    double height = generate_random_height();
    double x_coord = 6900; 
    vector_t top_left = {.x = x_coord, .y = y_coord + spacing};
    add_normal_wall(scene, top_left, width, width);
    y_coord += height + spacing;
  }
}

void first_checkpoint(scene_t *scene) {
  double y_coord = 0.0; 
  for (size_t i = 0; i < CHECKPOINT_WALLS; i++) {
    double width = generate_random_width();
    double height = generate_random_height();
    double x_coord = 7100; 
    vector_t top_left = {.x = x_coord, .y = y_coord};
    add_thorny_wall(scene, top_left, width, height);
    y_coord += height;
  }
}


void level_two_bottom_walls(scene_t *scene) {
  double x_coord = 7100; 
  for (size_t i = 0; i < LEVEL_TWO_BOTTOM_WALLS; i++) {
    int width = generate_random_width();
    int height = generate_random_height();
    double y_coord = WINDOW.y + 650;
    vector_t bottom_left = {.x = x_coord, .y = y_coord};
    add_thorny_wall(scene, bottom_left, width, height);
    x_coord += width; 
  } 
}

void level_two_platforms(scene_t *scene){
  double spacing = 300;
  double x_coord = 7000; 
  int interval = 3;
  list_t *base_platforms = list_init(LEVEL_TWO_PLATFORM_WALLS, NULL);
  for (size_t i = 0; i < LEVEL_TWO_PLATFORM_WALLS; i++) {
    int width = generate_random_width();
    double height = 50;
    vector_t bottom_left = {.x = x_coord + spacing, .y = WINDOW.y + 1000};    
    if ((rand() % interval) == 1) {
      add_thorny_wall(scene, bottom_left, width, height);
      body_t *latest_wall = scene_get_body(scene, scene_bodies(scene) - 1);
      list_add(base_platforms, latest_wall);
    } else {
      add_normal_wall(scene, bottom_left, width, height);
      body_t *latest_wall = scene_get_body(scene, scene_bodies(scene) - 1);
      list_add(base_platforms, latest_wall);
    }
    x_coord += width + spacing;
  }

  for(size_t i = 0; i < list_size(base_platforms); i ++){
    body_t *curr_body = list_get(base_platforms, i);
    if(get_type(curr_body) == THORNY_WALL){
      int width = generate_random_width();
      double height = 50;
      vector_t centroid = body_get_centroid(curr_body);   
      vector_t bottom_left = {.x = centroid.x - height, .y = centroid.y + 300};
      add_normal_wall(scene, bottom_left, width, height);
    }
  }
  list_free(base_platforms);
}

void second_checkpoint(scene_t *scene) {
  double right_total_height = -700; 
  for (size_t i = 0; i < SECOND_CHECKPOINT_WALLS; i++) {
    double width = generate_random_width();
    double height = generate_random_height();
    double x_right = 15500; 
    vector_t top_left = {.x = x_right, .y = right_total_height};
    add_normal_wall(scene, top_left, width, height);
    right_total_height += height;
  }
}

void make_downward_tunnel(scene_t *scene){
  double y_coord = 0.0; 
  for (size_t i = 0; i < DOWNWARD_TUNNEL; i++) {
    double width = generate_random_width();
    double height = generate_random_height();
    double x_coord = 16200; 
    vector_t top_left = {.x = x_coord, .y = y_coord};
    add_normal_wall(scene, top_left, width, height);
    y_coord += height;
  }
}

void make_downward_tunnel_obstacles(scene_t *scene){
  double spacing = 100;
  double right_total_width = 1500; 
  for (size_t i = 0; i < DOWNWARD_TUNNEL_OBSTACLES; i++) {
    double width = generate_random_width();
    double height = generate_random_height();
    double x_right = rand() % (16100 - 15600) + 15600;
    vector_t top_left = {.x = x_right, .y = right_total_width - spacing};
    add_thorny_wall(scene, top_left, width, width);
    right_total_width -= height + spacing;  // Update the total width for the downward tunnel
  }
}

void level_three_bottom_walls(scene_t *scene) {
  double bottom_total_width = 15500; 
  double y_coord = -700;
  for (size_t i = 0; i < LEVEL_TWO_BOTTOM_WALLS; i++) {
    int width = generate_random_width();
    int height = generate_random_height();
    vector_t bottom_left = {.x = bottom_total_width, .y = y_coord};
    add_thorny_wall(scene, bottom_left, width, height);
    bottom_total_width += width; 
  } 
}

void level_three_top_walls(scene_t *scene) {
  double x_coord = 16200; 
  double y_coord = -50;
  for (size_t i = 0; i < LEVEL_TWO_BOTTOM_WALLS; i++) {
    int width = generate_random_width();
    int height = generate_random_height();
    vector_t bottom_left = {.x = x_coord, .y = y_coord};
    add_thorny_wall(scene, bottom_left, width, height);
    x_coord += width; 
  } 
}

void level_three_horizontal_platforms(scene_t *scene){
  double spacing = 150;
  double x_coord = 16150; 
  for (size_t i = 0; i < LEVEL_ONE_PLATFORM_WALLS; i++) {
    double width = generate_random_width();
    double height = generate_random_height();
    double y_coord = rand() % (17100 - 16100) + 16100;
    vector_t top_left = {.x = x_coord + spacing, .y = y_coord};
    add_normal_wall(scene, top_left, width, width);
    x_coord += height + spacing;
  }
}


void initialize_level_design(scene_t *scene) {
  add_normal_wall(scene, (vector_t){.x = 400, .y = 300}, WALL_WIDTH, WALL_HEIGHT);
  level_one_bottom_walls(scene);
  level_one_top_walls(scene);
  init_left_wall(scene);
  level_one_horizontal_platforms(scene);
  make_upward_tunnel(scene);
  first_checkpoint(scene);
  level_two_bottom_walls(scene);
  level_two_platforms(scene);
  second_checkpoint(scene);
  make_downward_tunnel(scene);
  make_downward_tunnel_obstacles(scene);
  level_three_bottom_walls(scene);
  level_three_top_walls(scene);
  level_three_horizontal_platforms(scene);
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
  vector_t slime_cen= body_get_centroid(slime);
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
  return body_init_with_info(points, INFINITY, HOOK_COLOR, hook_info, (free_func_t) object_info_freeer);
}

void slime_grapple(state_t *state, vector_t mouse_pos) {
  scene_t *scene = state->scene;
  body_t *slime = scene_get_body(scene, 0);
  unfreeze(slime);
  unhook(scene_get_body(scene, 0));
  vector_t slime_pos = body_get_centroid(slime);
  vector_t direction = vec_subtract(mouse_pos, slime_pos);
  direction = vec_normalized(direction);
  list_t *hook_points = make_circle(vec_add(slime_pos, vec_multiply(SLIME_RADIUS + HOOK_RADIUS, direction)), HOOK_RADIUS, NUM_POINTS);
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
        slime_jump(state->scene, mouse);
        break;
      }
    }
  }
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
    }
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

void shift_camera(state_t *state) {
  if (is_dead(state->scene)) return;
  vector_t new_shift = VEC_ZERO;
  vector_t slime_pos = body_get_centroid(scene_get_body(state->scene, 0));
  if (slime_pos.x > MAX_X) {
    new_shift.x = slime_pos.x - MAX_X;
  }
  else if (slime_pos.x < MIN_X) {
    new_shift.x = slime_pos.x - MIN_X;
  }
  if (slime_pos.y > MAX_Y) {
    new_shift.y = slime_pos.y - MAX_Y;
  }
  else if (slime_pos.y < MIN_Y) {
    new_shift.y = slime_pos.y - MIN_Y;
  }
  if (new_shift.x != 0 || new_shift.y != 0) {
    state->shift = vec_add(state->shift, new_shift);
    shift_scene(state->scene, new_shift);
  }
}

void create_invisible_walls(state_t *state) {
  list_t *wall1_points = make_rect((vector_t){.x = -BORDER_LEN, .y = 0}, BORDER_LEN, WINDOW.y);
  object_info_t *w1_info = malloc(sizeof(object_info_t));
  w1_info->body_type = BORDER;
  w1_info->info = NULL;
  w1_info->info_freeer = NULL;
  body_t *wall1 = body_init_with_info(wall1_points, INFINITY, INVISIBLE, w1_info, (free_func_t) object_info_freeer);
  state->invis_wall1 = wall1;
  list_t *wall2_points = make_rect((vector_t){.x = WINDOW.x, .y = 0}, BORDER_LEN, WINDOW.y);
  object_info_t *w2_info = malloc(sizeof(object_info_t));
  w2_info->body_type = BORDER;
  w2_info->info = NULL;
  w2_info->info_freeer = NULL;
  body_t *wall2 = body_init_with_info(wall2_points, INFINITY, INVISIBLE, w2_info, (free_func_t) object_info_freeer);
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

scene_t *initialize_scene(state_t *state) {
  scene_t *scene = scene_init();
  body_t *slime = initialize_slime();
  create_downward_gravity(scene, GRAVITY, slime);
  scene_add_body(scene, slime);
  initialize_level_design(scene);
  create_invisible_walls(state);
  return scene;
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);

  sdl_init(VEC_ZERO, WINDOW);

  state->scene = initialize_scene(state);
  state->shift = VEC_ZERO;

  char const* const fileName = "assets/level.txt";
  FILE* file = fopen(fileName, "r");
  assert(file);
  char line[256];
  while (fgets(line, sizeof(line), file)) {
    printf("%s", line); 
  }
  fclose(file); //project 0 code
  
  sdl_on_click((mouse_handler_t) on_mouse);
  sdl_on_key((key_handler_t) on_key);
  return state;
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  if (dt > MAX_DT) {
    dt = MAX_DT;
  }
  scene_tick(state->scene, dt);
  if (is_dead(state->scene)) {
    add_invisible_walls(state);
    // unhook(scene_get_body(state->scene, 0)); // doesn't actually render unhook obviously can't do this slime isn't here
    //find alternative fix
    sdl_render_scene(state->scene);
    remove_invisible_walls(state);
    exit(0);
  }
  hook_line_updater(state->scene);
  enforce_freeze(state->scene);
  shift_camera(state);
  add_invisible_walls(state);
  sdl_render_scene(state->scene);
  remove_invisible_walls(state);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  body_free(state->invis_wall1);
  body_free(state->invis_wall2);
  free(state);
}

int main(int argc, char *argv[]) {
    state_t *state = emscripten_init();
    for (size_t i = 0; i < 10; i++) {
        emscripten_main(state);
        printf("%zu\n", i);
    }
    emscripten_free(state);
}