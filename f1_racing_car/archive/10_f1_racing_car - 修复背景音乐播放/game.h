#ifndef __GAME_H
#define __GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

// 游戏状态
typedef enum
{
    GAME_RUNNING,  // 游戏运行中
    GAME_PAUSED,  // 游戏暂停
    GAME_OVER  // 游戏结束
} GameState;

// 运行游戏主循环
int run_game(void);

#endif