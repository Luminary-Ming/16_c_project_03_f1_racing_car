#ifndef RENDERING_H
#define RENDERING_H

#include <SDL2/SDL.h>
#include "game.h"  // 包含GameState的完整定义

// 共享变量声明
extern int current_map_index;

// 渲染管理函数
int init_rendering(SDL_Renderer *renderer);
void render_map(GameState *game);
void render_explosion(GameState *game);
void cleanup_rendering(void);

#endif