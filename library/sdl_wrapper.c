#include "sdl_wrapper.h"
#include "list.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * The mousepress handler, or NULL if none has been configured.
 */
mouse_handler_t mouse_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

/** Gets the character code corresponding to a mouse click.*/
char get_mousecode(SDL_MouseButtonEvent mouse) {
  switch (mouse.button) {
    case SDL_BUTTON_LEFT:
      return MOUSE_LEFT;
      break;
    case SDL_BUTTON_RIGHT:
      return MOUSE_RIGHT;
      break;
    default:
    return '\0';
  }
  }

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
}

bool sdl_is_done(state_t *state) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      free(event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      // Skip the keypress if no handler is configured
      // or an unrecognized key was pressed
      if (key_handler == NULL)
        break;
      char key = get_keycode(event->key.keysym.sym);
      if (key == '\0')
        break;

      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type =
          event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(key, type, held_time, state);
      break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        if (mouse_handler == NULL) break;
        char button = get_mousecode(event->button);
        if (button == '\0') break;
                                
        mouse_event_type_t click = event->type == SDL_MOUSEBUTTONUP ? MOUSE_RELEASED : MOUSE_PRESSED;
        mouse_handler(button, click, 0, state);
        break;
    }
  }
  free(event);
  return false;
}

void *get_image_from_path(char *img_path) {
  return IMG_LoadTexture(renderer, img_path);
}

void remove_img_from_body(body_t *body) {
  SDL_Texture *img = body_get_img(body);
  if (img != NULL) {
    SDL_DestroyTexture(img);
  }
  body_set_img(body, NULL);
}

void sdl_draw(body_t *body) {
  SDL_Texture *img = body_get_img(body);
  assert(img);
  if (!is_rect(body) && !is_circle(body)) {
    int w, h;
    SDL_QueryTexture(img, NULL, NULL, &w, &h);
    vector_t body_pos = body_get_centroid(body);
    body_pos = get_window_position(body_pos, get_window_center());
    SDL_Rect texr;
    texr.w = w; texr.h = h;
    texr.x = body_pos.x - texr.w/2; texr.y = body_pos.y - texr.h/2; 
    double angle = -body_get_rotation(body)*180/M_PI;
    SDL_RenderCopyEx(renderer, img, NULL, &texr, angle, NULL, SDL_FLIP_NONE);
  }
  else if (is_circle(body)) {
    double radius = body_get_radius(body);
    vector_t top_left = body_get_centroid(body);
    top_left.x -= radius;
    top_left.y += radius;
    radius *= get_scene_scale(get_window_center());
    vector_t body_pos = get_window_position(top_left, get_window_center());
    SDL_Rect target;
    target.w = 2*radius; target.h = 2*radius;
    target.x = body_pos.x; target.y = body_pos.y;
    SDL_RenderCopy(renderer, img, NULL, &target);
  }
  else {
    double len = get_len(body);
    double height = get_height(body);
    vector_t top_left = body_get_centroid(body);
    top_left.x -= len/2;
    top_left.y += height/2;
    len *= get_scene_scale(get_window_center());
    height *= get_scene_scale(get_window_center());
    vector_t body_pos = get_window_position(top_left, get_window_center());
    SDL_Rect source;
    source.w = len; source.h = height;
    source.x = 0; source.y = 0;
    SDL_Rect texr;
    texr.w = len; texr.h = height;
    texr.x = body_pos.x; texr.y = body_pos.y;
    SDL_RenderCopy(renderer, img, &source, &texr);
  }
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);
  assert(0 <= color.r && color.r <= 1);
  assert(0 <= color.g && color.g <= 1);
  assert(0 <= color.b && color.b <= 1);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, color.a * 255);
  free(x_points);
  free(y_points);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_destroy_texture(void *texture) {
  SDL_DestroyTexture(texture);
}

void sdl_render_scene(scene_t *scene) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);

  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    char *img_path = body_get_img(body);
    if (img_path != NULL) {
      sdl_draw(body);
    }
  }

  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    sdl_draw_polygon(shape, body_get_color(body));
    list_free(shape);
  }

  sdl_show();
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

void sdl_on_click(mouse_handler_t handler) {mouse_handler = handler;}

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}

vector_t get_scene_pos(vector_t window_pos, vector_t window_center) {
  double scale = get_scene_scale(window_center);
  vector_t pixel_offset = vec_subtract(window_pos, window_center);
  vector_t scene_offset = vec_multiply(1 / scale, pixel_offset);
  vector_t scene_pos = vec_add(center, scene_offset);
  
  return scene_pos;
}


vector_t sdl_mouse_pos() {
    int *mouse_x = malloc(sizeof(int));
    int *mouse_y = malloc(sizeof(int));
    SDL_GetMouseState(mouse_x, mouse_y);
    vector_t window_mouse = (vector_t){*mouse_x, *mouse_y};
    vector_t scene_mouse = get_scene_pos(window_mouse,get_window_center() );
    
    free(mouse_x);
    free(mouse_y);
    return scene_mouse;
}
