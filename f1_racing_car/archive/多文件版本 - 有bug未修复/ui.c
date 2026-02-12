#include "ui.h"
#include "game.h"
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL_image.h>

// 全局UI状态
UIState ui_state = { 0 };

// 数字图片文件列表
static const char *number_files[] = {
    "assets/ui/0.jpg",
    "assets/ui/1.jpg",
    "assets/ui/2.jpg",
    "assets/ui/3.jpg",
    "assets/ui/4.jpg",
    "assets/ui/5.jpg",
    "assets/ui/6.jpg",
    "assets/ui/7.jpg",
    "assets/ui/8.jpg",
    "assets/ui/9.jpg"
};

// 静态函数声明
static void render_score(GameState *game);
static void render_lives(GameState *game);
static void render_speed_multiplier(GameState *game);
static void render_simple_char(SDL_Renderer *renderer, int x, int y, char ch);

// 初始化UI系统
int init_ui(SDL_Renderer *renderer)
{
    // 初始化UI状态
    ui_state.game_over_texture = NULL;

    // 初始化数字纹理数组
    for (int i = 0; i < 10; i++)
    {
        ui_state.number_textures[i] = NULL;
    }

    // 加载game over图片
    SDL_Surface *surface = IMG_Load("assets/ui/game_over.png");
    if (surface)
    {
        ui_state.game_over_texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!ui_state.game_over_texture)
        {
            printf("创建game over纹理失败\n");
        }
        else
        {
            printf("game over图片加载成功\n");
        }
    }
    else
    {
        printf("无法加载game over图片: %s\n", IMG_GetError());
    }

    // 加载数字纹理
    int loaded_count = 0;
    for (int i = 0; i < 10; i++)
    {
        surface = IMG_Load(number_files[i]);
        if (surface)
        {
            ui_state.number_textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);

            if (ui_state.number_textures[i])
            {
                loaded_count++;
            }
        }
    }

    printf("数字纹理加载: %d/10\n", loaded_count);

    // 即使没有加载到所有数字纹理，也返回成功
    return 1;
}

// 渲染UI（分数、生命值、速度倍率）
void render_ui(GameState *game)
{
    // 渲染分数
    render_score(game);

    // 渲染生命值
    render_lives(game);

    // 渲染速度倍率
    render_speed_multiplier(game);
}

// 渲染分数
static void render_score(GameState *game)
{
    // 绘制分数背景框
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 180);
    SDL_Rect bg = { 10, 10, 120, 50 };
    SDL_RenderFillRect(game->renderer, &bg);

    // 绘制分数边框
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(game->renderer, &bg);

    // 渲染分数文字标签
    int label_x = 20;
    int label_y = 15;

    // 绘制"SCORE:"标签（简单字符表示）
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    for (int i = 0; i < 6; i++)
    {
        SDL_Rect char_rect = { label_x + i * 12, label_y, 10, 15 };
        SDL_RenderDrawRect(game->renderer, &char_rect);
    }

    // 渲染分数值
    char score_str[20];
    sprintf(score_str, "%d", game->score);

    int score_x = 25;
    int score_y = 35;

    // 检查是否有可用的数字纹理
    int has_numbers = 0;
    for (int i = 0; i < 10; i++)
    {
        if (ui_state.number_textures[i])
        {
            has_numbers = 1;
            break;
        }
    }

    if (has_numbers)
    {
        // 使用数字纹理显示分数
        for (int i = 0; score_str[i]; i++)
        {
            int digit = score_str[i] - '0';
            if (digit >= 0 && digit <= 9 && ui_state.number_textures[digit])
            {
                SDL_Rect dest = { score_x, score_y, 20, 30 };
                SDL_RenderCopy(game->renderer, ui_state.number_textures[digit], NULL, &dest);
                score_x += 22;
            }
            else
            {
                // 如果没有对应数字纹理，使用简单图形
                render_simple_char(game->renderer, score_x, score_y, score_str[i]);
                score_x += 15;
            }
        }
    }
    else
    {
        // 如果没有数字纹理，使用简单图形显示
        for (int i = 0; score_str[i]; i++)
        {
            render_simple_char(game->renderer, score_x, score_y, score_str[i]);
            score_x += 15;
        }
    }
}

