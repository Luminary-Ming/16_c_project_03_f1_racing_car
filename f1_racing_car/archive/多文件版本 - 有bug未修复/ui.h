#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include "game.h"

// UI状态
typedef struct
{
    SDL_Texture *game_over_texture;
    SDL_Texture *number_textures[10];
} UIState;

// 全局UI状态声明
extern UIState ui_state;

// UI管理函数
int init_ui(SDL_Renderer *renderer);
void render_ui(GameState *game);       
void render_game_over(SDL_Renderer *renderer);
void cleanup_ui(void);

// 需要这个函数声明
float get_speed_multiplier(int score);

#endif