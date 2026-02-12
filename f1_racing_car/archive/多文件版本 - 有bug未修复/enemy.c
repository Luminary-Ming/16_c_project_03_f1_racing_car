#include "enemy.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_image.h>

// 定义全局敌方车辆数组
EnemyCar enemies[MAX_ENEMIES] = { 0 };

// 敌方车辆纹理数组
static SDL_Texture *enemy_textures[ENEMY_CARS_COUNT] = { NULL };

// 敌方车辆文件列表
static const char *enemy_files[] = {
    "assets/cars/red_down.png",
    "assets/cars/black_down.png",
    "assets/cars/bumblebee_down.png",
    "assets/cars/taxi_down.png",
    "assets/cars/tractor_down.png",
    "assets/cars/grey_down.png",
    "assets/cars/white_down.png",
    "assets/cars/motorcycle_down.png",
    "assets/cars/optimus_prime_down.png"
};

// 初始化敌方车辆系统
int init_enemy(SDL_Renderer *renderer)
{
    // 加载敌方车辆纹理
    for (int i = 0; i < ENEMY_CARS_COUNT; i++)
    {
        SDL_Surface *surface = IMG_Load(enemy_files[i]);
        if (surface)
        {
            enemy_textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
    }

    // 初始化敌方车辆
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].x = 0;
        enemies[i].y = -CAR_HEIGHT;
        enemies[i].width = CAR_WIDTH;
        enemies[i].height = CAR_HEIGHT;
        enemies[i].speed = 3.0f + (rand() % 10) / 5.0f;
        enemies[i].active = 0;
        enemies[i].type = rand() % ENEMY_CARS_COUNT;
        enemies[i].texture = enemy_textures[enemies[i].type];
    }

    return 1;
}

// 更新敌方车辆
void update_enemies(GameState *game, float speed_multiplier)
{
    // 生成新的敌方车辆
    game->enemy_spawn_timer++;
    if (game->enemy_spawn_timer >= game->enemy_spawn_delay)
    {
        game->enemy_spawn_timer = 0;
        spawn_enemy(game, speed_multiplier);
    }

    // 更新现有敌方车辆
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            float effective_speed = enemies[i].speed * (1.0f + game->enemy_scroll_factor);
            enemies[i].y += effective_speed;

            // 检查是否超出屏幕
            if (enemies[i].y > WINDOW_H)
            {
                enemies[i].active = 0;
                game->score += 10;
            }
        }
    }
}

// 生成敌方车辆
void spawn_enemy(GameState *game, float speed_multiplier)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            enemies[i].active = 1;

            int lane = rand() % 2;
            enemies[i].x = (lane == 0) ?
                (WINDOW_W / 2 - CAR_WIDTH - 30) :
                (WINDOW_W / 2 + CAR_WIDTH / 2);

            enemies[i].y = -CAR_HEIGHT;

            float base_speed = 3.0f + (rand() % 10) / 5.0f;
            enemies[i].speed = base_speed * speed_multiplier;

            enemies[i].type = rand() % ENEMY_CARS_COUNT;
            enemies[i].texture = enemy_textures[enemies[i].type];

            int base_delay = 60;
            int min_delay = 20;
            int adjusted_delay = base_delay - (game->score / 50) * 5;
            if (adjusted_delay < min_delay) adjusted_delay = min_delay;
            game->enemy_spawn_delay = adjusted_delay + (rand() % 20);

            break;
        }
    }
}

// 渲染敌方车辆
void render_enemies(SDL_Renderer *renderer)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active && enemies[i].texture)
        {
            SDL_Rect dest = { enemies[i].x, (int)enemies[i].y, CAR_WIDTH, CAR_HEIGHT };
            SDL_RenderCopy(renderer, enemies[i].texture, NULL, &dest);
        }
    }
}

// 清理敌方车辆资源
void cleanup_enemy(void)
{
    for (int i = 0; i < ENEMY_CARS_COUNT; i++)
    {
        if (enemy_textures[i])
        {
            SDL_DestroyTexture(enemy_textures[i]);
        }
    }
}