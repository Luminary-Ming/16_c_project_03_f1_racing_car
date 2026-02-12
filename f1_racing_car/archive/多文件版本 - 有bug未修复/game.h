#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#define WINDOW_W 500
#define WINDOW_H 800
#define CAR_WIDTH 70
#define CAR_HEIGHT 130
#define MAX_ENEMIES 5
#define ENEMY_CARS_COUNT 9

struct EnemyCar;
struct BGMState;
struct CarState;
struct TransitionState;
struct UIState;

// 游戏状态结构体
typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *buffer_texture;

    int running;
    int game_over;
    int score;
    int lives;

    int car_x;
    int car_y;
    int base_car_speed;
    int current_car_speed;

    SDL_Texture *current_map_texture;
    int current_map_height;
    float scroll_position;
    int scroll_speed;

    const Uint8 *key_state;
    float enemy_scroll_factor;

    int enemy_spawn_timer;
    int enemy_spawn_delay;

    SDL_Texture *explosion_texture;
    float explosion_alpha;
    int explosion_duration;
    int explosion_x;
    int explosion_y;
} GameState;

// 游戏管理函数
int init_game(GameState *game);
void handle_events(GameState *game);
void update_game(GameState *game);
void render_game(GameState *game);
void cleanup_game(GameState *game);
float get_speed_multiplier(int score);

#endif