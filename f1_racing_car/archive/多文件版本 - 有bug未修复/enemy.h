#ifndef ENEMY_H
#define ENEMY_H

#include <SDL2/SDL.h>
#include "game.h"  // 包含GameState的完整定义

// 敌方车辆结构
typedef struct EnemyCar
{
    int x;
    float y;
    int width;
    int height;
    float speed;
    int active;
    int type;
    SDL_Texture *texture;
} EnemyCar;

// 全局变量声明
extern EnemyCar enemies[];

// 敌方车辆管理函数
int init_enemy(SDL_Renderer *renderer);
void update_enemies(GameState *game, float speed_multiplier); 
void render_enemies(SDL_Renderer *renderer);
void cleanup_enemy(void);
void spawn_enemy(GameState *game, float speed_multiplier); 

#endif