// 渲染生命值
static void render_lives(GameState *game)
{
    int x = WINDOW_W - 150;
    int y = 15;

    // 绘制"LIVES:"标签
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    for (int i = 0; i < 6; i++)
    {
        SDL_Rect char_rect = { x + i * 12, y, 10, 15 };
        SDL_RenderDrawRect(game->renderer, &char_rect);
    }

    // 绘制生命值方块
    y = 35;
    x = WINDOW_W - 150;

    for (int i = 0; i < game->lives; i++)
    {
        // 绘制红色方块表示生命值
        SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
        SDL_Rect life_rect = { x, y, 30, 30 };
        SDL_RenderFillRect(game->renderer, &life_rect);

        // 绘制生命值方块边框
        SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(game->renderer, &life_rect);

        x += 35;
    }
}

// 渲染速度倍率
static void render_speed_multiplier(GameState *game)
{
    // 计算速度倍率
    float multiplier = get_speed_multiplier(game->score);

    // 格式化速度倍率字符串
    char speed_str[20];
    sprintf(speed_str, "SPEED: x%.1f", multiplier);

    int x = WINDOW_W - 150;
    int y = 65;

    // 绘制速度倍率背景
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 180);
    SDL_Rect bg = { x - 10, y - 5, 140, 30 };
    SDL_RenderFillRect(game->renderer, &bg);

    // 绘制边框
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(game->renderer, &bg);

    // 渲染速度倍率文字
    for (int i = 0; speed_str[i]; i++)
    {
        render_simple_char(game->renderer, x, y, speed_str[i]);
        x += 12;
    }
}

// 渲染游戏结束画面
void render_game_over(SDL_Renderer *renderer)
{
    if (ui_state.game_over_texture)
    {
        // 使用game over纹理
        SDL_Rect dest = { 0, 0, WINDOW_W, WINDOW_H };
        SDL_RenderCopy(renderer, ui_state.game_over_texture, NULL, &dest);

    }
}

// 清理UI资源
void cleanup_ui(void)
{
    // 清理game over纹理
    if (ui_state.game_over_texture)
    {
        SDL_DestroyTexture(ui_state.game_over_texture);
        ui_state.game_over_texture = NULL;
        printf("清理game over纹理\n");
    }

    // 清理数字纹理
    for (int i = 0; i < 10; i++)
    {
        if (ui_state.number_textures[i])
        {
            SDL_DestroyTexture(ui_state.number_textures[i]);
            ui_state.number_textures[i] = NULL;
        }
    }

    printf("UI资源清理完成\n");
}

// 渲染简单字符（用矩形表示）
static void render_simple_char(SDL_Renderer *renderer, int x, int y, char ch)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // 根据字符类型绘制不同的简单图形
    if ((ch >= '0' && ch <= '9') ||
        (ch >= 'A' && ch <= 'Z') ||
        (ch >= 'a' && ch <= 'z'))
    {
        // 字母和数字：绘制矩形框
        SDL_Rect char_rect = { x, y, 12, 20 };
        SDL_RenderDrawRect(renderer, &char_rect);
    }
    else if (ch == '.')
    {
        // 小数点：绘制小矩形
        SDL_Rect dot_rect = { x + 5, y + 15, 4, 4 };
        SDL_RenderFillRect(renderer, &dot_rect);
    }
    else if (ch == 'x' || ch == 'X')
    {
        // 乘号：绘制交叉线
        SDL_Rect cross1 = { x, y, 12, 20 };
        SDL_RenderDrawRect(renderer, &cross1);

        // 绘制斜线
        SDL_RenderDrawLine(renderer, x, y, x + 12, y + 20);
        SDL_RenderDrawLine(renderer, x + 12, y, x, y + 20);
    }
    else if (ch == ':')
    {
        // 冒号：绘制两个小点
        SDL_Rect dot1 = { x + 5, y + 5, 4, 4 };
        SDL_Rect dot2 = { x + 5, y + 15, 4, 4 };
        SDL_RenderFillRect(renderer, &dot1);
        SDL_RenderFillRect(renderer, &dot2);
    }
    else
    {
        // 其他字符：绘制简单的矩形
        SDL_Rect char_rect = { x, y, 12, 20 };
        SDL_RenderDrawRect(renderer, &char_rect);
    }
}