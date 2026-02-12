#include "rendering.h"
#include "game.h"
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL_image.h>

static SDL_Texture *map_textures[5] = { NULL };
static int map_heights[5] = { 0 };

int current_map_index = 0;

static const char *map_files[] = {
    "assets/maps/map1.png",
    "assets/maps/map2.png",
    "assets/maps/map3.png",
    "assets/maps/map4.png",
    "assets/maps/map5.png"
};

int init_rendering(SDL_Renderer *renderer)
{
    for (int i = 0; i < 5; i++)
    {
        SDL_Surface *surface = IMG_Load(map_files[i]);
        if (surface)
        {
            map_textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_QueryTexture(map_textures[i], NULL, NULL, NULL, &map_heights[i]);
            SDL_FreeSurface(surface);
        }
    }
    return 1;
}

// 渲染地图
void render_map(GameState *game)
{
    if (!map_textures[current_map_index] || map_heights[current_map_index] <= 0)
    {
        return;
    }

    // 使用双缓冲纹理渲染
    if (game->buffer_texture)
    {
        SDL_SetRenderTarget(game->renderer, game->buffer_texture);
        SDL_RenderClear(game->renderer);

        // 计算滚动位置
        float actual_scroll = fmod(game->scroll_position, map_heights[current_map_index]);
        if (actual_scroll < 0) actual_scroll += map_heights[current_map_index];

        // 计算第一部分
        int src_y1 = (int)actual_scroll;
        int height1 = WINDOW_H;

        // 如果需要绘制第二部分
        if (src_y1 + WINDOW_H > map_heights[current_map_index])
        {
            height1 = map_heights[current_map_index] - src_y1;

            // 绘制第二部分
            SDL_Rect src2 = { 0, 0, WINDOW_W, WINDOW_H - height1 };
            SDL_Rect dest2 = { 0, height1, WINDOW_W, WINDOW_H - height1 };
            SDL_RenderCopy(game->renderer, map_textures[current_map_index], &src2, &dest2);
        }

        // 绘制第一部分
        SDL_Rect src1 = { 0, src_y1, WINDOW_W, height1 };
        SDL_Rect dest1 = { 0, 0, WINDOW_W, height1 };
        SDL_RenderCopy(game->renderer, map_textures[current_map_index], &src1, &dest1);

        SDL_SetRenderTarget(game->renderer, NULL);
        SDL_RenderCopy(game->renderer, game->buffer_texture, NULL, NULL);
    }
}

// 渲染爆炸特效
void render_explosion(GameState *game)
{
    if (!game->explosion_texture) return;  // 添加安全检查

    SDL_SetTextureAlphaMod(game->explosion_texture, (Uint8)(game->explosion_alpha * 255));

    SDL_Rect dest = {
        game->explosion_x,
        game->explosion_y,
        80,
        80
    };
    SDL_RenderCopy(game->renderer, game->explosion_texture, NULL, &dest);

    SDL_SetTextureAlphaMod(game->explosion_texture, 255);
}

// 清理渲染资源
void cleanup_rendering(void)
{
    for (int i = 0; i < 5; i++)
    {
        if (map_textures[i])
        {
            SDL_DestroyTexture(map_textures[i]);
        }
    }
}